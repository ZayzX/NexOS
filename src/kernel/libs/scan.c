#include "scan.h"
#include "../drivers/keyboard.h"

static int string_to_int(const char* s) {
    int value = 0;
    int sign = 1;

    if (*s == '-') {
        sign = -1;
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        value = value * 10 + (*s - '0');
        s++;
    }

    return value * sign;
}

char scan_char(void) {
    return keyboard_getchar();
}

void scan_string(char* buffer, int maxlen) {
    if (maxlen <= 0) {
        return;
    }

    int index = 0;
    while (1) {
        char c = keyboard_getchar();

        if (c == '\r' || c == '\n') {
            break;
        }

        if (c == '\b') {
            if (index > 0) {
                index--;
            }
            continue;
        }

        if (index < maxlen - 1) {
            buffer[index++] = c;
        }
    }

    buffer[index] = '\0';
}

int scan_int(void) {
    char buffer[16];
    scan_string(buffer, sizeof(buffer));
    return string_to_int(buffer);
}
