/***************
 *
 * FlashPluses - interval timer sample
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

// [O] RTC ~INT pin  -- K LED A -- CRD/R -- VCC

RTC_PCF8563 RTC;

void setup (void) {
    Serial.begin(CONSOLE_BAUD);
    Serial.println("Startup");

    while (!RTC.begin()) {
        Serial.println(F("RTC not ready"));
        delay(1000);
    }
    RTC.activeAlarm(false);                 // before alarm disable

    RTC.activeTimer(false);                 // current timer
    RTC.setTimer(RTC_TIMER_64HZ + 8);       // 8Hz (125msec) 
    // RTC.setTimer(RTC_TIMER_4KHZ + 1);       // 4096Hz (244usec) 

    RTC.activeTimer(true);                  // Active Timer
    RTC.activePulse(true);                 // Active Plulses
}

void loop (void) {}

// end of code