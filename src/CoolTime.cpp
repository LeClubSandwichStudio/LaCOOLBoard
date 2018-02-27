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

#include <Arduino.h>
#include <FS.h>

#include <ArduinoJson.h>
#include <TimeLib.h>

#include "CoolConfig.h"
#include "CoolLog.h"
#include "CoolTime.h"

/**
 *  CoolTime::begin():
 *  This method is provided to init
 *  the udp connection
 *
 */
void CoolTime::begin() {
  Udp.begin(localPort);
  this->update();
}

/**
 *  CoolTime::offGrid:
 *  This method is provided to init
 *  the udp connection
 *
 */
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
                        __TIMESTAMP__[posMarker - 1], '\0'};
    int Hour = atoi(&tempHour[0]);
    char tempMinute[3] = {__TIMESTAMP__[posMarker + 1],
                          __TIMESTAMP__[posMarker + 2], '\0'};
    int Minute = atoi(&tempMinute[0]);
    char tempSecond[3] = {__TIMESTAMP__[posMarker + 4],
                          __TIMESTAMP__[posMarker + 5], '\0'};
    int Second = atoi(&tempSecond[0]);
    char tempYear[3] = {__TIMESTAMP__[posMarker + 9],
                        __TIMESTAMP__[posMarker + 10], '\0'};
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

/**
 *  CoolTime::update():
 *  This method is provided to correct the
 *  rtc Time when it drifts,once every week.
 */
void CoolTime::update() {
  if (this->NTP == 1) {
    if (!(this->isTimeSync())) {
      DEBUG_LOG("Waiting for sync");
      this->timeSync = this->getNtpTime();
      breakTime(this->getNtpTime(), this->tmSet);
      this->rtc.set(makeTime(this->tmSet), CLOCK_ADDRESS);
      this->saveTimeSync();
    }
  }
}

/**
 *  CoolTime::setDateTime(year,month,dat,hour,minutes,seconds):
 *  This method is provided to manually set the RTc Time
 *
 */
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

/**
 *  CoolTime::getTimeDate():
 *  This method is provided to get the RTC Time
 *
 *  \returns a tmElements_t structre that has
 *  the time in it
 */
tmElements_t CoolTime::getTimeDate() {
  tmElements_t tm;
  // FIXME: experimental: dummy call to prevent slow RTC
  rtc.get(CLOCK_ADDRESS);
  delay(50);
  time_t timeDate = this->rtc.get(CLOCK_ADDRESS);
  breakTime(timeDate, tm);
  return (tm);
}

/**
 *  CoolTime::getESDate():
 *  This method is provided to return an
 *  Elastic Search compatible date Format
 *
 *  \return date String in Elastic Search
 *  format
 */
String CoolTime::getESDate() {
  tmElements_t tm = this->getTimeDate();

  // format: "yyyy-mm-ddTHH:MM:ssZ"
  String elasticSearchString =
      String(tm.Year + 1970) + "-" + this->formatDigits(tm.Month) + "-";
  elasticSearchString +=
      this->formatDigits(tm.Day) + "T" + this->formatDigits(tm.Hour) + ":";
  elasticSearchString +=
      this->formatDigits(tm.Minute) + ":" + this->formatDigits(tm.Second) + "Z";
  return (elasticSearchString);
}

/**
 *  CoolTime::getLastSyncTime():
 *  This method is provided to get the last time
 *  we syncronised the time
 *
 *  \return unsigned long representation of
 *  last syncronisation time in seconds
 */
unsigned long CoolTime::getLastSyncTime() {
  DEBUG_VAR("Last RTC sync time:", this->timeSync);
  return (this->timeSync);
}

/**
 *  CoolTime::isTimeSync( time in seconds):
 *  This method is provided to test if the
 *  time is syncronised or not.
 *  By default we test once per week.
 *
 *  \return true if time is syncronised,false
 *  otherwise
 */
