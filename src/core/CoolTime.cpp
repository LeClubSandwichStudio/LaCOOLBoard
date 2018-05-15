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

void CoolTime::begin() {
  Udp.begin(localPort);
  this->update();
}

void CoolTime::offGrid() {
  if (compileTime == 1 && NTP == 0) {
    char posMarker = 0;
    for (int i = 0; i <= sizeof(__TIMESTAMP__); i++) {
      if (__TIMESTAMP__[i] == ':') {
        posMarker = i;
        break;
      }
    }
    char monthAbbr[4] = {__TIMESTAMP__[4], __TIMESTAMP__[5], __TIMESTAMP__[6],
                         0};
    char tempDay[3] = {__TIMESTAMP__[8], __TIMESTAMP__[9], 0};
    int Day = atoi(&tempDay[0]);
    char tempHour[3] = {__TIMESTAMP__[posMarker - 2],
                        __TIMESTAMP__[posMarker - 1], 0};
    int Hour = atoi(&tempHour[0]);
    char tempMinute[3] = {__TIMESTAMP__[posMarker + 1],
                          __TIMESTAMP__[posMarker + 2], 0};
    int Minute = atoi(&tempMinute[0]);
    char tempSecond[3] = {__TIMESTAMP__[posMarker + 4],
                          __TIMESTAMP__[posMarker + 5], 0};
    int Second = atoi(&tempSecond[0]);
    char tempYear[3] = {__TIMESTAMP__[posMarker + 9],
                        __TIMESTAMP__[posMarker + 10], 0};
    int Year = atoi(&tempYear[0]);
    int Month;

    if (strstr(monthAbbr, "Jan")) {
      Month = 1;
    }
    if (strstr(monthAbbr, "Feb")) {
      Month = 2;
    }
    if (strstr(monthAbbr, "Mar")) {
      Month = 3;
    }
    if (strstr(monthAbbr, "Apr")) {
      Month = 4;
    }
    if (strstr(monthAbbr, "May")) {
      Month = 5;
    }
    if (strstr(monthAbbr, "Jun")) {
      Month = 6;
    }
    if (strstr(monthAbbr, "Jul")) {
      Month = 7;
    }
    if (strstr(monthAbbr, "Aug")) {
      Month = 8;
    }
    if (strstr(monthAbbr, "Sep")) {
      Month = 9;
    }
    if (strstr(monthAbbr, "Oct")) {
      Month = 10;
    }
    if (strstr(monthAbbr, "Nov")) {
      Month = 11;
    }
    if (strstr(monthAbbr, "Dec")) {
      Month = 12;
    }
    setDateTime(y2kYearToTm(Year), Month, Day, Hour, Minute, Second);
    unsigned long instantTime = RTC.get(CLOCK_ADDRESS);
    this->timeSync = instantTime;
    this->compileTime = 0;
    saveTimeSync();
    DEBUG_VAR("RTC set from:", __TIMESTAMP__);
    DEBUG_VAR("Seconds since UNIX Epoch:", instantTime);
  }
}

void CoolTime::update() {
  if (this->NTP == 1 && WiFi.status() == WL_CONNECTED) {
    if (this->timePool == -1) {
      this->timePool = timePoolConfig();
    }
    if (!(this->isTimeSync())) {
      int repeats = 0;

      DEBUG_LOG("Waiting for sync");
      this->timeSync = this->getNtpTime();
      while (this->timeSync == 0) {
        delay(1000);
        this->timeSync = this->getNtpTime();

        if (repeats >= 4) {
          timePoolConfig();
          delay(500);
          this->timeSync = this->getNtpTime();
          break;
        }
        repeats++;
      }
      breakTime(this->timeSync, this->tmSet);
      this->rtc.set(makeTime(this->tmSet), CLOCK_ADDRESS); // set the clock
      this->saveTimeSync();
    }
  }
}

