// keyboard.h
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);   // À appeler au boot
char keyboard_getchar(void); // Bloquant, retourne le prochain caractère

/* Codes spéciaux renvoyés par le driver clavier */
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

#define KBD_ENTER_NP 0xAD
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

#endif