#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "tusb.h"
#include "include/common.h"
#include "include/lcd_i2c.h"
#include "include/keypad.h"
#include "include/user_management.h"

// Configuración de pines
#define CLOCK_PIN 9
#define DATA_PIN_BASE 10
#define LED_STATUS 6
#define LCD_SDA 4
#define LCD_SCL 5
#define LCD_ADDR 0x27

// Factores de calibración preestablecidos (ajustar según calibración previa)
#define SCALE_FACTOR1 -229.14f
#define SCALE_FACTOR2 -202.82f
#define SCALE_FACTOR3 -201.77f 
#define SCALE_FACTOR4 -4100.11f 
// Estados del sistema
typedef enum {
    SYSTEM_IDLE,
    WAITING_ID,
    WAITING_PASSWORD,
    SYSTEM_RUNNING
} SystemState;
void get_weights(hx711_multi_t *hxm, float scale_factors[], float weights[]);
void process_keypad_input(char key, lcd_i2c_t *lcd);
void update_display(lcd_i2c_t *lcd, float weights[]);

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
    
    printf("\nIniciando sistema de pesaje...\n");
    
    // Configuración del HX711
    hx711_multi_config_t hxmcfg;
    hx711_multi_get_default_config(&hxmcfg);
    hxmcfg.clock_pin = CLOCK_PIN;
    hxmcfg.data_pin_base = DATA_PIN_BASE;
    hxmcfg.chips_len = 2; 
    // Inicialización del HX711 como en el ejemplo original
    hx711_multi_t hxm;
    hx711_multi_init(&hxm, &hxmcfg);
    // Encendido y configuración del HX711
    hx711_multi_power_up(&hxm, hx711_gain_128);
    hx711_wait_settle(hx711_gain_128);
    printf("hx711_set_gain\n");
    
    // Inicialización del LED de estado
    gpio_init(LED_STATUS);
    gpio_set_dir(LED_STATUS, GPIO_OUT);
    gpio_put(LED_STATUS, 0); // Sistema inactivo inicialmente
    printf("led_status\n");
    // Inicialización del keypad
    keypad_init();
    printf("keypad_init\n");
    // Inicialización del sistema de usuarios
    user_init();
    printf("user_init\n");
    
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(LCD_SDA, GPIO_FUNC_I2C);
    gpio_set_function(LCD_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(LCD_SDA);
    gpio_pull_up(LCD_SCL);
    // Configuración del LCD I2C
    lcd_i2c_t lcd = {
        .i2c = i2c0,
        .addr = LCD_ADDR
    };
    lcd_init(&lcd);
    printf("lcd_init\n");
    
    lcd_write_string(&lcd, "Ingrese ID:");
    printf("Ingrese ID:\n");

    float scale_factors[] = {SCALE_FACTOR1, SCALE_FACTOR2};
    float weights[2] = {0};
    while (true) {
        char key = get_key();
        if (key != '\0') {
            printf("Tecla presionada: %c\n", key);
            process_keypad_input(key, &lcd);
        }
        
        if (currentState == SYSTEM_RUNNING) {
            get_weights(&hxm, scale_factors, weights);
            update_display(&lcd, weights);
        }
        
        sleep_ms(100);
    }
    
    return 0;
}

void get_weights(hx711_multi_t *hxm, float scale_factors[], float weights[]) {
    int32_t raw_values[2];
    printf("hx711_multi_get_values\n");
    hx711_multi_get_values(hxm, raw_values);
    printf("hx711_multi_get_values\n", raw_values[0], raw_values[1]);
    
    for (int i = 0; i < 2; i++) {
        weights[i] = (float)raw_values[i] / scale_factors[i];
    }
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
                
                // Automatically switch to password input when 6 digits are entered
                if (buffer_pos == 6) {
                    strcpy(current_id, input_buffer);
                    buffer_pos = 0;
                    input_buffer[0] = '\0';
                    currentState = WAITING_PASSWORD;
                    lcd_clear(lcd);
                    printf("Ingrese Clave:\n");
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
                        printf("Sistema activo\n");
                        lcd_write_string(lcd, "Sistema Activo");
                    } else {
                        currentState = SYSTEM_IDLE;
                        buffer_pos = 0;
                        lcd_clear(lcd);
                        lcd_write_string(lcd, "Error! Reintente");
                        printf("Error! Reintente\n");
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

void update_display(lcd_i2c_t *lcd, float weights[]) {
    static uint32_t last_update = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (current_time - last_update >= 5000) { // Actualizar cada 500ms
        char buffer[32];
        lcd_clear(lcd);
        printf("P1: %.1fg P2: %.1fg P3: 0.0g P4: 0.0g\n", 
               weights[0], weights[1]);

        snprintf(buffer, sizeof(buffer), "P1:%.1fg P2:%.1fg", 
                weights[0], weights[1]);
        lcd_set_cursor(lcd, 0, 0);
        lcd_write_string(lcd, buffer);
        
        snprintf(buffer, sizeof(buffer), "P3:0.0g P4:0.0g");
        lcd_set_cursor(lcd, 1, 0);
        last_update = current_time;
    }
}
