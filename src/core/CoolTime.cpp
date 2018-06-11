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

static constexpr dateConfig _dateConfig[] = {
    {"Jan", 1}, {"Feb", 2}, {"Mar", 3}, {"Apr", 4},  {"May", 5},  {"Jun", 6},
    {"Jul", 7}, {"Aug", 8}, {"Sep", 9}, {"Oct", 10}, {"Nov", 11}, {"Dec", 12}};

void CoolTime::begin() { Udp.begin(localPort); }

void CoolTime::offGrid() {
  if (compileTime && !NTP) {
    char posMarker = 0;
    for (uint8_t i = 0; i <= sizeof(__TIMESTAMP__); i++) {
      if (__TIMESTAMP__[i] == ':') {
        posMarker = i;
        break;
      }
    }
    char monthAbbr[4] = {__TIMESTAMP__[4], __TIMESTAMP__[5], __TIMESTAMP__[6],
                         0};
    char tempDay[3] = {__TIMESTAMP__[8], __TIMESTAMP__[9], 0};
    uint8_t Day = atoi(&tempDay[0]);
    char tempHour[3] = {__TIMESTAMP__[posMarker - 2],
                        __TIMESTAMP__[posMarker - 1], 0};
    uint8_t Hour = atoi(&tempHour[0]);
    char tempMinute[3] = {__TIMESTAMP__[posMarker + 1],
                          __TIMESTAMP__[posMarker + 2], 0};
    uint8_t Minute = atoi(&tempMinute[0]);
    char tempSecond[3] = {__TIMESTAMP__[posMarker + 4],
                          __TIMESTAMP__[posMarker + 5], 0};
    uint8_t Second = atoi(&tempSecond[0]);
    char tempYear[3] = {__TIMESTAMP__[posMarker + 9],
                        __TIMESTAMP__[posMarker + 10], 0};
    int Year = atoi(&tempYear[0]);
    uint8_t Month;

    for (uint8_t i = 0; i <= 12; i++) {
      if (strstr(monthAbbr, _dateConfig[i].month)) {
        Month = _dateConfig[i].dec;
      }
    }

    this->timeSync = this->rtc.getTimestamp();
    this->compileTime = false;
    this->config(false);
    DEBUG_VAR("RTC set from:", __TIMESTAMP__);
    DEBUG_VAR("Seconds since UNIX Epoch:", timeSync);
  }
}

bool CoolTime::update() {
  if (this->NTP && WiFi.status() == WL_CONNECTED) {
    this->selectTimeServer();
    this->printConf();
    if (!(this->isTimeSync())) {
      int repeats = 0;
      DEBUG_LOG("Waiting for sync");
      this->timeSync = this->getNtpTime();
      while (!this->timeSync) {
        delay(1000);
        this->timeSync = this->getNtpTime();

        if (repeats >= 4) {
          selectTimeServer();
          delay(500);
          this->timeSync = this->getNtpTime();
          break;
        }
        repeats++;
      }
      this->rtc.clearOSF();
      this->rtc.getTime(this->timeSync, Year, Month, Day, Hour, Minute, Second);
      this->rtc.setDate(Year, Month, Day);
      this->rtc.setTime(Hour, Minute, Second);
    }
    this->config(true);
    return (true);
  }
}

void CoolTime::setDateTime(int year, int month, int day, int hour, int minutes,
                           int seconds) {
  this->rtc.setTime(hour, minutes, seconds);
  this->rtc.setDate(year, month, day);
  DEBUG_VAR("Time set to: ", this->getESDate());
}

String CoolTime::getESDate() {
  // // output format: "yyyy-mm-ddTHH:MM:ssZ"
  String elasticSearchString ="20" + 
      this->rtc.getDate().getDateString() + "T" + this->rtc.getDate().getTimeString() + "Z";
  return (elasticSearchString);
}

unsigned long CoolTime::getLastSyncTime() {
  DEBUG_VAR("Last RTC sync time:", this->timeSync);
  return (this->timeSync);
}

bool CoolTime::isTimeSync(unsigned long seconds) {
  unsigned long instantTime = this->rtc.getTimestamp();
  DEBUG_VAR("Current RTC time:", instantTime);
  DEBUG_VAR("Time since last sync:", instantTime - this->timeSync);
  if ((instantTime - this->timeSync) > (seconds)) {
    WARN_LOG("RTC is not synchronised");
    return (false);
  }
  DEBUG_LOG("RTC is synchronised");
  this->rtc.clearOSF();
  return (true);
}

