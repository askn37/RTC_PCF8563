/***************
 *
 * CenturyLeapDay - century bit test sample
 *
 * target architectures: Atmel AVR (ATmega 328P, 1284P and other)
 *
 * release site: https://github.com/askn37/GPS_MTK333X
 * maintainer: askn https://twitter.com/askn37
 *
 */

#include <Arduino.h>
#include <IntervalEvent.h>
#include "RTC_PCF8563.h"

#define CONSOLE_BAUD    9600

RTC_PCF8563 RTC;
IntervalEvent event;

void setup (void) {
    Serial.begin(CONSOLE_BAUD);
    Serial.println("Startup");

    while (!RTC.begin()) {
        Serial.println(F("RTC not ready"));
        delay(1000);
    }
    RTC.activeAlarm(false);     		// before alarm disable
    RTC.activeTimer(false);

    event.setTimeout(set_2000_leap, 0);
}

void loop (void) {
    event.yield();

    bcddatetime_t bcddatetime = RTC.now();
    Serial.print(bcddatetime.date, HEX);
    Serial.write(' ');
    Serial.print(bcdTimeToTimeString(bcddatetime.time));
    Serial.println();
    delay(1000);
}

void set_2000_leap (void) {
    bcddatetime_t bcddatetime = { 0x20000228, 0x235955 };
    if (RTC.adjust(bcddatetime)) Serial.println(F("Adjust 2000/02/28 23:59:55"));
    event.setTimeout(set_2000_may, 10000);
}

void set_2000_may (void) {
    bcddatetime_t bcddatetime = { 0x20000229, 0x235955 };
    if (RTC.adjust(bcddatetime)) Serial.println(F("Adjust 2000/02/29 23:59:55"));
    event.setTimeout(set_2100_leap, 10000);
}

void set_2100_leap (void) {
    bcddatetime_t bcddatetime = { 0x21000228, 0x235955 };
    if (RTC.adjust(bcddatetime)) Serial.println(F("Adjust 2100/02/28 23:59:55"));
    event.setTimeout(set_halt, 10000);
}

void set_halt (void) {
    Serial.println(F("Stop"));
    while (true);
}

// end of code