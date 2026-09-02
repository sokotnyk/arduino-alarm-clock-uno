#include "../../include/screens.h"
#include "../../include/lcd_wrapper.h"
#include "../../include/rtc_wrapper.h"
#include "../../include/sensors.h"
#include "../../include/config.h"
#include <Arduino.h>
#include <EEPROM.h>


// ─── Helpers ─────────────────────────────────────────────────────────────────

// Re-use debounce logic from mastermind — same algorithm, same signature
static bool is_button_pressed(int btn_idx, int pin, Context* ctx) {
    int reading = digitalRead(pin);

    if (reading != ctx->last_button_state[btn_idx]) {
        ctx->last_debounce_time[btn_idx] = millis();
    }

    if ((millis() - ctx->last_debounce_time[btn_idx]) > DEBOUNCE_DELAY_MS) {
        if (reading != ctx->current_button_state[btn_idx]) {
            ctx->current_button_state[btn_idx] = reading;
            if (ctx->current_button_state[btn_idx] == LOW) {
                ctx->last_button_state[btn_idx] = reading;
                return true;
            }
        }
    }

    ctx->last_button_state[btn_idx] = reading;
    return false;
}

static const int BTN_PINS[BTN_COUNT] = {BTN_1_PIN, BTN_2_PIN, BTN_3_PIN, BTN_4_PIN};

static inline bool btn_up(Context* ctx)   { return is_button_pressed(BTN_UP,   BTN_PINS[BTN_UP],   ctx); }
static inline bool btn_down(Context* ctx) { return is_button_pressed(BTN_DOWN, BTN_PINS[BTN_DOWN], ctx); }
static inline bool btn_next(Context* ctx) { return is_button_pressed(BTN_NEXT, BTN_PINS[BTN_NEXT], ctx); }
static inline bool btn_mode(Context* ctx) { return is_button_pressed(BTN_MODE, BTN_PINS[BTN_MODE], ctx); }

// RGB LED helper
static void rgb_set(bool r, bool g, bool b) {
    digitalWrite(RGB_R_PIN, r ? HIGH : LOW);
    digitalWrite(RGB_G_PIN, g ? HIGH : LOW);
    digitalWrite(RGB_B_PIN, b ? HIGH : LOW);
}

static void leds_set(bool red, bool yellow, bool blue) {
    digitalWrite(LED_RED_PIN,    red    ? HIGH : LOW);
    digitalWrite(LED_YELLOW_PIN, yellow ? HIGH : LOW);
    digitalWrite(LED_BLUE_PIN,   blue   ? HIGH : LOW);
}


// ─── STATE_CLOCK ─────────────────────────────────────────────────────────────

void screen_clock_enter(Context* ctx) {
    lcd_clear();
    ctx->needs_redraw = true;
    rgb_set(false, ctx->alarm_enabled, false);
    leds_set(false, false, true);
}

void screen_clock_update(Context* ctx) {
    static RtcTime last = {255, 255, 255, 0, 0, 0, 0};
    RtcTime now = rtc_get_time();

    // Сброс флага будильника когда минута меняется
    if (now.min != last.min) {
        ctx->alarm_triggered = false;
    }

    // Перерисовка только когда время меняется
    if (ctx->needs_redraw ||
        now.hour != last.hour || now.min != last.min || now.sec != last.sec ||
        now.day  != last.day  || now.month != last.month) {

        char line0[17], line1[17];
        snprintf(line0, sizeof(line0), "%02d:%02d:%02d  %s",
                 now.hour, now.min, now.sec,
                 ctx->alarm_enabled ? "ALM" : "   ");
        snprintf(line1, sizeof(line1), "%02d.%02d.%04d",
                 now.day, now.month, now.year);

        lcd_print_line(0, line0);
        lcd_print_line(1, line1);

        last = now;
        ctx->needs_redraw = false;
    }

    // Проверка будильника
    if (ctx->alarm_enabled &&
        !ctx->alarm_triggered &&
        now.hour == ctx->alarm_hour &&
        now.min  == ctx->alarm_min) {
        ctx->alarm_triggered = true;
        ctx->state           = STATE_ALARMING;
        screen_alarming_enter(ctx);
        return;
    }

    // BTN_MODE → датчики
    if (btn_mode(ctx)) {
        ctx->state = STATE_SENSORS;
        screen_sensors_enter(ctx);
        return;
    }

    // BTN_NEXT → установка времени
    if (btn_next(ctx)) {
        ctx->state = STATE_SET_TIME;
        screen_set_time_enter(ctx);
        return;
    }

    // BTN_UP → включить/выключить будильник
    if (btn_up(ctx)) {
        ctx->alarm_enabled = !ctx->alarm_enabled;
        EEPROM.write(EEPROM_ALARM_ENABLED_ADDR, ctx->alarm_enabled ? 1 : 0);
        rgb_set(false, ctx->alarm_enabled, false);
        leds_set(false, false, true);
        ctx->needs_redraw = true;
    }
}

