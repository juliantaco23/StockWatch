#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "tusb.h"
#include "include/common.h"
#include "include/lcd_i2c.h"
#include "include/keypad.h"
#include "include/user_management.h"
#include "hardware/uart.h"
// Configuración de pines
#define CLOCK_PIN 9
#define DATA_PIN 10
#define LED_STATUS 6
#define LCD_SDA 4
#define LCD_SCL 5
#define LCD_ADDR 0x27
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1
// Factor de calibración para la celda única (ajustar según calibración)
#define SCALE_FACTOR -229.48f  

// Estados del sistema
typedef enum {
    SYSTEM_IDLE,
    WAITING_ID,
    WAITING_PASSWORD,
    SYSTEM_RUNNING
} SystemState;

void get_weight(hx711_t *hx, float scale_factor, float *weight);
void process_keypad_input(char key, lcd_i2c_t *lcd);
void update_display(lcd_i2c_t *lcd, float weight);

// Variables globales
static SystemState currentState = SYSTEM_IDLE;
static char input_buffer[10] = {0};
static int buffer_pos = 0;
static char current_id[7] = {0};

int main() {
    stdio_init_all();
    
    // Esperar a que el USB CDC esté listo
    while (!tud_cdc_connected()) {
        sleep_ms(100);
    }
    sleep_ms(1000);  // Dar tiempo adicional para que la conexión se establezca
    printf("\nIniciando uart\n");
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    printf("\nIniciando sistema de pesaje...\n");
    
    // Configuración del HX711 para una sola celda
    hx711_config_t hxcfg;
    hx711_get_default_config(&hxcfg);
    hxcfg.clock_pin = CLOCK_PIN;
    hxcfg.data_pin = DATA_PIN;
    
    hx711_t hx;
    hx711_init(&hx, &hxcfg);
    hx711_power_up(&hx, hx711_gain_128);
    hx711_wait_settle(hx711_gain_128);
    printf("HX711 inicializado\n");
    
    // Inicialización del LED de estado
    gpio_init(LED_STATUS);
    gpio_set_dir(LED_STATUS, GPIO_OUT);
    gpio_put(LED_STATUS, 0); // Sistema inactivo inicialmente
    
    // Inicialización del keypad
    keypad_init();
    
    // Inicialización del sistema de usuarios
    user_init();
    
    // Inicialización del LCD I2C
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(LCD_SDA, GPIO_FUNC_I2C);
    gpio_set_function(LCD_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(LCD_SDA);
    gpio_pull_up(LCD_SCL);
    lcd_i2c_t lcd = {
        .i2c = i2c0,
        .addr = LCD_ADDR
    };
    lcd_init(&lcd);
    lcd_write_string(&lcd, "Ingrese ID:");

    float weight = 0.0f;
    while (true) {
        char key = get_key();
        if (key != '\0') {
            printf("Tecla presionada: %c\n", key);
            process_keypad_input(key, &lcd);
        }
        
        if (currentState == SYSTEM_RUNNING) {
            get_weight(&hx, SCALE_FACTOR, &weight);
            update_display(&lcd, weight);
        }
        
        sleep_ms(100);
    }
    
    return 0;
}

void get_weight(hx711_t *hx, float scale_factor, float *weight) {
    int32_t raw_value = hx711_get_value(hx);
    *weight = ((float)raw_value / scale_factor) - 300.0f;
}

void send_weight_uart(float weight) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.1f\n", weight);
    uart_puts(UART_ID, buffer);
}

void process_keypad_input(char key, lcd_i2c_t *lcd) {
    switch (currentState) {
        case SYSTEM_IDLE:
        case WAITING_ID:
            if (buffer_pos < 6 && key >= '0' && key <= '9') {
                input_buffer[buffer_pos++] = key;
                input_buffer[buffer_pos] = '\0';
                lcd_set_cursor(lcd, 1, 0);
                lcd_write_string(lcd, input_buffer);
                
                if (buffer_pos == 6) {
                    strcpy(current_id, input_buffer);
                    buffer_pos = 0;
                    input_buffer[0] = '\0';
                    currentState = WAITING_PASSWORD;
                    lcd_clear(lcd);
                    lcd_write_string(lcd, "Ingrese Clave:");
                }
            }
            break;
            
        case WAITING_PASSWORD:
            if (buffer_pos < 4 && key >= '0' && key <= '9') {
                input_buffer[buffer_pos++] = key;
                input_buffer[buffer_pos] = '\0';
                lcd_set_cursor(lcd, 1, 0);
                lcd_write_string(lcd, "****");
            
                if (buffer_pos == 4) {
                    if (verify_user(current_id, input_buffer)) {
                        currentState = SYSTEM_RUNNING;
                        gpio_put(LED_STATUS, 1); // Sistema activo
                        lcd_clear(lcd);
                        lcd_write_string(lcd, "Sistema Activo");
                    } else {
                        currentState = SYSTEM_IDLE;
                        buffer_pos = 0;
                        lcd_clear(lcd);
                        lcd_write_string(lcd, "Error! Reintente");
                        sleep_ms(2000);
                        lcd_clear(lcd);
                        lcd_write_string(lcd, "Ingrese ID:");
                    }
                }
            }  
            break;
            
        case SYSTEM_RUNNING:
            if (key == '*') {
                currentState = SYSTEM_IDLE;
                buffer_pos = 0;
                gpio_put(LED_STATUS, 0); // Sistema inactivo
                lcd_clear(lcd);
                lcd_write_string(lcd, "Sistema Inactivo");
                sleep_ms(2000);
                lcd_clear(lcd);
                lcd_write_string(lcd, "Ingrese ID:");
            }
            break;
    }
}

void update_display(lcd_i2c_t *lcd, float weight) {
    static uint32_t last_update = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (current_time - last_update >= 500) { // Actualizar cada 500 ms
        char buffer[32];
        lcd_clear(lcd);
        snprintf(buffer, sizeof(buffer), "Peso: %.1f g", weight);
        lcd_set_cursor(lcd, 0, 0);
        lcd_write_string(lcd, buffer);
        send_weight_uart(weight);
        last_update = current_time;
    }
}
