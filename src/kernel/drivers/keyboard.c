// keyboard.c
#include "keyboard.h"
#include "../idt/idt.h"
#include "../pic/pic.h"
#include <stdint.h>

#define KEYBOARD_DATA_PORT 0x60
#define KB_BUFFER_SIZE     256

#define KBD_SHIFT  0x01
#define KBD_CTRL   0x02
#define KBD_ALT    0x04
#define KBD_ALTGR  0x08
#define KBD_EXT    0x10

#define KBD_F1  0xF1
#define KBD_F2  0xF2
#define KBD_F3  0xF3
#define KBD_F4  0xF4
#define KBD_F5  0xF5
#define KBD_F6  0xF6
#define KBD_F7  0xF7
#define KBD_F8  0xF8
#define KBD_F9  0xF9
#define KBD_F10 0xFA
#define KBD_F11 0xFB
#define KBD_F12 0xFC

#define KBD_SUPER_L 0xA0
#define KBD_SUPER_R 0xA1
#define KBD_MENU    0xA2
#define KBD_INSERT  0xA3
#define KBD_DELETE  0xA4
#define KBD_HOME    0xA5
#define KBD_END     0xA6
#define KBD_PAGEUP  0xA7
#define KBD_PAGEDOWN 0xA8
#define KBD_UP      0xA9
#define KBD_DOWN    0xAA
#define KBD_LEFT    0xAB
#define KBD_RIGHT   0xAC
#define KBD_ENTER_NP 0xAD
#define KBD_PRINTSCR 0xAE

// --- File circulaire ---
static volatile char kb_buffer[KB_BUFFER_SIZE];
static volatile uint8_t kb_head = 0;  // Prochain caractère à lire
static volatile uint8_t kb_tail = 0;  // Prochain emplacement à écrire
static uint8_t keyboard_state = 0;