void CoolTime::setDateTime(int year, int month, int day, int hour, int minutes,
                           int seconds) {
  tmElements_t tm;

  tm.Second = seconds;
  tm.Minute = minutes;
  tm.Hour = hour;
  tm.Day = day;
  tm.Month = month;
  tm.Year = year;
  this->rtc.set(makeTime(tm), CLOCK_ADDRESS);
  DEBUG_VAR("Time set to:", this->getESDate());
}

tmElements_t CoolTime::getTimeDate() {
  tmElements_t tm;

  // FIXME: experimental: dummy call to prevent slow RTC
  rtc.get(CLOCK_ADDRESS);
  delay(50);
  time_t timeDate = this->rtc.get(CLOCK_ADDRESS);
  breakTime(timeDate, tm);
  return (tm);
}

String CoolTime::getESDate() {
  tmElements_t tm = this->getTimeDate();

  // output format: "yyyy-mm-ddTHH:MM:ssZ"
  String elasticSearchString =
      String(tm.Year + 1970) + "-" + this->formatDigits(tm.Month) + "-";
  elasticSearchString +=
      this->formatDigits(tm.Day) + "T" + this->formatDigits(tm.Hour) + ":";
  elasticSearchString +=
      this->formatDigits(tm.Minute) + ":" + this->formatDigits(tm.Second) + "Z";
  return (elasticSearchString);
}

unsigned long CoolTime::getLastSyncTime() {
  DEBUG_VAR("Last RTC sync time:", this->timeSync);
  return (this->timeSync);
}

bool CoolTime::isTimeSync(unsigned long seconds) {
  // FIXME: experimental: dummy call to prevent slow RTC
  RTC.get(CLOCK_ADDRESS);

  unsigned long instantTime = RTC.get(CLOCK_ADDRESS);

  DEBUG_VAR("Current RTC time:", instantTime);
  DEBUG_VAR("Time since last sync:", instantTime - this->timeSync);
  if ((instantTime - this->timeSync) > (seconds)) {
    WARN_LOG("RTC is not synchronised");
    return (false);
  }
  DEBUG_LOG("RTC is synchronised");
  return (true);
}

time_t CoolTime::getNtpTime() {
  WiFi.hostByName(timeServer[timePool], timeServerIP);
  if (timeServerIP[0] == 0 && timeServerIP[1] == 0 && timeServerIP[2] == 0 &&
      timeServerIP[3] == 0) {
    WARN_VAR("No IP address for timeserver", this->timeServer[this->timePool]);
    WARN_LOG("Will run NTP benchmark later on");
    return 0;
  } else {
    DEBUG_VAR("Sending NTP request to:", this->timeServerIP);

    while (Udp.parsePacket() > 0)
      ; // discard any previously received packets
    sendNTPpacket(timeServerIP);
    uint32_t beginWait = millis();

    while (millis() - beginWait < TIMEOUT) {
      int size = Udp.parsePacket();
      if (size >= NTP_PACKET_SIZE) {
        unsigned long secsSince1900;
        unsigned long unixTime;
        DEBUG_LOG("Received response from NTP server");
        Udp.read(packetBuffer, NTP_PACKET_SIZE);
        // convert four bytes starting at location 40 to a long integer
        secsSince1900 = (unsigned long)packetBuffer[40] << 24;
        secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
        secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
        secsSince1900 |= (unsigned long)packetBuffer[43];
        unixTime = secsSince1900 - 2208988800UL;
        DEBUG_VAR("Received UNIX time:", unixTime);
        return unixTime;
      }
    }
  }
  ERROR_LOG("No response form NTP server");
  return 0;
}

