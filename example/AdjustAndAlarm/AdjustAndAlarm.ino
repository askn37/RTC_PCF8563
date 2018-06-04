/***************
 *
 * AdjustAndAlarm - adjust and alarm sample
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

    Serial.print(__DATE__);
    Serial.write(' ');
    Serial.print(__TIME__);
    Serial.println();

    bcddatetime_t bcddatetime = buildtime(__DATE__, __TIME__);
    while (true) {
        if (RTC.adjust(bcddatetime)) {
            Serial.print(F("Adjust buildtime "));
            Serial.print(bcddatetime.date, HEX);
            Serial.write(' ');
            Serial.print(bcdTimeToTimeString(bcddatetime.time));
            Serial.println();
            break;
        }
    }
    initAlarm();
}

void loop (void) {
    bcddatetime_t bcddatetime = RTC.now();
    Serial.print(bcddatetime.date, HEX);
    Serial.write(' ');
    Serial.print(bcdTimeToTimeString(bcddatetime.time));
    Serial.println();
    delay(1000);

    if (!digitalRead(RTC_ARLM)) {
        Serial.println(F("[RingRingRingRing]"));
        RTC.activeAlarm(false);
        initAlarm();
    }
}

void initAlarm (void) {
    bcddatetime_t bcddatetime = RTC.now();
    time_t mm = bcdTimeToSeconds(bcddatetime.time) + 60;
    date_t alarm = 0xFFFF00FF | epochToBcdTime(mm);
    if (RTC.setAlarm(alarm) && RTC.activeAlarm(true)) {
        Serial.print(F("[setting alarm next: "));
        date_t alarm = RTC.getAlarm();
        Serial.print(alarm, HEX);
        Serial.println(']');
    }
}

//
// datetime
//
uint8_t conv2d (const char* p) {
	uint8_t v = 0;
	if ('0' <= *p && *p <= '9') v = *p - '0';
	return 10 * v + *++p - '0';
}

bcddatetime_t buildtime (const char* date, const char* time) {
	// "May 31 2018", time = "12:34:56"
	uint8_t x, y, m, d, hh, mm, ss;
	x = conv2d(date + 7);
	y = conv2d(date + 9);
	d = conv2d(date + 4);
	switch (date[0]) {
		case 'J': m = (date[1] == 'a') ? 1 : (m = date[2] == 'n') ? 6 : 7; break;
		case 'F': m = 2; break;
		case 'A': m = date[2] == 'r' ? 4 : 8; break;
		case 'M': m = date[2] == 'r' ? 3 : 5; break;
		case 'S': m = 9; break;
		case 'O': m = 10; break;
		case 'N': m = 11; break;
		case 'D': m = 12; break;
		default:
            d = conv2d(date + 0);
            m = conv2d(date + 3);
            x = conv2d(date + 6);
            y = conv2d(date + 8);
	}
	hh = conv2d(time + 0);
	mm = conv2d(time + 3);
	ss = conv2d(time + 6);
    bcddatetime_t bcddatetime = {
        ((uint32_t)dtob(x) << 24) + ((uint32_t)dtob(y) << 16) + ((uint16_t)dtob(m) << 8) + dtob(d),
        ((uint32_t)dtob(hh) << 16) + ((uint16_t)dtob(mm) << 8) + dtob(ss)
    };
    return bcddatetime;
}

// end of code