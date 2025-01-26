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
#define CLOCK_PIN1 14
#define DATA_PIN1 15
#define CLOCK_PIN2 12
#define DATA_PIN2 13
#define LED_STATUS 11
#define LCD_SDA 4
#define LCD_SCL 5
#define LCD_ADDR 0x27

// Factores de calibración preestablecidos (ajustar según calibración previa)
#define SCALE_FACTOR1 -478.507f
#define SCALE_FACTOR2 -478.507f

// Estados del sistema
typedef enum {
    SYSTEM_IDLE,
    WAITING_ID,
    WAITING_PASSWORD,
    SYSTEM_RUNNING
} SystemState;
float get_weight(hx711_t *hx, float scale_factor);
void process_keypad_input(char key, lcd_i2c_t *lcd);
void update_display(lcd_i2c_t *lcd, float weight1);

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
    hx711_config_t hxcfg;
    hx711_get_default_config(&hxcfg);
    hxcfg.clock_pin = CLOCK_PIN1;
    hxcfg.data_pin = DATA_PIN1;
    hx711_t hx;
    // Inicialización del HX711 como en el ejemplo original
    hx711_init(&hx, &hxcfg);
    // Encendido y configuración del HX711
    hx711_power_up(&hx, hx711_gain_128);
    hx711_wait_settle(hx711_rate_80);
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
    
    lcd_write_string(&lcd, "Sistema de Pesaj");
    lcd_set_cursor(&lcd, 1, 0);
    lcd_write_string(&lcd, "Ingrese ID:");
    printf("Ingrese ID:\n");
    
    while (true) {
        char key = get_key();
        if (key != '\0') {
            printf("Tecla presionada: %c\n", key);
            process_keypad_input(key, &lcd);
        }
        
        if (currentState == SYSTEM_RUNNING) {
            float weight1 = get_weight(&hx, SCALE_FACTOR1);
            update_display(&lcd, weight1);
        }
        
        sleep_ms(100);
    }
    
    return 0;
}

float get_weight(hx711_t *hx, float scale_factor) {
    int32_t raw_value = hx711_get_value(hx);
    return (float)raw_value / scale_factor;
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

void update_display(lcd_i2c_t *lcd, float weight1) {
    static uint32_t last_update = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (current_time - last_update >= 500) { // Actualizar cada 500ms
        char buffer[32];
        lcd_clear(lcd);
        snprintf(buffer, sizeof(buffer), "P1:%.1fg P2:%.1fg", weight1);
        lcd_write_string(lcd, buffer);
        last_update = current_time;
    }
}

// Nota: Las funciones lcd_init, lcd_clear, lcd_write_string y lcd_set_cursor
// necesitan ser implementadas según la biblioteca específica del LCD I2C que se use