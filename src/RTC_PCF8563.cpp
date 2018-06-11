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

#include <Arduino.h>
#include "RTC_PCF8563.h"

bool RTC_PCF8563::begin (long speed, uint8_t i2caddr) {
    if (!_speed) Wire.begin();
    _i2caddr = i2caddr;
    _speed = speed;
    return isRunning();
}

bool RTC_PCF8563::reset (void) {
    Wire.setClock(_speed);
    Wire.beginTransmission(_i2caddr);
    Wire.write(0U);                     // Start address
    Wire.write(0U);                     // CS1
    Wire.write(0U);                     // CS2
    return !Wire.endTransmission();
}

bool RTC_PCF8563::isRunning (void) {
    getStatus();
    return !(_cs1 & RTC_CTRL_STOP);     // STOP bit is nagative
}

bool RTC_PCF8563::isAlarm (void) {
    return (getStatus() & RTC_CTRL_AF);
}

bool RTC_PCF8563::isTimer (void) {
    return (getStatus() & RTC_CTRL_TF);
}

uint8_t RTC_PCF8563::getStatus (void) {
    Wire.setClock(_speed);
    Wire.beginTransmission(_i2caddr);
    Wire.write(0U);
    Wire.endTransmission(false);
    Wire.requestFrom(_i2caddr, 2U);
    _cs1 = Wire.read();                 // CS1
    _cs2 = Wire.read();                 // CS2
    return _cs2;
}

bcddatetime_t RTC_PCF8563::now (bool twice) {
    bcdtime_t t_old = 0;
    bcddatetime_t t_bcd;
    uint16_t t_year;
    uint8_t t_sec, t_min, t_hour, t_mday, t_mon;
    // Twice read RTC
    do {
        t_old = t_bcd.time;
        Wire.setClock(_speed);
        Wire.beginTransmission(_i2caddr);
        Wire.write(0U);                     // Start address
        Wire.endTransmission(false);
        Wire.requestFrom(_i2caddr, 9U);     // read 9 byte
        _cs1   = Wire.read();               // CS1
        _cs2   = Wire.read();               // CS2
        _cs3   = Wire.read();
        t_min  = Wire.read();               // minute
        t_hour = Wire.read();               // hour
        t_mday = Wire.read();               // day
        _wday  = Wire.read();               // weekday
        t_mon  = Wire.read();               // mon
        t_year = btod(Wire.read()) + 2000;  // year
        t_sec  = _cs3 & 0x7F;               // second clear VL_status
        if (t_mon >> 7) t_year += 100;      // century_bit 0:2000 1:1900/2100
        t_sec &= 0x7F;
        t_bcd = {
            ((uint32_t)wdtob(t_year) << 16) + ((uint16_t)(t_mon & 0x7F) << 8) + t_mday,
            ((uint32_t)t_hour << 16) + ((uint16_t)t_min << 8) + t_sec,
        };
        if (t_old == t_bcd.time) return t_bcd;
    } while (twice);
    return t_bcd;
}

time_t RTC_PCF8563::epoch (bool twice) {
    return bcdDateTimeToEpoch(now(twice));
}

bool RTC_PCF8563::adjust (bcddatetime_t t_bcd) {
    uint16_t t_year = wbtod(t_bcd.date >> 16);
    uint8_t century_bit = ((t_year / 100) % 4 == 0) ? 0 : 0x80;
    Wire.setClock(_speed);
    Wire.beginTransmission(_i2caddr);
    Wire.write(2U);                             // start address
    Wire.write(t_bcd.time & 0x7F);              // second
    Wire.write((t_bcd.time >> 8) & 0x7F);       // minute
    Wire.write((t_bcd.time >> 16) & 0x3F);      // hour
    Wire.write(t_bcd.date & 0x3F);              // day
    Wire.write(bcdDateToWeekday(t_bcd.date));   // weekday
    Wire.write(((t_bcd.date >> 8) & 0x1F) | century_bit);   // mon and century_bit
    Wire.write(dtob(t_year % 100));             // year
    return !Wire.endTransmission();
}