// ─── STATE_SENSORS ───────────────────────────────────────────────────────────

void screen_sensors_enter(Context* ctx) {
    // Force a fresh sensor read immediately
    sensors_read(&ctx->temperature, &ctx->humidity);
    rgb_set(false, true, true); // голубой
    leds_set(false, true, false);
    ctx->last_sensor_read_ms = millis();
    ctx->needs_redraw = true;
}

void screen_sensors_update(Context* ctx) {
    // Re-read every 2 seconds
    if (millis() - ctx->last_sensor_read_ms >= 2000UL) {
        sensors_read(&ctx->temperature, &ctx->humidity);
        ctx->last_sensor_read_ms = millis();
        ctx->needs_redraw = true;
    }

    if (ctx->needs_redraw) {
        char line0[17], line1[17];
        snprintf(line0, sizeof(line0), "Temp:  %5.1f %cC", ctx->temperature, (char)223);
        snprintf(line1, sizeof(line1), "Humid: %5.1f %%",  ctx->humidity);
        lcd_print_line(0, line0);
        lcd_print_line(1, line1);
        ctx->needs_redraw = false;
    }

    // BTN_MODE → back to clock
    if (btn_mode(ctx)) {
        ctx->state = STATE_CLOCK;
        screen_clock_enter(ctx);
    }
}

// ─── STATE_SET_TIME ───────────────────────────────────────────────────────────
// Fields: 0=hour, 1=min, 2=sec  →  BTN_NEXT confirms each, after sec saves

void screen_set_time_enter(Context* ctx) {
    RtcTime now = rtc_get_time();
    ctx->edit_h     = now.hour;
    ctx->edit_m     = now.min;
    ctx->edit_s     = now.sec;
    ctx->edit_field = 0;
    ctx->needs_redraw = true;
    rgb_set(true, false, false); // красный
    leds_set(true, false, false);
}

