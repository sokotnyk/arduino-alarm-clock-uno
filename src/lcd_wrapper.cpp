#include "../include/lcd_wrapper.h"
#include <LiquidCrystal_I2C.h>

// Global LCD object (I2C, 16, 2)
static LiquidCrystal_I2C lcd(0x27, 16, 2);

void lcd_init() {
    lcd.init();
    lcd.backlight();
    lcd_clear();
}

void lcd_clear() {
    lcd.clear();
}

void lcd_set_cursor(int col, int row) {
    lcd.setCursor(col, row);
}

void lcd_print(const char* message) {
    lcd.print(message);
}

void lcd_print_line(int line, const char* message) {
    lcd_set_cursor(0, line);
    char buffer[17];
    snprintf(buffer, sizeof(buffer), "%-16s", message);
    lcd_print(buffer);
}
