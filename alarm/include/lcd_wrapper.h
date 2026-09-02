#ifndef LCD_WRAPPER_H
#define LCD_WRAPPER_H

#include <Arduino.h>

void lcd_init();
void lcd_clear();
void lcd_set_cursor(int col, int row);
void lcd_print(const char* message);
void lcd_print_line(int line, const char* message);

#endif 
