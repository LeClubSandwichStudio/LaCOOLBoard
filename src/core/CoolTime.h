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

#ifndef COOLTIME_H
#define COOLTIME_H

#include <Arduino.h>

#include <DS1337.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <coredecls.h>
#include <sys/time.h>
#include <time.h>

#define TIMEOUT 2000
#define SECONDS_IN_WEEK 604800
#define TZ 0 
#define DST_MN 60 
#define TZ_MN ((TZ)*60)
#define TZ_SEC ((TZ)*3600)
#define DST_SEC ((DST_MN)*60)

class CoolTime {

public:
  void begin();
  int8_t timeServerIdx = -1;
  bool update();
  bool config(bool overwrite = false);
  void printConf();
  void setDateTime(int year, int month, int day, int hour, int minutes,
                   int seconds);
  String getESDate();
  unsigned long getLastSyncTime();
  bool isTimeSync(unsigned long seconds = SECONDS_IN_WEEK);
  int Year, Month, Day, Hour, Minute, Second;

private:
  const char *timeServer = "pool.ntp.org";
  DS1337 rtc;
};

#endif
