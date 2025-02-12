#include "keypad.h"
#include "hardware/irq.h"
#include "hardware/sync.h"

const char teclas[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

const uint pin_columnas[] = {20, 21, 22};
const uint pin_filas[] = {16, 17, 18, 19};

volatile char last_key = '\0';
volatile bool key_pressed = false;
static volatile uint32_t last_interrupt_time = 0;
#define DEBOUNCE_TIME_MS 200

// Función de callback para la interrupción
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Debouncing simple
    if (current_time - last_interrupt_time < DEBOUNCE_TIME_MS) {
        return;
    }
    last_interrupt_time = current_time;
    
    // Encontrar qué columna generó la interrupción
    int columna = -1;
    for (int i = 0; i < 3; i++) {
        if (gpio == pin_columnas[i]) {
            columna = i;
            break;
        }
    }
    
    if (columna != -1) {
        // Escanear las filas para esta columna
        for (int fila = 0; fila < 4; fila++) {
            gpio_put(pin_filas[fila], true);
            sleep_us(10); // Pequeña espera para estabilización
            
            if (gpio_get(pin_columnas[columna])) {
                last_key = teclas[fila][columna];
                key_pressed = true;
                gpio_put(pin_filas[fila], false);
                break;
            }
            
            gpio_put(pin_filas[fila], false);
        }
    }
}

void keypad_init() {
    // Configuración de filas como salidas
    for (int i = 0; i < 4; i++) {
        gpio_init(pin_filas[i]);
        gpio_set_dir(pin_filas[i], GPIO_OUT);
        gpio_put(pin_filas[i], false);
    }
    
    // Configuración de columnas como entradas con pull-down y interrupciones
    for (int i = 0; i < 3; i++) {
        gpio_init(pin_columnas[i]);
        gpio_set_dir(pin_columnas[i], GPIO_IN);
        gpio_pull_down(pin_columnas[i]);
        
        // Habilitar interrupciones en flancos de subida
        gpio_set_irq_enabled_with_callback(pin_columnas[i], GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    }
}

char get_key() {
    if (key_pressed) {
        key_pressed = false;
        return last_key;
    }
    return '\0';
}