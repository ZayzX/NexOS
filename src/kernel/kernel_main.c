#include "libs/print_char.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include "idt/idt.h"
#include "pic/pic.h"
#include "libs/print_string.h"
#include "libs/scan.h"
#include "shell/shell.h"

void kmain(void) {
    clear_screen();

    idt_init();
    pic_init();
    keyboard_init();
    __asm__ __volatile__("sti");

    shell();
    

    for (;;){ __asm__ __volatile__("hlt"); }
}
