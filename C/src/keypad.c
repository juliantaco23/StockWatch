#include "keypad.h"

const char teclas[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

const uint pin_columnas[] = {20, 21, 22};
const uint pin_filas[] = {16, 17, 18, 19};

void keypad_init() {
    for (int i = 0; i < 4; i++) {
        gpio_init(pin_filas[i]);
        gpio_set_dir(pin_filas[i], GPIO_OUT);
        gpio_put(pin_filas[i], false);
    }
    for (int i = 0; i < 3; i++) {
        gpio_init(pin_columnas[i]);
        gpio_set_dir(pin_columnas[i], GPIO_IN);
        gpio_pull_down(pin_columnas[i]);
    }
}

int scan(int fila, int columna) {
    gpio_put(pin_filas[fila], true);
    int tecla = TECLA_ARRIBA;

    if (gpio_get(pin_columnas[columna])) {
        tecla = TECLA_ABAJO;
        sleep_ms(500);
    }

    gpio_put(pin_filas[fila], false);

    return tecla;
}

char get_key() {
    for (int fila = 0; fila < 4; fila++) {
        for (int columna = 0; columna < 3; columna++) {
            if (scan(fila, columna) == TECLA_ABAJO) {
                return teclas[fila][columna];
            }
        }
    }
    return '\0';  // No key pressed
}