void screen_set_time_update(Context* ctx) {
    if (ctx->needs_redraw) {
        char line0[17], line1[17];
        snprintf(line0, sizeof(line0), "Set Time:       ");
        snprintf(line1, sizeof(line1), "%02d:%02d:%02d  [%c]",
                 ctx->edit_h, ctx->edit_m, ctx->edit_s,
                 ctx->edit_field == 0 ? 'H' : ctx->edit_field == 1 ? 'M' : 'S');
        lcd_print_line(0, line0);
        lcd_print_line(1, line1);
        ctx->needs_redraw = false;
    }

    // Джойстик
    static unsigned long last_joy_move = 0;
    int joy_y = analogRead(JOYSTICK_Y_PIN);
    if (millis() - last_joy_move > 250) {
        if (joy_y < (JOY_CENTER - JOY_THRESHOLD)) {
            // Вверх
            if      (ctx->edit_field == 0) { ctx->edit_h = (ctx->edit_h + 1) % 24; }
            else if (ctx->edit_field == 1) { ctx->edit_m = (ctx->edit_m + 1) % 60; }
            else                           { ctx->edit_s = (ctx->edit_s + 1) % 60; }
            ctx->needs_redraw = true;
            last_joy_move = millis();
        } else if (joy_y > (JOY_CENTER + JOY_THRESHOLD)) {
            // Вниз
            if      (ctx->edit_field == 0) { ctx->edit_h = (ctx->edit_h + 23) % 24; }
            else if (ctx->edit_field == 1) { ctx->edit_m = (ctx->edit_m + 59) % 60; }
            else                           { ctx->edit_s = (ctx->edit_s + 59) % 60; }
            ctx->needs_redraw = true;
            last_joy_move = millis();
        }
    }

    // Кнопки
    if (btn_up(ctx)) {
        if      (ctx->edit_field == 0) { ctx->edit_h = (ctx->edit_h + 1) % 24; }
        else if (ctx->edit_field == 1) { ctx->edit_m = (ctx->edit_m + 1) % 60; }
        else                           { ctx->edit_s = (ctx->edit_s + 1) % 60; }
        ctx->needs_redraw = true;
    }

    if (btn_down(ctx)) {
        if      (ctx->edit_field == 0) { ctx->edit_h = (ctx->edit_h + 23) % 24; }
        else if (ctx->edit_field == 1) { ctx->edit_m = (ctx->edit_m + 59) % 60; }
        else                           { ctx->edit_s = (ctx->edit_s + 59) % 60; }
        ctx->needs_redraw = true;
    }

    if (btn_next(ctx)) {
        if (ctx->edit_field < 2) {
            ctx->edit_field++;
            ctx->needs_redraw = true;
        } else {
            ctx->state = STATE_SET_DATE;
            screen_set_date_enter(ctx);
            return;
        }
    }

    if (btn_mode(ctx)) {
        ctx->state = STATE_CLOCK;
        screen_clock_enter(ctx);
    }
}

// ─── STATE_SET_DATE ───────────────────────────────────────────────────────────

void screen_set_date_enter(Context* ctx) {
    RtcTime now = rtc_get_time();
    ctx->edit_day   = (now.day   >= 1 && now.day   <= 31) ? now.day   : 1;
    ctx->edit_month = (now.month >= 1 && now.month <= 12) ? now.month : 1;
    ctx->edit_year  = (now.year  >= 2000 && now.year <= 2099) ? now.year : 2026;
    // НЕ трогаем edit_h, edit_m, edit_s - они уже установлены в SET TIME!
    ctx->edit_field = 0;
    ctx->needs_redraw = true;
    rgb_set(true, false, false);
    leds_set(true, false, false);
}

