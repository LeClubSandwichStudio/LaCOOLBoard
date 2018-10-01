/**
 *  Copyright (c) 2018 La Cool Co SAS
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 *
 */

#include <FS.h>

#include <ArduinoJson.h>

#include "Concurrent.h"
#include "CoolLog.h"
#include "CoolTime.h"

bool CoolTime::ntpSync = false;

CoolTime &CoolTime::getInstance() {
  static CoolTime instance;
  return instance;
}

void timeSet(void) {
  CoolTime::ntpSync = true;
  DEBUG_LOG("Received response from NTP server");
}

void CoolTime::printStatus() {
  INFO_VAR("RTC ISO8601 timestamp:", this->getIso8601DateTime());
  DEBUG_VAR("NTP UNIX timestamp:", time(nullptr));
  DEBUG_VAR("RTC UNIX timestamp:", this->rtc.getTimestamp());
}

void CoolTime::begin() {
  DEBUG_LOG("CoolTime::begin() i2c LOCK");
  lockI2c();
  this->printStatus();
  if (this->rtc.hasStopped()) {
    WARN_LOG("RTC has stopped, need to resync");
  }
#ifdef ESP8266
  settimeofday_cb(timeSet);
#endif
  configTime(0, 0, "0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org");
  unlockI2c();
  DEBUG_LOG("CoolTime::begin() i2c UN-LOCK");
}

bool CoolTime::sync() {
  uint32_t waitUntil = millis() + NTP_TIMEOUT_MS;

  INFO_LOG("Waiting for NTP...");
  while (!CoolTime::ntpSync && millis() < waitUntil) {
#ifdef ESP32
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      timeSet();
    }
#endif
    delay(100);
  }
  DEBUG_LOG("CoolTime::sync() i2c LOCK");
  lockI2c();
  if (CoolTime::ntpSync) {
    this->rtc.setDateTime(time(nullptr));
    this->rtc.clearOSF();
    INFO_LOG("RTC was synchronized with NTP");
    this->printStatus();
    unlockI2c();
    DEBUG_LOG("CoolTime::sync() i2c UN-LOCK");
    return true;
  } else {
    if (rtc.hasStopped()) {
      unlockI2c();
      DEBUG_LOG("CoolTime::sync() i2c UN-LOCK");
      return false;
    } else {
      WARN_LOG("NTP failed, falling back to RTC");
      unlockI2c();
      DEBUG_LOG("CoolTime::sync() i2c UN-LOCK");
      return true;
    }
  }
}

String CoolTime::getIso8601DateTime() {
  char iso8601Date[] = "YYYY-MM-DDTHH:MM:SSZ";
  time_t t;
  time(&t);
  strftime(iso8601Date, sizeof iso8601Date, "%FT%TZ", gmtime(&t));
  return String(iso8601Date);
}