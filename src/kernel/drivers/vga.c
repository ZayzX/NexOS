#include <stdint.h>
#define VGA_TEXT ((volatile uint16_t*)0xB8000)

int currentColumn = 0;
int currentRow = 0;

void puts_at(int row, int col, const char* s) {
    volatile uint16_t* v = VGA_TEXT + (row * 80 + col);
    while (*s) {
        uint8_t ch = (uint8_t)*s++;
        uint8_t attr = 0x07; 
        *v++ = (uint16_t)ch | ((uint16_t)attr << 8);
    }
}

void put_char_at(int row, int col, char c) {
    volatile uint16_t* v = VGA_TEXT + (row * 80 + col);

    uint8_t ch = (uint8_t)c;
    uint8_t attr = 0x07;

    *v = (uint16_t)ch | ((uint16_t)attr << 8);
}

void clear_screen() {
    for (int i = 0; i < 80*25; i++) {
        VGA_TEXT[i] = (uint16_t)' ' | (uint16_t)(0x07 << 8);
    } 

    currentColumn = 0;
    currentRow = 0;
}

int strlen(const char* s) {
    int len = 0;
    while (*s++) len++;
    return len;
}

void itoa(int value, char* buffer) {
    char temp[12];
    int i = 0;
    int is_negative = 0;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = 0;
        return;
    }

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    while (value > 0) {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative) {
        temp[i++] = '-';
    }

    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }

    buffer[j] = 0;
}