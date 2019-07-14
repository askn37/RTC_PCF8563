/***************
 *
 * IntervalTimer - interval timer sample
 *
 * target architectures: Atmel AVR (ATmega 328P, 1284P and other)
 *
 * release site: https://github.com/askn37/GPS_MTK333X
 * maintainer: askn https://twitter.com/askn37
 *
 */

#include <Arduino.h>
#include "RTC_PCF8563.h"

#define CONSOLE_BAUD    9600

#define RTC_ARLM        9		// [I] D9 pin --- [O] RTC ~INT pin

RTC_PCF8563 RTC;

void setup (void) {
    pinMode(RTC_ARLM, INPUT_PULLUP);

    Serial.begin(CONSOLE_BAUD);
    Serial.println("Startup");

    while (!RTC.begin()) {
        Serial.println(F("RTC not ready"));
        delay(1000);
    }
    RTC.activeAlarm(false);     // before alarm disable
    RTC.activeTimer(false);

    RTC.setTimer(RTC_TIMER_1S + 5);
    RTC.activeTimer(true);
}

void loop (void) {
    if (!digitalRead(RTC_ARLM)) {
        // Serial.println(F("[RingRingRingRing]"));
        RTC.activeTimer(false);
        viewRtc();
        RTC.activeTimer(true);
    }
}

void viewRtc (void) {
    bcddatetime_t bcddatetime = RTC.now(false);
    Serial.print(bcddatetime.date, HEX);
    Serial.write(' ');
    Serial.print(bcdTimeToTimeString(bcddatetime.time));
    Serial.println();
}

// end of code