void screen_set_date_update(Context* ctx) {
    if (ctx->needs_redraw) {
        char line0[17], line1[17];
        snprintf(line0, sizeof(line0), "Set Date:       ");
        snprintf(line1, sizeof(line1), "%02d.%02d.%04d [%c]",
                 ctx->edit_day, ctx->edit_month, ctx->edit_year,
                 ctx->edit_field == 0 ? 'D' : ctx->edit_field == 1 ? 'M' : 'Y');
        lcd_print_line(0, line0);
        lcd_print_line(1, line1);
        ctx->needs_redraw = false;
    }

    // Джойстик
    static unsigned long last_joy_move = 0;
    int joy_y = analogRead(JOYSTICK_Y_PIN);
    if (millis() - last_joy_move > 250) {
        if (joy_y < (JOY_CENTER - JOY_THRESHOLD)) {
            if      (ctx->edit_field == 0) { ctx->edit_day   = (ctx->edit_day   % 31) + 1; }
            else if (ctx->edit_field == 1) { ctx->edit_month = (ctx->edit_month % 12) + 1; }
            else                           { ctx->edit_year++; }
            ctx->needs_redraw = true;
            last_joy_move = millis();
        } else if (joy_y > (JOY_CENTER + JOY_THRESHOLD)) {
            if      (ctx->edit_field == 0) { ctx->edit_day   = ctx->edit_day   > 1 ? ctx->edit_day   - 1 : 31; }
            else if (ctx->edit_field == 1) { ctx->edit_month = ctx->edit_month > 1 ? ctx->edit_month - 1 : 12; }
            else                           { if (ctx->edit_year > 2000) ctx->edit_year--; }
            ctx->needs_redraw = true;
            last_joy_move = millis();
        }
    }

    // Кнопки
    if (btn_up(ctx)) {
        if      (ctx->edit_field == 0) { ctx->edit_day   = (ctx->edit_day   % 31) + 1; }
        else if (ctx->edit_field == 1) { ctx->edit_month = (ctx->edit_month % 12) + 1; }
        else                           { ctx->edit_year++; }
        ctx->needs_redraw = true;
    }

    if (btn_down(ctx)) {
        if      (ctx->edit_field == 0) { ctx->edit_day   = ctx->edit_day   > 1 ? ctx->edit_day   - 1 : 31; }
        else if (ctx->edit_field == 1) { ctx->edit_month = ctx->edit_month > 1 ? ctx->edit_month - 1 : 12; }
        else                           { if (ctx->edit_year > 2000) ctx->edit_year--; }
        ctx->needs_redraw = true;
    }

    if (btn_next(ctx)) {
        if (ctx->edit_field < 2) {
            ctx->edit_field++;
            ctx->needs_redraw = true;
        } else {
            RtcTime t;
            t.hour        = ctx->edit_h;
            t.min         = ctx->edit_m;
            t.sec         = ctx->edit_s;
            t.day         = ctx->edit_day;
            t.month       = ctx->edit_month;
            t.year        = ctx->edit_year;
            t.day_of_week = 1;
            rtc_set_time(&t);
            lcd_clear();
            ctx->state      = STATE_SET_ALARM;
            ctx->edit_field = 0;
            ctx->edit_h     = ctx->alarm_hour;
            ctx->edit_m     = ctx->alarm_min;
            ctx->needs_redraw = true;
            screen_set_alarm_enter(ctx);
            return;
        }
    }

    if (btn_mode(ctx)) {
        ctx->state = STATE_CLOCK;
        screen_clock_enter(ctx);
    }
}

// ─── STATE_SET_ALARM ─────────────────────────────────────────────────────────
// Fields: 0=hour, 1=min

void screen_set_alarm_enter(Context* ctx) {
    ctx->edit_h     = ctx->alarm_hour;
    ctx->edit_m     = ctx->alarm_min;
    ctx->edit_field = 0;
    ctx->needs_redraw = true;
    rgb_set(true, false, true); // фиолетовый
    leds_set(true, false, false);
}