time_t CoolTime::getNtpTime() {
  IPAddress timeServerIp;

  WiFi.hostByName(this->TIME_SERVER_LIST[this->timeServerIdx], timeServerIp);
  if (timeServerIp[0] == 0 && timeServerIp[1] == 0 && timeServerIp[2] == 0 &&
      timeServerIp[3] == 0) {
    WARN_VAR("No IP address for timeserver",
             this->TIME_SERVER_LIST[this->timeServerIdx]);
    WARN_LOG("Will run NTP benchmark later on");
    return 0;
  } else {
    DEBUG_VAR("Sending NTP request to:", timeServerIp);

    while (Udp.parsePacket() > 0)
      ; // discard any previously received packets
    sendNTPpacket(timeServerIp);
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
  ERROR_LOG("No response from NTP server");
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

bool CoolTime::config(bool overwrite) {
  CoolConfig config("/rtcConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to parse RTC configuration");
    return (false);
  }
  JsonObject &json = config.get();
  DEBUG_JSON("RTC configuration JSON:", json);
  config.set<int8_t>(json, "timePool", this->timeServerIdx, overwrite);
  config.set<unsigned long>(json, "timeSync", this->timeSync, overwrite);
  config.set<bool>(json, "NTP", this->NTP, overwrite);
  config.set<bool>(json, "compileTime", this->compileTime, overwrite);
  if (!config.writeJsonToFile()) {
    ERROR_LOG("Failed to save RTC configuration");
    return (false);
  }
  INFO_LOG("RTC configuration loaded");

  return (true);
}

bool CoolTime::isServerSelected() const {
  return (this->timeServerIdx >= 0 && this->timeServerIdx < SERVERCOUNT);
}

void CoolTime::printConf() {
  INFO_LOG("RTC configuration");
  String timeServer;
  if (this->isServerSelected()) {
    timeServer = this->TIME_SERVER_LIST[this->timeServerIdx];
  } else {
    timeServer = "NONE";
  }
  INFO_VAR("  Local port            =", this->localPort);
  INFO_VAR("  NTP enabled           =", this->NTP);
  INFO_VAR("  Use compilation date  =", this->compileTime);
  INFO_VAR("  Selected time server  =", timeServer);
  INFO_VAR("  RTC timestamp         =", this->timeSync);
  if( this->rtc.hasStopped()){
    WARN_LOG(" RTC clock Was Stopped, need to resync");
  } else {
    INFO_LOG(" RTC clock: OK");
  }
}

String CoolTime::formatDigits(int digits) {
  if (digits < 10) {
    return (String("0") + String(digits));
  }
  return (String(digits));
}

bool CoolTime::selectTimeServer() {
  INFO_LOG("Performing NTP server benchmark...");
  uint32_t latency[SERVERCOUNT];
  bool timeout[SERVERCOUNT];

  for (int i = 0; i < SERVERCOUNT; i++) {
    latency[i] = UINT32_MAX;
    timeout[i] = false;
  }

  for (int j = 1; j <= NTP_OVERSAMPLE; j++) {
    for (int i = 0; i < SERVERCOUNT; i++) {
      while (Udp.parsePacket() > 0) {
        ; // discard any previously received packets
      }
      IPAddress timeServerIp;
      const char *timeServer = TIME_SERVER_LIST[i];
      WiFi.hostByName(timeServer, timeServerIp);
      if (timeServerIp[0] == 0 && timeServerIp[1] == 0 &&
          timeServerIp[2] == 0 && timeServerIp[3] == 0) {
        WARN_VAR("Could not get IP of NTP server:", timeServer);
        timeout[i] = true;
      } else {
        DEBUG_VAR("Sending NTP request to:", timeServer);
        sendNTPpacket(timeServerIp);
        uint32_t beginWait = millis();

        while ((millis() - beginWait) < (TIMEOUT + 200)) {
          int size = Udp.parsePacket();
          if (size >= NTP_PACKET_SIZE) {
            latency[i] += (millis() - beginWait);
            DEBUG_VAR("Received response from NTP server:", timeServer);
            break;
          }
          if ((millis() - beginWait) >= TIMEOUT) {
            timeout[i] = true;
            WARN_VAR("Hit timeout for NTP server:", timeServer);
            break;
          }
        }
      }
    }
  }

  unsigned long minLatency = UINT32_MAX;
  for (int i = 0; i < SERVERCOUNT; i++) {
    if (!timeout[i] && (latency[i] < minLatency)) {
      minLatency = latency[i];
      this->timeServerIdx = i;
    }
  }
  INFO_VAR("NTP minimum latency:", minLatency / NTP_OVERSAMPLE);
  if (this->isServerSelected()) {
    INFO_VAR("NTP latency test finished, fastest is:",
             TIME_SERVER_LIST[this->timeServerIdx]);
    return true;
  } else {
    ERROR_LOG("NTP latency test finished, no suitable server found!");
    return false;
  }
}