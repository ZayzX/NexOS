#include "print_string.h"
#include "../drivers/vga.h"

void print_int(int n) {
    char buf[16];
    itoa(n, buf);
    print_string(buf);
}