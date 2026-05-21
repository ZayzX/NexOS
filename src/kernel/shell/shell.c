#include "../libs/scan.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../libs/print_string.h"
#include "../time/time.h"
#include "shell.h"
#include "../libs/printf.h"
#include "../process/process.h"

#include <stdint.h>

/* implémentations minimales (pas de libc) */
static int strcmp(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return (unsigned char)*a - (unsigned char)*b;
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static int atoi(const char* s) {
    int sign = 1; int v = 0;
    if (*s == '-') { sign = -1; s++; }
    while (*s >= '0' && *s <= '9') { v = v*10 + (*s - '0'); s++; }
    return v * sign;
}

static int is_leap(int y) {
    return (y%4==0 && y%100!=0) || (y%400==0);
}

static long rtc_to_seconds(const struct rtc_time *t) {
    int mdays[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    long days = 0;
    for (int y = 2000; y < t->year; ++y) days += 365 + (is_leap(y)?1:0);
    for (int m = 1; m < t->month; ++m) { days += mdays[m]; if (m==2 && is_leap(t->year)) days++; }
    days += t->day - 1;
    return days * 86400 + t->hour * 3600 + t->min * 60 + t->sec;
}

static void print_prompt(void) {
    puts_at(currentRow, 0, "> ");
    currentColumn = 2;
}

/* Lit une ligne en echoant les caractères à l'écran */
static void read_line(char* buf, int maxlen) {
    int idx = 0;
    while (1) {
        char c = scan_char();
        unsigned char uc = (unsigned char)c;

        if (uc == KBD_ENTER_NP || c == '\r' || c == '\n') {
            /* nouvelle ligne */
            buf[idx] = 0;
            currentRow++;
            currentColumn = 0;
            break;
        }

        if (uc == '\b') {
            if (idx > 0) {
                idx--;
                if (currentColumn > 0) {
                    currentColumn--;
                    put_char_at(currentRow, currentColumn, ' ');
                } else if (currentRow > 0) {
                    currentRow--;
                    currentColumn = 79;
                    put_char_at(currentRow, currentColumn, ' ');
                }
            }
            continue;
        }

        if (uc == KBD_SUPER_L || uc == KBD_SUPER_R) continue;

        /* printable */
        if (idx < maxlen - 1) {
            buf[idx++] = c;
            put_char_at(currentRow, currentColumn, c);
            currentColumn++;
            if (currentColumn >= 80) { currentColumn = 0; currentRow++; }
        }
    }
}

/* Tokenize in-place; returns number of tokens (max tokens limited). */
static int split_tokens(char* s, char* argv[], int max_args) {
    int argc = 0;
    char* p = s;
    while (*p && argc < max_args) {
        while (*p == ' ') p++;
        if (!*p) break;
        argv[argc++] = p;
        while (*p && *p != ' ') p++;
        if (*p == ' ') { *p = 0; p++; }
    }
    return argc;
}

void shell() {
    char line[128];
    char* argv[16];
    struct rtc_time boot_rt;
    rtc_read_time(&boot_rt);
    proc_register("shell");


    while (1) {
        print_prompt();
        read_line(line, sizeof(line));

        int argc = split_tokens(line, argv, 16);
        if (argc == 0) continue;

        /* commandes internes */
        if (strcmp(argv[0], "help") == 0) {
            puts_at(currentRow, 0, "help - affiche cette aide"); currentRow++;
            puts_at(currentRow, 0, "clear - efface l'écran"); currentRow++;
            puts_at(currentRow, 0, "echo ... - affiche le texte"); currentRow++;
            puts_at(currentRow, 0, "sum n m - affiche la somme de n et m"); currentRow++;
            puts_at(currentRow, 0, "version - affiche la version de l'os"); currentRow++;
            puts_at(currentRow, 0, "time - affiche la date et l'heure"); currentRow++;
            puts_at(currentRow, 0, "sysinfo - affiche des informations sur l'os"); currentRow++;
            puts_at(currentRow, 0, "reboot - tente un reset matériel via contrôleur clavier"); currentRow++;
            puts_at(currentRow, 0, "stop - met le CPU en halt"); currentRow++;
            continue;
        }

        if (strcmp(argv[0], "clear") == 0 || strcmp(argv[0], "cls") == 0) {
            clear_screen();
            continue;
        }

        if (strcmp(argv[0], "echo") == 0) {
            if (argc > 1) {
                int i;
                int col = currentColumn;
                for (i = 1; i < argc; ++i) {
                    puts_at(currentRow, currentColumn, argv[i]);
                    currentColumn += strlen(argv[i]);
                    if (i + 1 < argc) {
                        put_char_at(currentRow, currentColumn, ' ');
                        currentColumn++;
                    }
                }
                currentRow++;
                currentColumn = 0;
            } else {
                puts_at(currentRow, 0, ""); currentRow++;
            }
            continue;
        }

        if (strcmp(argv[0], "sum") == 0) {
            if (argc >= 3) {
                int a = atoi(argv[1]);
                int b = atoi(argv[2]);
                char buf[32];
                itoa(a + b, buf);
                puts_at(currentRow, 0, buf);
                currentRow++;
                currentColumn = 0;
            } else {
                puts_at(currentRow, 0, "usage: sum n m"); currentRow++; currentColumn = 0;
            }
            continue;
        }

        if (strcmp(argv[0], "version") == 0) {
            print_string("NexOs - Version ALPHA 3a");
            continue;
        }

        if (strcmp(argv[0], "time") == 0) {
            struct rtc_time t;
            if (rtc_read_time(&t) == 0) {
                printf("time: %dh:%dm:%ds | %d/%d/%d", t.hour, t.min, t.sec, t.day, t.month, t.year);
            }
            continue;
        }

        if (strcmp(argv[0], "sysinfo") == 0) {
            printf("NexOS ALPHA 3a\n");
            printf("Architecture: i386 (protected mode)\n");
            printf("Minimal kernel build\n");
            /* processes */
            const struct proc_entry* list = proc_list();
            printf("Processes:\n");
            for (int i = 0; i < MAX_PROCESSES; ++i) {
                if (list[i].pid) {
                    printf("  %d: %s\n", list[i].pid, list[i].name);
                }
            }
            extern char _kernel_start, _kernel_end;
            unsigned int kstart = (unsigned int)&_kernel_start;
            unsigned int kend = (unsigned int)&_kernel_end;
            unsigned int ksize = (kend > kstart) ? (kend - kstart) : 0;
            char buf[32];
            itoa(ksize, buf);
            printf("Kernel size: %s bytes\n", buf);
            continue;
        }

        if (strcmp(argv[0], "reboot") == 0) {
            __asm__ volatile ("cli");
            __asm__ volatile ("outb %0, %1" :: "a"((unsigned char)0xFE), "Nd"((unsigned short)0x64));
            for (;;) { __asm__ volatile ("hlt"); }
        }

        if (strcmp(argv[0], "stop") == 0) {
            printf("Halting CPU...\n");
            __asm__ volatile ("cli");
            for (;;) { __asm__ volatile ("hlt"); }
        }

        /* commande inconnue */
        puts_at(currentRow, 0, "commande non reconnue"); currentRow++; currentColumn = 0;
    }
}