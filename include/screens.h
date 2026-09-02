#ifndef SCREENS_H
#define SCREENS_H

#include "context.h"

void screen_init_enter(Context* ctx);
void screen_clock_enter(Context* ctx);
void screen_sensors_enter(Context* ctx);
void screen_set_time_enter(Context* ctx);
void screen_set_date_enter(Context* ctx);
void screen_set_alarm_enter(Context* ctx);
void screen_alarming_enter(Context* ctx);

void screen_clock_update(Context* ctx);
void screen_sensors_update(Context* ctx);
void screen_set_time_update(Context* ctx);
void screen_set_date_update(Context* ctx);
void screen_set_alarm_update(Context* ctx);
void screen_alarming_update(Context* ctx);

#endif 
