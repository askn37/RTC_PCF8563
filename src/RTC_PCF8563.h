/***************
 *
 * RTC_PCF8563 - RTC PCF8563/8583 I2C Interface for Arduino
 *
 * call constructor:
 *
 *		RTC_PCF8563()
 *
 * target architectures: Atmel AVR (ATmega 328P, 1284P and other)
 *
 * release site: https://github.com/askn37/RTC_PCF8563
 * maintainer: askn https://twitter.com/askn37
 *
 */

#ifndef __RTC_PCF8563_H
#define __RTC_PCF8563_H

#include <Arduino.h>
#include <Wire.h>
#include <bcdtime.h>

// I2C values
#define PCF8563_ADDR        0x51    // R:A3 W:A2

#define I2C_SPEED_STANDARD  100000
#define I2C_SPEED_DOUBLE    200000

#define RTC_CTRL_STOP       0x20
#define RTC_CTRL_TITP       0x10
#define RTC_CTRL_AF         0x08
#define RTC_CTRL_TF         0x04
#define RTC_CTRL_AIE        0x02
#define RTC_CTRL_TIE        0x01

#define RTC_COT_DISABLE     0x00
#define RTC_COT_32KHZ       0x80
#define RTC_COT_1KHZ        0x81
#define RTC_COT_32HZ        0x82
#define RTC_COT_1HZ         0x83

#define RTC_TIMER_DISABLE   0x0000
#define RTC_TIMER_4KHZ      0x8000
#define RTC_TIMER_64HZ      0x8100
#define RTC_TIMER_1S        0x8200
#define RTC_TIMER_60S       0x8300

class RTC_PCF8563 {
private :
    long _speed;
    uint8_t _i2caddr = 0;
    uint8_t _cs1 = 0;
    uint8_t _cs2 = 0;
    uint8_t _cs3 = 0;
    uint8_t _wday = 0;
public :
    bool begin (long = I2C_SPEED_STANDARD, uint8_t = PCF8563_ADDR);
    void setClock (long speed) { _speed = speed; Wire.setClock(_speed); }
    bool reset (void);
    bool isRunning (void);
    bool isAlarm (void);
    bool isTimer (void);
    uint8_t getStatus (void);
    bcddatetime_t now (bool = true);
    time_t epoch (bool = true);
    bool adjust (bcddatetime_t);
    bool adjust (time_t);
    bool activeAlarm (bool);
    bool setAlarm (bcdtime_t);
    bcdtime_t getAlarm (void);
    bool activeTimer (bool);
    bool setTimer (uint16_t);
    uint16_t getTimer (void);
    bool setClockOut (uint8_t);
    uint8_t getClockOut (void);
};

#endif

//  end of header
