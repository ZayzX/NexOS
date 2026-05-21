#include "../drivers/vga.h"

void print_char(char c) {
    put_char_at(currentRow, currentColumn, c);

    currentRow++;
}