bool CoolTime::isTimeSync(unsigned long seconds) {
  // FIXME: experimental: dummy call to prevent slow RTC
  RTC.get(CLOCK_ADDRESS);

  unsigned long instantTime = RTC.get(CLOCK_ADDRESS);
  unsigned long testSync = instantTime - this->timeSync;

  DEBUG_VAR("Current RTC time:", instantTime);
  this->getLastSyncTime();
  DEBUG_VAR("Time since last sync:", testSync);
  if ((instantTime - this->timeSync) > (seconds)) {
    WARN_LOG("RTC is not synchronised");
    return (false);
  }
  DEBUG_LOG("RTC is synchronised");
  return (true);
}

/**
 *  CoolTime::getNtopTime():
 *  This method is provided to get the
 *  Time through an NTP request to
 *  a Time Server
 *
 *  \return a time_t (unsigned long ) timestamp in seconds
 */
time_t CoolTime::getNtpTime() {
  while (Udp.parsePacket() > 0)
    ; // discard any previously received packets
  WiFi.hostByName(timeServer, timeServerIP);
  DEBUG_VAR("Sending NTP request to:", timeServer);
  sendNTPpacket(timeServerIP);
  uint32_t beginWait = millis();

  while (millis() - beginWait < 2000) {
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
  ERROR_LOG("No response form NTP server");
  return 0;
}

/**
 *  CoolTime::sendNTPpacket( Time Server IP address):
 *  This method is provided to send an NTP request to
 *  the time server at the given address
 */
void CoolTime::sendNTPpacket(IPAddress &address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, version, mode
  packetBuffer[1] = 0;          // stratum, or type of clock
  packetBuffer[2] = 6;          // polling interval
  packetBuffer[3] = 0xEC;       // peer clock precision
  // 8 bytes of zero for root delay & root dispersion
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

/**
 *  CoolTime::config():
 *  This method is provided to configure
 *  the CoolTime object through a configuration
 *  file.
 *
 *  \return true if successful,false otherwise
 */
bool CoolTime::config() {
  CoolConfig config("/rtcConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read RTC config");
    return (false);
  }
  JsonObject &json = config.get();

  if (json["timeServer"].success()) {
    const char *tempServer = json["timeServer"];
    for (int i = 0; i < 50; i++) {
      timeServer[i] = tempServer[i];
    }
  }
  json["timeServer"] = this->timeServer;

  if (json["localPort"].success()) {
    this->localPort = json["localPort"];
  }
  json["localPort"] = this->localPort;

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
    ERROR_LOG("failed to write RTC config");
    return (false);
  }
  return (true);
}

/**
 *  CoolTime::saveTimeSync()
 *  This method is provided to save
 *  the last sync time in the
 *  SPIFFS.
 *
 *  \return true if successful,false
 *  otherwise
 */
bool CoolTime::saveTimeSync() {
  CoolConfig config("/rtcConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read /rtcConfig.json");
    return (false);
  }
  JsonObject &json = config.get();
  json["timeSync"] = this->timeSync;
  json["NTP"] = this->NTP;
  json["compileTime"] = this->compileTime;

  if (!config.writeJsonToFile()) {
    ERROR_LOG("failed to save RTC config");
    return (false);
  }
  return (true);
}

/**
 *  CoolTime::printConf():
 *  This method is provided to print
 *  the CoolTime configuration to the
 *  Serial Monitor
 */
void CoolTime::printConf() {
  INFO_LOG("RTC configuration");
  INFO_VAR("  NTP server          :", timeServer);
  INFO_VAR("  Local port          :", localPort);
  INFO_VAR("  NTP enabled         :", NTP);
  INFO_VAR("  Use compilation date:", compileTime);
}

/**
 *  CoolTime::printDigits(digit)
 *
 *  utility method for digital clock display
 *  adds leading 0
 *
 *  \return formatted string of the input digit
 */
String CoolTime::formatDigits(int digits) {
  if (digits < 10) {
    return (String("0") + String(digits));
  }
  return (String(digits));
}