bool RTC_PCF8563::adjust (time_t t_time) {
    return adjust(epochToBcd(t_time));
}

bool RTC_PCF8563::activeAlarm (bool t_enable) {
    if (!isRunning()) return false;
    _cs2 &= ~RTC_CTRL_AF;
    if (t_enable) _cs2 |=  RTC_CTRL_AIE;
    else          _cs2 &= ~RTC_CTRL_AIE;
    Wire.setClock(_speed);
    Wire.beginTransmission(_i2caddr);
    Wire.write(1U);
    Wire.write(_cs2);
    return !Wire.endTransmission();
}

bool RTC_PCF8563::activeTimer (bool t_enable) {
    if (!isRunning()) return false;
    _cs2 &= ~RTC_CTRL_TF;
    if (t_enable) _cs2 |=  RTC_CTRL_TIE;
    else          _cs2 &= ~RTC_CTRL_TIE;
    Wire.setClock(_speed);
    Wire.beginTransmission(_i2caddr);
    Wire.write(1U);
    Wire.write(_cs2);
    return !Wire.endTransmission();
}

//
// alarmTime = 0xDDHHmmWW
// 0x80 bit is disable
//
bool RTC_PCF8563::setAlarm (const bcdtime_t alarmTime) {
    Wire.setClock(_speed);
    Wire.beginTransmission(_i2caddr);
    Wire.write(9U);                     // start address
    Wire.write((alarmTime >> 8) & 0xFF);
    Wire.write((alarmTime >> 24) & 0xFF);
    Wire.write((alarmTime >> 16) & 0xFF);
    Wire.write(alarmTime & 0xF7);
    return !Wire.endTransmission();
}

bcdtime_t RTC_PCF8563::getAlarm (void) {
    uint8_t t_min, t_hour, t_mday, t_week;
    Wire.setClock(_speed);
    Wire.beginTransmission(_i2caddr);
    Wire.write(9U);                     // Start address
    Wire.endTransmission(false);
    Wire.requestFrom(_i2caddr, 4U);     // read 9 byte
    t_min  = Wire.read();               // minute
    t_hour = Wire.read();               // hour
    t_mday = Wire.read();               // day
    t_week = Wire.read();               // weekday
    return (((uint32_t)t_mday << 24)
          + ((uint32_t)t_hour << 16)
          + ((uint16_t)t_min << 8) + t_week);
}

bool RTC_PCF8563::setTimer (const uint16_t timer) {
    Wire.setClock(_speed);
    Wire.beginTransmission(_i2caddr);
    Wire.write(0xEU);                   // start address
    Wire.write((timer >> 8) & 0xFF);
    Wire.write(timer & 0xFF);
    return !Wire.endTransmission();
}

uint16_t RTC_PCF8563::getTimer (void) {
    uint8_t t_ctrl, t_count;
    Wire.setClock(_speed);
    Wire.beginTransmission(_i2caddr);
    Wire.write(0xEU);                   // Start address
    Wire.endTransmission(false);
    Wire.requestFrom(_i2caddr, 2U);     // read 2 byte
    t_ctrl  = Wire.read();              // minute
    t_count = Wire.read();              // hour
    return (((uint16_t)t_ctrl << 8) + t_count);
}

bool RTC_PCF8563::setClockOut (uint8_t clock) {
    Wire.setClock(_speed);
    Wire.beginTransmission(_i2caddr);
    Wire.write(0xDU);                   // start address
    Wire.write(clock);
    return !Wire.endTransmission();
}

uint8_t RTC_PCF8563::getClockOut (void) {
    Wire.setClock(_speed);
    Wire.beginTransmission(_i2caddr);
    Wire.write(0xDU);                   // Start address
    Wire.endTransmission(false);
    Wire.requestFrom(_i2caddr, 1U);     // read 1 byte
    uint8_t t_clock  = Wire.read();
    return (t_clock);
}

// end of code