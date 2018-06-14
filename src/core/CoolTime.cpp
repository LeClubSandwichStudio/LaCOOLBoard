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

#include "CoolConfig.h"
#include "CoolLog.h"
#include "CoolTime.h"

extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);
timeval cbtime;
timeval tv;
timespec tp;
time_t now;

void CoolTime::begin() { configTime(0, 0, "pool.ntp.org"); }

bool CoolTime::update() {
  if (WiFi.status() == WL_CONNECTED) {
    INFO_LOG("Researching NTP time...");
    configTime(0, 0, timeServer);
    gettimeofday(&tv, nullptr);
    clock_gettime(0, &tp);
    now = time(nullptr);
    DEBUG_VAR("Received UNIX time: ", (uint32_t)now);
    this->rtc.getTime((uint32_t)now, Year, Month, Day, Hour, Minute, Second);
    this->setDateTime(Year, Month, Day, Hour, Minute, Second);
    return (true);
  }
  return (false);
}

void CoolTime::setDateTime(int year, int month, int day, int hour, int minutes,
                           int seconds) {
  this->rtc.setTime(hour, minutes, seconds);
  this->rtc.setDate(year, month, day);
  this->rtc.clearOSF();
  DEBUG_VAR("Time set to: ", this->getESDate());
}

String CoolTime::getESDate() {
  // // output format: "yyyy-mm-ddTHH:MM:ssZ"
  String elasticSearchString = "20" + this->rtc.getDate().getDateString() +
                               "T" + this->rtc.getDate().getTimeString() + "Z";
  return (elasticSearchString);
}

unsigned long CoolTime::getLastSyncTime() {
  DEBUG_VAR("Last RTC sync time:", (uint32_t)now);
  return ((uint32_t)now);
}

bool CoolTime::isTimeSync(unsigned long seconds) {
  unsigned long instantTime = this->rtc.getTimestamp();
  DEBUG_VAR("Current RTC time:", instantTime);
  DEBUG_VAR("Time since last sync:", instantTime - (uint32_t)now);
  if ((instantTime - (uint32_t)now) > (seconds)) {
    WARN_LOG("RTC is not synchronised");
    return (false);
  }
  DEBUG_LOG("RTC is synchronised");
  this->rtc.clearOSF();
  return (true);
}

void CoolTime::printConf() {
  INFO_LOG("RTC configuration");
  INFO_VAR("  Selected time server  =", timeServer);
  INFO_VAR("  RTC timestamp         =", this->rtc.getTimestamp());
  if (this->rtc.hasStopped()) {
    WARN_LOG("  RTC clock Was Stopped, need to resync");
  } else {
    INFO_LOG("  RTC battery clock: OK");
  }
}