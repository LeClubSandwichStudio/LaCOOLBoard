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

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <time.h>

#include <DS1337RTC.h>
#include <TimeLib.h>

#define NTP_PACKET_SIZE 48
#define SERVERCOUNT 6
#define NTP_OVERSAMPLE 3
#define TIMEOUT 2000
#define SECONDS_IN_WEEK 604800

class CoolTime {

public:
  void begin();
  void offGrid();
  void update();
  bool config(bool overwrite = false);
  void printConf();
  void setDateTime(int year, int month, int day, int hour, int minutes,
                   int seconds);
  tmElements_t getTimeDate();
  String getESDate();
  unsigned long getLastSyncTime();
  bool isTimeSync(unsigned long seconds = SECONDS_IN_WEEK);
  time_t getNtpTime();
  void sendNTPpacket(IPAddress &address);
  String formatDigits(int digits);
  bool selectTimeServer();
  bool isServerSelected() const;

private:
  unsigned long timeSync = 0;
  int8_t timeServerIdx = -1;
  const char *TIME_SERVER_LIST[SERVERCOUNT] = {
      "africa.pool.ntp.org",  "asia.pool.ntp.org",
      "europe.pool.ntp.org",  "north-america.pool.ntp.org",
      "oceania.pool.ntp.org", "south-america.pool.ntp.org"};
  bool NTP = true;
  bool compileTime = false;
  WiFiUDP Udp;
  unsigned int localPort = 0;
  byte packetBuffer[NTP_PACKET_SIZE];
  tmElements_t tmSet;
  DS1337RTC rtc;
};

#endif
