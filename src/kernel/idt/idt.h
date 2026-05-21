// idt.h
#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Une entrée dans l'IDT (8 octets)
typedef struct __attribute__((packed)) {
    uint16_t offset_low;    // Bits 0-15 de l'adresse du handler
    uint16_t selector;      // Sélecteur de segment code (0x08 en général)
    uint8_t  zero;          // Toujours 0
    uint8_t  type_attr;     // Type + attributs (0x8E = interrupt gate, ring 0)
    uint16_t offset_high;   // Bits 16-31 de l'adresse du handler
} idt_entry_t;

// Pointeur vers l'IDT (pour lidt)
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} idt_ptr_t;

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t handler, uint16_t selector, uint8_t flags);

#endif