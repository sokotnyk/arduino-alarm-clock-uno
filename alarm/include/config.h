#ifndef CONFIG_H
#define CONFIG_H

// КНОПКИ
#define BTN_1_PIN   5   // ВЕРХ
#define BTN_2_PIN   4   // DOWN / decrement
#define BTN_3_PIN   3   // NEXT field / confirm
#define BTN_4_PIN   2   // MODE / back

// ПИЩАЛКА
#define BUZZER_PIN  8

// ЛАМПОЧКИ
#define RGB_R_PIN   9
#define RGB_G_PIN   10
#define RGB_B_PIN   11

// DHT 11
#define DHT_PIN     7
#define DHT_TYPE    DHT11

// DS1302 RTC
#define RTC_CLK_PIN A1
#define RTC_DAT_PIN A2
#define RTC_RST_PIN A3

// ─── EEPROM address
#define EEPROM_ALARM_HOUR_ADDR   0
#define EEPROM_ALARM_MIN_ADDR    1
#define EEPROM_ALARM_ENABLED_ADDR 2
#define EEPROM_MAGIC_ADDR        3
#define EEPROM_MAGIC_VALUE       0xAB


#define DEBOUNCE_DELAY_MS  50

// САМ БУДИЛЬНИК
#define ALARM_BUZZ_DURATION_MS  500
#define ALARM_PAUSE_MS          500

#define LED_RED_PIN    6
#define LED_YELLOW_PIN 13
#define LED_BLUE_PIN   12

#define JOYSTICK_Y_PIN  A0
#define JOY_THRESHOLD   200  // порог срабатывания
#define JOY_CENTER      512  // центр джойстика

#endif 
