#ifndef VGA_H
#define VGA_H

#include <stdint.h>

extern int currentRow;
extern int currentColumn;

void puts_at(int row, int col, const char* s);
void put_char_at(int row, int col, char c);
void clear_screen();
int strlen(const char* s);
void itoa(int value, char* buffer);

#endif