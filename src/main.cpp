#include <Arduino.h>
#include <EEPROM.h>

#include "../include/config.h"
#include "../include/context.h"
#include "../include/lcd_wrapper.h"
#include "../include/rtc_wrapper.h"
#include "../include/sensors.h"
#include "../include/screens.h"

static Context ctx;
// настройка пин, инициализация,будильник, экран
void setup() {
    pinMode(BTN_1_PIN, INPUT_PULLUP);
    pinMode(BTN_2_PIN, INPUT_PULLUP);
    pinMode(BTN_3_PIN, INPUT_PULLUP);
    pinMode(BTN_4_PIN, INPUT_PULLUP);

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    pinMode(RGB_R_PIN, OUTPUT);
    pinMode(RGB_G_PIN, OUTPUT);
    pinMode(RGB_B_PIN, OUTPUT);
    pinMode(LED_RED_PIN,    OUTPUT);
    pinMode(LED_YELLOW_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN,   OUTPUT);

    pinMode(6, OUTPUT);
    digitalWrite(6, HIGH);

    lcd_init();
    rtc_init();
    sensors_init();

    memset(&ctx, 0, sizeof(ctx));
    uint8_t magic = EEPROM.read(EEPROM_MAGIC_ADDR);
    if (magic == EEPROM_MAGIC_VALUE) {
        lcd_init();
        lcd_print_line(0, "EEPROM OK");
        char buf[17];
        snprintf(buf, sizeof(buf), "ALM: %02d:%02d",
                EEPROM.read(EEPROM_ALARM_HOUR_ADDR),
                EEPROM.read(EEPROM_ALARM_MIN_ADDR));
        lcd_print_line(1, buf);
        delay(2000);
    } else {
        lcd_init();
        lcd_print_line(0, "EEPROM EMPTY");
        char buf[17];
        snprintf(buf, sizeof(buf), "Magic: %02X", magic);
        lcd_print_line(1, buf);
        delay(2000);
    }

    const int pins[BTN_COUNT] = {BTN_1_PIN, BTN_2_PIN, BTN_3_PIN, BTN_4_PIN};
    for (int i = 0; i < BTN_COUNT; i++) {
        ctx.last_button_state[i]    = digitalRead(pins[i]);
        ctx.current_button_state[i] = ctx.last_button_state[i];
        ctx.last_debounce_time[i]   = 0;
    }

    ctx.temperature = 0.0f;
    ctx.humidity    = 0.0f;

    if (EEPROM.read(EEPROM_MAGIC_ADDR) == EEPROM_MAGIC_VALUE) {
        ctx.alarm_hour    = EEPROM.read(EEPROM_ALARM_HOUR_ADDR);
        ctx.alarm_min     = EEPROM.read(EEPROM_ALARM_MIN_ADDR);
        ctx.alarm_enabled = (EEPROM.read(EEPROM_ALARM_ENABLED_ADDR) == 1);
    } else {
        ctx.alarm_hour    = 7;
        ctx.alarm_min     = 0;
        ctx.alarm_enabled = false;
    }

    ctx.state        = STATE_CLOCK;
    ctx.needs_redraw = true;

    screen_init_enter(&ctx);
}
// без цикл
void loop() {
    switch (ctx.state) {
        case STATE_CLOCK:      screen_clock_update(&ctx);      break;
        case STATE_SENSORS:    screen_sensors_update(&ctx);    break;
        case STATE_SET_TIME:   screen_set_time_update(&ctx);   break;
        case STATE_SET_DATE:   screen_set_date_update(&ctx);   break;
        case STATE_SET_ALARM:  screen_set_alarm_update(&ctx);  break;
        case STATE_ALARMING:   screen_alarming_update(&ctx);   break;
        default: break;
    }
    delay(20);
}