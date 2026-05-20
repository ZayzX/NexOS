#include "../drivers/vga.h"

void print(const char* s) {
    puts_at(currentRow, currentColumn, s);

    currentRow++;
}