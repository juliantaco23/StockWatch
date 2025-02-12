#ifndef LCD_I2C_H
#define LCD_I2C_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Comandos LCD
#define LCD_CLEARDISPLAY    0x01
#define LCD_RETURNHOME      0x02
#define LCD_ENTRYMODESET    0x04
#define LCD_DISPLAYCONTROL  0x08
#define LCD_CURSORSHIFT     0x10
#define LCD_FUNCTIONSET     0x20
#define LCD_SETCGRAMADDR    0x40
#define LCD_SETDDRAMADDR    0x80

// Flags para el modo de entrada
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYLEFT          0x02

// Flags para control de display y cursor
#define LCD_BLINKON    0x01
#define LCD_CURSORON   0x02
#define LCD_DISPLAYON  0x04

// Flags para display y desplazamiento del cursor
#define LCD_MOVERIGHT    0x04
#define LCD_DISPLAYMOVE  0x08

// Flags para set de funciones
#define LCD_5x10DOTS   0x04
#define LCD_2LINE      0x08
#define LCD_8BITMODE   0x10

// Flag para control de backlight
#define LCD_BACKLIGHT  0x08
#define LCD_ENABLE_BIT 0x04

// Modos para lcd_send_byte
#define LCD_CHARACTER  1
#define LCD_COMMAND    0

// Estructura para manejar el LCD
typedef struct {
    i2c_inst_t *i2c;
    uint8_t addr;
} lcd_i2c_t;

// Funciones de la API
void lcd_init(lcd_i2c_t *lcd);
void lcd_clear(lcd_i2c_t *lcd);
void lcd_home(lcd_i2c_t *lcd);
void lcd_set_cursor(lcd_i2c_t *lcd, int line, int position);
void lcd_write_char(lcd_i2c_t *lcd, char c);
void lcd_write_string(lcd_i2c_t *lcd, const char *str);
void lcd_set_backlight(lcd_i2c_t *lcd, bool backlight);
void lcd_display_control(lcd_i2c_t *lcd, bool display, bool cursor, bool blink);

#endif // LCD_I2C_H