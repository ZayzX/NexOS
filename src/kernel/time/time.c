#include "time.h"
#include <stdint.h>

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t v;
    __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

static uint8_t cmos_read(uint8_t reg) {
    outb(CMOS_ADDR, (uint8_t)(reg | 0x80));
    return inb(CMOS_DATA);
}

static int bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

int rtc_read_time(struct rtc_time* t) {
    if (!t) return -1;

    while (cmos_read(0x0A) & 0x80);

    uint8_t sec = cmos_read(0x00);
    uint8_t min = cmos_read(0x02);
    uint8_t hour = cmos_read(0x04);
    uint8_t day = cmos_read(0x07);
    uint8_t month = cmos_read(0x08);
    uint8_t year = cmos_read(0x09);
    uint8_t regb = cmos_read(0x0B);

    int is_binary = regb & 0x04;
    int is_24h = regb & 0x02;

    if (!is_binary) {
        sec = (uint8_t)bcd_to_bin(sec);
        min = (uint8_t)bcd_to_bin(min);
        hour = (uint8_t)bcd_to_bin(hour & 0x7F);
        day = (uint8_t)bcd_to_bin(day);
        month = (uint8_t)bcd_to_bin(month);
        year = (uint8_t)bcd_to_bin(year);
    } else {
        hour = hour & 0x7F;
    }

    if (!is_24h) {
        int pm = hour & 0x80;
        if (pm) hour = (hour & 0x7F) + 12;
    }

    int full_year = 2000 + year;
    if (full_year < 1970) full_year += 100;

    t->sec = sec;
    t->min = min;
    t->hour = hour;
    t->day = day;
    t->month = month;
    t->year = full_year;

    return 0;
}