static inline uint8_t inb(uint16_t port) {
    uint8_t v;
    __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

static const char azerty_map[128] = {
    0,    0,  '&','e','"','\'', '(', '-', 'e','_','c','a', ')','=', '\b',
    '\t','a','z','e','r','t','y','u','i','o','p','^','$','\n',
    0,   'q','s','d','f','g','h','j','k','l','m','u','*',
    0,   '<','w','x','c','v','b','n',',',';',':','!',' ',  0,
    0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,   KBD_F1, KBD_F2, KBD_F3, KBD_F4, KBD_F5, KBD_F6, KBD_F7, KBD_F8, KBD_F9, KBD_F10, 0, 0, 0, 0, 0,
    0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  KBD_F11, KBD_F12
};

static const char azerty_shift_map[128] = {
    0,    0,  '1','2','3','4','5','6','7','8','9','0', 'o','+','\b',
    '\t','A','Z','E','R','T','Y','U','I','O','P','2','L','\n',
    0,   'Q','S','D','F','G','H','J','K','L','M','%','u',
    0,   '>','W','X','C','V','B','N','?','.','/','\xA7',' ',  0,
    0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,   KBD_F1, KBD_F2, KBD_F3, KBD_F4, KBD_F5, KBD_F6, KBD_F7, KBD_F8, KBD_F9, KBD_F10, 0, 0, 0, 0, 0,
    0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  KBD_F11, KBD_F12
};

static const char azerty_altgr_map[128] = {
    0,    0,  0,   '~', '#', '{', '[', '|', '`', '\\', '^', '@',  ']', '}', 0,
    0,    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, '\xA4','\n',
    0,    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,    '|', 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,    KBD_F1, KBD_F2, KBD_F3, KBD_F4, KBD_F5, KBD_F6, KBD_F7, KBD_F8, KBD_F9, KBD_F10, 0, 0, 0, 0, 0,
    0,    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  KBD_F11, KBD_F12
};

/* mapping for extended (E0) scancodes */
static char ext_map[128] = { 0 };

static void init_ext_map(void) {
    /* common extended scancodes (set 1) */
    /* numpad Enter */
    ((char*)ext_map)[0x1C] = KBD_ENTER_NP;
    /* print screen */
    ((char*)ext_map)[0x37] = KBD_PRINTSCR;
    /* navigation cluster */
    ((char*)ext_map)[0x47] = KBD_HOME;
    ((char*)ext_map)[0x48] = KBD_UP;
    ((char*)ext_map)[0x49] = KBD_PAGEUP;
    ((char*)ext_map)[0x4B] = KBD_LEFT;
    ((char*)ext_map)[0x4D] = KBD_RIGHT;
    ((char*)ext_map)[0x4F] = KBD_END;
    ((char*)ext_map)[0x50] = KBD_DOWN;
    ((char*)ext_map)[0x51] = KBD_PAGEDOWN;
    ((char*)ext_map)[0x52] = KBD_INSERT;
    ((char*)ext_map)[0x53] = KBD_DELETE;
    /* windows / menu keys */
    ((char*)ext_map)[0x5B] = KBD_SUPER_L;
    ((char*)ext_map)[0x5C] = KBD_SUPER_R;
    ((char*)ext_map)[0x5D] = KBD_MENU;
}

static void update_modifier_state(uint8_t code, uint8_t pressed, uint8_t extended) {
    if (extended) {
        if (code == 0x1D) {
            if (pressed) keyboard_state |= KBD_CTRL;
            else keyboard_state &= ~KBD_CTRL;
        } else if (code == 0x38) {
            if (pressed) keyboard_state |= KBD_ALTGR;
            else keyboard_state &= ~KBD_ALTGR;
        }
        return;
    }

    switch (code) {
        case 0x2A:
        case 0x36:
            if (pressed) keyboard_state |= KBD_SHIFT;
            else keyboard_state &= ~KBD_SHIFT;
            break;
        case 0x1D:
            if (pressed) keyboard_state |= KBD_CTRL;
            else keyboard_state &= ~KBD_CTRL;
            break;
        case 0x38:
            if (pressed) keyboard_state |= KBD_ALT;
            else keyboard_state &= ~KBD_ALT;
            break;
        case 0x3A:
            break;
    }
}

static char map_scancode(uint8_t code, uint8_t extended) {
    char c;

    if (extended) {
        char m = ext_map[code];
        if (m != 0) return m;
    }

    if (keyboard_state & KBD_ALTGR) {
        c = azerty_altgr_map[code];
    } else if (keyboard_state & KBD_SHIFT) {
        c = azerty_shift_map[code];
    } else {
        c = azerty_map[code];
    }

    if (keyboard_state & KBD_CTRL) {
        if (c >= 'a' && c <= 'z') return c - 'a' + 1;
        if (c >= 'A' && c <= 'Z') return c - 'A' + 1;
        if (c == '[') return 27;
        if (c == '\\') return 28;
        if (c == ']') return 29;
        if (c == '^') return 30;
        if (c == '_') return 31;
    }

    /* Ensure space is available even if table entry missing (scancode 0x39) */
    if (c == 0 && code == 0x39) return ' ';

    return c;
}

void keyboard_handler_main(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (scancode == 0xE0) {
        keyboard_state |= KBD_EXT;
        pic_send_eoi(1);
        return;
    }

    uint8_t extended = keyboard_state & KBD_EXT;
    keyboard_state &= ~KBD_EXT;
    uint8_t released = scancode & 0x80;
    uint8_t code = scancode & 0x7F;

    update_modifier_state(code, !released, extended);

    if (released) {
        pic_send_eoi(1);
        return;
    }

    char c = map_scancode(code, extended);
    /* Fallback: some BIOS/keyboards use scancode 0x39 for space; ensure it maps */
    if (c == 0 && code == 0x39) c = ' ';
    if (c != 0) {
        uint8_t next = (kb_tail + 1) % KB_BUFFER_SIZE;
        if (next != kb_head) {
            kb_buffer[kb_tail] = c;
            kb_tail = next;
        }
    }

    pic_send_eoi(1);  // IRQ1 → EOI obligatoire
}

void __attribute__((naked)) keyboard_handler(void) {
    __asm__ volatile (
        "pusha\n"
        "push %ds\n"
        "push %es\n"
        "push %fs\n"
        "push %gs\n"
        "mov $0x10, %ax\n"
        "mov %ax, %ds\n"
        "mov %ax, %es\n"
        "mov %ax, %fs\n"
        "mov %ax, %gs\n"
        "call keyboard_handler_main\n"
        "pop %gs\n"
        "pop %fs\n"
        "pop %es\n"
        "pop %ds\n"
        "popa\n"
        "iret\n"
    );
}

void keyboard_init(void) {
    init_ext_map();
    // Brancher le handler sur le vecteur 33 (IRQ1 = 32+1)
    idt_set_gate(33, (uint32_t)keyboard_handler, 0x08, 0x8E);

    // Autoriser IRQ1
    pic_unmask_irq(1);
}

// Bloquant : attend qu'un caractère soit disponible dans la file
char keyboard_getchar(void) {
    while (kb_head == kb_tail);  // Spin jusqu'à ce qu'un char arrive

    char c = kb_buffer[kb_head];
    kb_head = (kb_head + 1) % KB_BUFFER_SIZE;
    return c;
}