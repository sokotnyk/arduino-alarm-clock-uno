#include "../include/rtc_wrapper.h"
#include "../include/config.h"
#include <ClearDS1302.h>

static ClearDS1302 rtc(RTC_DAT_PIN, RTC_RST_PIN, RTC_CLK_PIN);

void rtc_init() {
    rtc.set.WriteProtect(false);
    // Если время мусор - установить дефолтное
    byte h = rtc.get.time.hour().toInt();
    if (h > 23) {
        rtc.set.time.second(0);
        rtc.set.time.minutes(0);
        rtc.set.time.hour(12);
        rtc.set.time.date(1);
        rtc.set.time.month(1);
        rtc.set.time.year(26);
        rtc.set.time.day(1);
    }
}

RtcTime rtc_get_time() {
    RtcTime rt;
    rt.hour        = rtc.get.time.hour().toInt();
    rt.min         = rtc.get.time.minutes();
    rt.sec         = rtc.get.time.second();
    rt.day         = rtc.get.time.date();
    rt.month       = rtc.get.time.month();
    rt.year        = 2000 + rtc.get.time.year();
    rt.day_of_week = rtc.get.time.day();
    if (rt.hour > 23) rt.hour = 0;
    if (rt.min  > 59) rt.min  = 0;
    if (rt.sec  > 59) rt.sec  = 0;
    if (rt.day  < 1 || rt.day > 31)   rt.day   = 1;
    if (rt.month < 1 || rt.month > 12) rt.month = 1;
    if (rt.year < 2000 || rt.year > 2099) rt.year = 2026;
    return rt;
}

void rtc_set_time(const RtcTime* rt) {
    rtc.set.WriteProtect(false);
    rtc.set.time.second(rt->sec);
    rtc.set.time.minutes(rt->min);
    rtc.set.time.hour(rt->hour);
    rtc.set.time.date(rt->day);
    rtc.set.time.month(rt->month);
    rtc.set.time.year(rt->year - 2000);
    rtc.set.time.day(rt->day_of_week);
}