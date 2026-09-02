#ifndef RTC_WRAPPER_H
#define RTC_WRAPPER_H

#include <Arduino.h>

typedef struct {
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
    uint8_t  day;
    uint8_t  month;
    uint16_t year;
    uint8_t  day_of_week;  
} RtcTime;

void    rtc_init();
RtcTime rtc_get_time();
void    rtc_set_time(const RtcTime* t);

#endif 