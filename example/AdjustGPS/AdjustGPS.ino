/***************
 *
 * AdjustGPS - adjust from GPS sample
 *
 * target architectures: Atmel AVR (ATmega 328P, 1284P and other)
 *
 * release site: https://github.com/askn37/GPS_MTK333X
 * maintainer: askn https://twitter.com/askn37
 *
 */

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <GPS_MTK333X_SoftwareSerial.h>
#include <IntervalEvent.h>
#include "RTC_PCF8563.h"

#define CONSOLE_BAUD	9600
#define GPS_BAUD		9600

#define GPS_TX		    6				// [O] D6 pin --- [I] GPS RX pin
#define GPS_RX		    5				// [I] D5 pin --- [O] GPS TX pin

#define TZ_SECONDS		(+9 * 3600)		// JST : +9:00

RTC_PCF8563 RTC;
GPS_MTK333X_SoftwareSerial GPS(GPS_RX, GPS_TX);
IntervalEvent event;

void setup (void) {
    Serial.begin(CONSOLE_BAUD);
    Serial.println(F("Startup"));
    Serial.print(F("TZ Offset: "));
    Serial.println(TZ_SECONDS / 3600.0);

    while (!GPS.begin(GPS_BAUD)) {
        Serial.println(F("GPS notready"));
        delay(1000);
    }
    GPS.sendMTKcommand(314, F(",0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"));	// Enable G*RTC

    while (!RTC.begin()) {
        Serial.println(F("RTC not ready"));
        delay(1000);
    }
    RTC.activeAlarm(false);     		// before alarm disable
    RTC.activeTimer(false);

    event.setInterval(viewRtc, 1000);
}

void loop (void) {
    static uint8_t count = 250;
    event.yield();
    if (GPS.check() && GPS.isTimeUpdate()) {
        time_t epoch = GPS.epoch() + TZ_SECONDS;
        if (++count == 0) {
            RTC.adjust(epoch);
            Serial.println(F("RTC Adjust"));
        }
        Serial.print(F("GPS: "));
        Serial.print(epoch);
        Serial.write(' ');
        Serial.print(epochToBcdDate(epoch), HEX);
        Serial.write(' ');
        Serial.print(epochToTimeString(epoch));
        Serial.println();
        GPS.statusReset();
    }
}

void viewRtc (void) {
    bcddatetime_t bcddatetime = RTC.now();
    Serial.print(F("RTC: "));
    Serial.print(bcdDateTimeToEpoch(bcddatetime));
    Serial.write(' ');
    Serial.print(bcddatetime.date, HEX);
    Serial.write(' ');
    Serial.print(bcdTimeToTimeString(bcddatetime.time));
    Serial.println();
}

// end of code