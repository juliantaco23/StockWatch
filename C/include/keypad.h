// keypad.h
#ifndef KEYPAD_H
#define KEYPAD_H

#include "pico/stdlib.h"

#define TECLA_ARRIBA 0
#define TECLA_ABAJO 1

void keypad_init();
int scan(int fila, int columna);
char get_key();

#endif // KEYPAD_H