#include "../include/lcd_i2c.h"
#include <stdio.h>
#include <string.h>

// Funciones auxiliares privadas
static void lcd_send_byte(lcd_i2c_t *lcd, uint8_t val, int mode);
static void lcd_toggle_enable(lcd_i2c_t *lcd, uint8_t val);
static void i2c_write_byte(lcd_i2c_t *lcd, uint8_t val);

static void i2c_write_byte(lcd_i2c_t *lcd, uint8_t val) {
    i2c_write_blocking(lcd->i2c, lcd->addr, &val, 1, false);
}

static void lcd_toggle_enable(lcd_i2c_t *lcd, uint8_t val) {
    sleep_us(600);
    i2c_write_byte(lcd, val | LCD_ENABLE_BIT);
    sleep_us(600);
    i2c_write_byte(lcd, val & ~LCD_ENABLE_BIT);
    sleep_us(600);
}

static void lcd_send_byte(lcd_i2c_t *lcd, uint8_t val, int mode) {
    uint8_t high = mode | (val & 0xF0) | LCD_BACKLIGHT;
    uint8_t low = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT;

    i2c_write_byte(lcd, high);
    lcd_toggle_enable(lcd, high);
    i2c_write_byte(lcd, low);
    lcd_toggle_enable(lcd, low);
}

void lcd_init(lcd_i2c_t *lcd) {
    sleep_ms(50);  // Esperar a que el LCD se inicialice
    lcd_set_backlight(lcd, 0);
    // Secuencia de inicialización
    lcd_send_byte(lcd, 0x03, LCD_COMMAND);
    lcd_send_byte(lcd, 0x03, LCD_COMMAND);
    lcd_send_byte(lcd, 0x03, LCD_COMMAND);
    lcd_send_byte(lcd, 0x02, LCD_COMMAND);

    // Configurar el LCD
    lcd_send_byte(lcd, LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND);
    lcd_send_byte(lcd, LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND);
    lcd_send_byte(lcd, LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND);
    lcd_clear(lcd);
}

void lcd_clear(lcd_i2c_t *lcd) {
    lcd_send_byte(lcd, LCD_CLEARDISPLAY, LCD_COMMAND);
    sleep_ms(2);  // Este comando necesita más tiempo
}

void lcd_home(lcd_i2c_t *lcd) {
    lcd_send_byte(lcd, LCD_RETURNHOME, LCD_COMMAND);
    sleep_ms(2);  // Este comando necesita más tiempo
}

void lcd_set_cursor(lcd_i2c_t *lcd, int line, int position) {
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_byte(lcd, val, LCD_COMMAND);
}

void lcd_write_char(lcd_i2c_t *lcd, char c) {
    lcd_send_byte(lcd, c, LCD_CHARACTER);
}

void lcd_write_string(lcd_i2c_t *lcd, const char *str) {
    while (*str) {
        lcd_write_char(lcd, *str++);
    }
}

void lcd_set_backlight(lcd_i2c_t *lcd, bool backlight) {
    if (backlight) {
        i2c_write_byte(lcd, LCD_BACKLIGHT);
    } else {
        i2c_write_byte(lcd, 0);
    }
}

void lcd_display_control(lcd_i2c_t *lcd, bool display, bool cursor, bool blink) {
    uint8_t cmd = LCD_DISPLAYCONTROL;
    if (display) cmd |= LCD_DISPLAYON;
    if (cursor) cmd |= LCD_CURSORON;
    if (blink) cmd |= LCD_BLINKON;
    lcd_send_byte(lcd, cmd, LCD_COMMAND);
}