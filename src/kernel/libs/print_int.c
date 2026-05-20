#include "print.h"

void print_int(int n) {
    char buf[16];
    itoa(n, buf);
    print(buf);
}