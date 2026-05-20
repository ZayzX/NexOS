#include "libs/print.h"
#include "drivers/vga.h"
#include "libs/print_int.h"

void kmain(void) {
    clear_screen();
    
    print_int(5);
    print("hello");
    

    for (;;){ __asm__ __volatile__("hlt"); }
}
