#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lcd_i2c.h"

// Configuración de pines I2C
#define I2C_SDA 4
#define I2C_SCL 5
#define LCD_ADDR 0x27

int main() {
    stdio_init_all();
    
    // Inicializar I2C
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    // Configurar estructura del LCD
    lcd_i2c_t lcd = {
        .i2c = i2c0,
        .addr = LCD_ADDR
    };
    
    // Inicializar LCD
    sleep_ms(100);  // Esperar a que el LCD esté listo
    lcd_init(&lcd);
    
    // Mensajes de prueba
    const char *mensajes[] = {
        "Prueba LCD 16x2",
        "LCD Funcionando",
        "Test I2C OK",
        "Hola Mundo!",
        "Test Linea 1",
        "Test Linea 2"
    };
    int num_mensajes = sizeof(mensajes) / sizeof(mensajes[0]);
    int contador = 0;
    
    while (1) {
        // Mostrar mensaje en primera línea
        lcd_clear(&lcd);
        lcd_set_cursor(&lcd, 0, 0);
        lcd_write_string(&lcd, mensajes[contador % num_mensajes]);
        
        // Mostrar contador en segunda línea
        lcd_set_cursor(&lcd, 1, 0);
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "Count: %d", contador);
        lcd_write_string(&lcd, buffer);
        
        // Extras para demostrar las nuevas funciones de la librería
        if (contador % 5 == 0) {
            // Cada 5 conteos, parpadear el backlight
            lcd_set_backlight(&lcd, false);
            sleep_ms(200);
            lcd_set_backlight(&lcd, true);
        }
        
        // Cada 10 conteos, mostrar el cursor parpadeante por un momento
        if (contador % 10 == 0) {
            lcd_display_control(&lcd, true, true, true);  // Display on, cursor on, blink on
            sleep_ms(2000);
            lcd_display_control(&lcd, true, false, false);  // Volver a normal
        }
        
        contador++;
        sleep_ms(1000);  // Esperar 1 segundo entre mensajes
    }
    
    return 0;
}