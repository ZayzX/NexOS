#ifndef TIME_H
#define TIME_H

#include <stdint.h>

struct rtc_time {
    int sec;
    int min;
    int hour;
    int day;
    int month;
    int year;
};

int rtc_read_time(struct rtc_time* t);

#endif