void CoolTime::sendNTPpacket(IPAddress &address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

bool CoolTime::config() {
  CoolConfig config("/rtcConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to parse RTC configuration");
    return (false);
  }
  JsonObject &json = config.get();
  DEBUG_JSON("RTC configuration JSON:", json);
  if (json["timePool"].success()) {
    this->timePool = json["timePool"];
  }
  json["timePool"] = this->timePool;
  if (json["timeSync"].success()) {
    this->timeSync = json["timeSync"];
  }
  json["timeSync"] = this->timeSync;
  if (json["NTP"].success()) {
    this->NTP = json["NTP"].as<bool>();
  }
  json["NTP"] = this->NTP;
  if (json["compileTime"].success()) {
    this->compileTime = json["compileTime"].as<bool>();
  }
  json["compileTime"] = this->compileTime;
  if (!config.writeJsonToFile()) {
    ERROR_LOG("Failed to save RTC configuration");
    return (false);
  }
  INFO_LOG("RTC configuration loaded");
  return (true);
}

bool CoolTime::saveTimeSync() {
  CoolConfig config("/rtcConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to parse RTC configuration");
    return (false);
  }
  JsonObject &json = config.get();
  DEBUG_JSON("RTC configuration JSON:", json);
  json["timePool"] = this->timePool;
  json["timeSync"] = this->timeSync;
  json["NTP"] = this->NTP;
  json["compileTime"] = this->compileTime;
  if (!config.writeJsonToFile()) {
    ERROR_LOG("Failed to save RTC configuration");
    return (false);
  }
  INFO_LOG("RTC configuration updated");
  return (true);
}

void CoolTime::printConf() {
  INFO_LOG("RTC configuration");
  INFO_LOG("  NTP servers         :");
  for (int i = 0; i < SERVERCOUNT; i++) {
    INFO_VAR("    ", timeServer[i]);
  }
  INFO_VAR("  Local port          :", localPort);
  INFO_VAR("  NTP enabled         :", NTP);
  INFO_VAR("  Use compilation date:", compileTime);
}

String CoolTime::formatDigits(int digits) {
  if (digits < 10) {
    return (String("0") + String(digits));
  }
  return (String(digits));
}

int CoolTime::timePoolConfig() {
  INFO_LOG("Performing NTP server benchmark...");

  unsigned long latency[SERVERCOUNT];
  bool timeout[SERVERCOUNT];

  for (int i = 0; i < SERVERCOUNT; i++) {
    latency[i] = 0;
    timeout[i] = false;
  }

  for (int j = 1; j <= NTP_OVERSAMPLE; j++) {
    for (int i = 0; i < SERVERCOUNT; i++) {
      while (Udp.parsePacket() > 0) {
        ; // discard any previously received packets
      }
      WiFi.hostByName(timeServer[i], timeServerIP);
      if (timeServerIP[0] == 0 && timeServerIP[1] == 0 &&
          timeServerIP[2] == 0 && timeServerIP[3] == 0) {
        WARN_VAR("Could not get IP of NTP server:", timeServer[i]);
        timeout[i] = true;
      } else {
        DEBUG_VAR("Sending NTP request to:", timeServer[i]);
        sendNTPpacket(timeServerIP);

        uint32_t beginWait = millis();

        while ((millis() - beginWait) < (TIMEOUT + 200)) {
          int size = Udp.parsePacket();
          if (size >= NTP_PACKET_SIZE) {
            latency[i] += (millis() - beginWait);
            DEBUG_VAR("Received response from NTP server:", timeServer[i]);
            break;
          }
          if ((millis() - beginWait) >= TIMEOUT) {
            timeout[i] = true;
            WARN_VAR("Hit timeout for NTP server:", timeServer[i]);
            break;
          }
        }
      }
    }
  }

  unsigned long temp = 0;
  int result = -1;

  if (latency[0] != 0 && !timeout[0]) {
    temp = latency[0];
    result = 0;
  }

  for (int i = 0; i < SERVERCOUNT; i++) {
    if ((latency[i] != 0) && !timeout[i] && (latency[i] < temp)) {
      temp = latency[i];
      result = i;
    }
  }

  INFO_VAR("NTP latency test finished, fastest is:", timeServer[result]);
  INFO_VAR("NTP minimum latency:", latency[result] / NTP_OVERSAMPLE);
  return result;
}