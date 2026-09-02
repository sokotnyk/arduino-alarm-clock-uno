#ifndef CONTEXT_H
#define CONTEXT_H

#include <Arduino.h>
#include <stdbool.h>

typedef enum {
    STATE_CLOCK,        // время и дата
    STATE_SENSORS,      // температура влажность
    STATE_SET_TIME,     
    STATE_SET_DATE,     
    STATE_SET_ALARM,    
    STATE_ALARMING      
} AppState;


#define BTN_COUNT 4
#define BTN_UP    0   // первая кнопка
#define BTN_DOWN  1   
#define BTN_NEXT  2   
#define BTN_MODE  3  

typedef struct {

    AppState state;

    unsigned long last_debounce_time[BTN_COUNT];
    int           last_button_state[BTN_COUNT];
    int           current_button_state[BTN_COUNT];

    // Сенсор
    float temperature;
    float humidity;
    unsigned long last_sensor_read_ms;

    // будильник
    uint8_t alarm_hour;
    uint8_t alarm_min;
    bool    alarm_enabled;
    bool    alarm_triggered;

    // эдит
    uint8_t edit_field;   
    uint8_t edit_h;
    uint8_t edit_m;
    uint8_t edit_s;
    uint8_t edit_day;
    uint8_t edit_month;
    uint16_t edit_year;

    // рефреш
    bool needs_redraw;
} Context;

#endif
