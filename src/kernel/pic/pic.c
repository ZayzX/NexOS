// pic.c
#include "pic.h"
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t v;
    __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

static inline void io_wait(void) {
    outb(0x80, 0x00);  // Petit délai pour le PIC
}

void pic_init(void) {
    // ICW1 : début d'initialisation
    outb(PIC1_CMD,  0x11); io_wait();
    outb(PIC2_CMD,  0x11); io_wait();

    // ICW2 : vecteurs de base (IRQ0-7 → 32-39, IRQ8-15 → 40-47)
    outb(PIC1_DATA, 0x20); io_wait();  // IRQ0 = vecteur 32
    outb(PIC2_DATA, 0x28); io_wait();  // IRQ8 = vecteur 40

    // ICW3 : cascade
    outb(PIC1_DATA, 0x04); io_wait();  // PIC1 : IRQ2 connecté au PIC2
    outb(PIC2_DATA, 0x02); io_wait();  // PIC2 : identifiant cascade

    // ICW4 : mode 8086
    outb(PIC1_DATA, 0x01); io_wait();
    outb(PIC2_DATA, 0x01); io_wait();

    // Masquer toutes les IRQ par défaut
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}

void pic_unmask_irq(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t  bit  = irq % 8;
    outb(port, inb(port) & ~(1 << bit));
}

void pic_mask_irq(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t  bit  = irq % 8;
    outb(port, inb(port) | (1 << bit));
}