void screen_set_alarm_update(Context* ctx) {
    if (ctx->needs_redraw) {
        char line0[17], line1[17];
        snprintf(line0, sizeof(line0), "Set Alarm:  %s",
                 ctx->alarm_enabled ? "ON " : "OFF");
        snprintf(line1, sizeof(line1), "%02d:%02d      [%c]",
                 ctx->edit_h, ctx->edit_m,
                 ctx->edit_field == 0 ? 'H' : 'M');
        lcd_print_line(0, line0);
        lcd_print_line(1, line1);
        ctx->needs_redraw = false;
    }

    // Джойстик
    static unsigned long last_joy_move = 0;
    int joy_y = analogRead(JOYSTICK_Y_PIN);
    if (millis() - last_joy_move > 250) {
        if (joy_y < (JOY_CENTER - JOY_THRESHOLD)) {
            if (ctx->edit_field == 0) { ctx->edit_h = (ctx->edit_h + 1) % 24; }
            else                      { ctx->edit_m = (ctx->edit_m + 1) % 60; }
            ctx->needs_redraw = true;
            last_joy_move = millis();
        } else if (joy_y > (JOY_CENTER + JOY_THRESHOLD)) {
            if (ctx->edit_field == 0) { ctx->edit_h = (ctx->edit_h + 23) % 24; }
            else                      { ctx->edit_m = (ctx->edit_m + 59) % 60; }
            ctx->needs_redraw = true;
            last_joy_move = millis();
        }
    }

    // Кнопки
    if (btn_up(ctx)) {
        if (ctx->edit_field == 0) { ctx->edit_h = (ctx->edit_h + 1) % 24; }
        else                      { ctx->edit_m = (ctx->edit_m + 1) % 60; }
        ctx->needs_redraw = true;
    }

    if (btn_down(ctx)) {
        if (ctx->edit_field == 0) { ctx->edit_h = (ctx->edit_h + 23) % 24; }
        else                      { ctx->edit_m = (ctx->edit_m + 59) % 60; }
        ctx->needs_redraw = true;
    }

    if (btn_next(ctx)) {
        if (ctx->edit_field == 0) {
            ctx->edit_field = 1;
            ctx->needs_redraw = true;
        } else {
            ctx->alarm_hour    = ctx->edit_h;
            ctx->alarm_min     = ctx->edit_m;
            ctx->alarm_enabled = true;
            EEPROM.write(EEPROM_ALARM_HOUR_ADDR,    ctx->alarm_hour);
            EEPROM.write(EEPROM_ALARM_MIN_ADDR,     ctx->alarm_min);
            EEPROM.write(EEPROM_ALARM_ENABLED_ADDR, 1);
            EEPROM.write(EEPROM_MAGIC_ADDR,         EEPROM_MAGIC_VALUE);
            lcd_clear();
            ctx->state = STATE_CLOCK;
            screen_clock_enter(ctx);
            return;
        }
    }

    if (btn_mode(ctx)) {
        ctx->alarm_enabled = !ctx->alarm_enabled;
        EEPROM.write(EEPROM_ALARM_ENABLED_ADDR, ctx->alarm_enabled ? 1 : 0);
        EEPROM.write(EEPROM_MAGIC_ADDR,         EEPROM_MAGIC_VALUE);
        lcd_clear();
        ctx->state = STATE_CLOCK;
        screen_clock_enter(ctx);
    }
}
// ─── STATE_ALARMING ──────────────────────────────────────────────────────────

void screen_alarming_enter(Context* ctx) {
    lcd_clear();
    lcd_print_line(0, "*** ALARM!!! ***");
    char line1[17];
    snprintf(line1, sizeof(line1), "  %02d:%02d  WAKE UP", ctx->alarm_hour, ctx->alarm_min);
    lcd_print_line(1, line1);
    rgb_set(true, false, false);
    leds_set(true, true, true);  // Red LED = alarming
}

void screen_alarming_update(Context* ctx) {
    static unsigned long last_beep = 0;
    static bool buzzer_on = false;

    unsigned long now_ms = millis();

    if (buzzer_on && (now_ms - last_beep >= ALARM_BUZZ_DURATION_MS)) {
        digitalWrite(BUZZER_PIN, LOW);
        buzzer_on = false;
        last_beep = now_ms;
    } else if (!buzzer_on && (now_ms - last_beep >= ALARM_PAUSE_MS)) {
        digitalWrite(BUZZER_PIN, HIGH);
        buzzer_on = true;
        last_beep = now_ms;
    }

    // Сброс debounce для всех кнопок
    const int pins[BTN_COUNT] = {BTN_1_PIN, BTN_2_PIN, BTN_3_PIN, BTN_4_PIN};
    for (int i = 0; i < BTN_COUNT; i++) {
        if (digitalRead(pins[i]) == LOW) {
            digitalWrite(BUZZER_PIN, LOW);
            ctx->alarm_triggered = true;
            rgb_set(false, ctx->alarm_enabled, false);
            leds_set(false, false, true);
            ctx->state = STATE_CLOCK;
            screen_clock_enter(ctx);
            return;
        }
    }
}
