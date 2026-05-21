#include "printf.h"
#include "../drivers/vga.h"
#include <stdarg.h>

static void k_putc(char c) {
    if (c == '\n') {
        currentRow++;
        currentColumn = 0;
        return;
    }
    put_char_at(currentRow, currentColumn, c);
    currentColumn++;
    if (currentColumn >= 80) {
        currentColumn = 0;
        currentRow++;
    }
}

static void k_puts(const char *s) {
    while (*s) k_putc(*s++);
}

int printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    const char *p = format;
    char numbuf[32];
    int written = 0;

    while (*p) {
        if (*p != '%') {
            k_putc(*p++);
            written++;
            continue;
        }

        p++; 
        if (*p == '\0') break;

        switch (*p) {
            case 'd': {
                int v = va_arg(ap, int);
                itoa(v, numbuf);
                k_puts(numbuf);
                while (numbuf[written]) { written++; } 
            } break;
            case 'x': {
                unsigned int v = va_arg(ap, unsigned int);
                char buf[16]; int i = 0;
                if (v == 0) { buf[i++] = '0'; }
                while (v) { int d = v & 0xF; buf[i++] = (d < 10) ? ('0'+d) : ('a'+d-10); v >>= 4; }
                for (int j = i-1; j >= 0; --j) k_putc(buf[j]);
                written += i;
            } break;
            case 'c': {
                char ch = (char)va_arg(ap, int);
                k_putc(ch);
                written++;
            } break;
            case 's': {
                const char *s = va_arg(ap, const char*);
                k_puts(s);
                const char *q = s; while (*q++) written++;
            } break;
            case '%': {
                k_putc('%'); written++;
            } break;
            default: {
                k_putc('%'); k_putc(*p); written += 2;
            }
        }
        p++;
    }

    va_end(ap);
    currentRow++;
    currentColumn = 0;
    return written;
}
