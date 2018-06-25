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
#include "CoolWifi.h"
#include <ESP8266HTTPClient.h>

#define MAX_WIFI_NETWORKS 10
#define WIFI_CONNECT_TIMEOUT_DECISECONDS 300

void CoolWifi::connect() {
  this->config();
  INFO_LOG("Wifi connecting...");
  int i = 0;
  DEBUG_VAR("Entry time to Wifi connection attempt:", millis());
  while ((this->wifiMulti.run() != WL_CONNECTED) &&
         (i < WIFI_CONNECT_TIMEOUT_DECISECONDS)) {
    i++;
    delay(100);
  }
  DEBUG_VAR("Exit time from Wifi connection attempt:", millis());

  printStatus(WiFi.status());
}

void CoolWifi::printStatus(wl_status_t status) {
  switch (status) {
  case WL_NO_SHIELD:
    ERROR_LOG("Wifi status: no shield");
    break;
  case WL_IDLE_STATUS:
    WARN_LOG("Wifi status: idle");
    break;
  case WL_NO_SSID_AVAIL:
    ERROR_LOG("Wifi status: no SSID available");
    break;
  case WL_SCAN_COMPLETED:
    WARN_LOG("Wifi status: scan completed");
    break;
  case WL_CONNECTED:
    INFO_LOG("Wifi status: connected");
    break;
  case WL_CONNECT_FAILED:
    ERROR_LOG("Wifi status: connection failed");
    break;
  case WL_DISCONNECTED:
    WARN_LOG("Wifi status: disconnected");
    break;
  default:
    ERROR_LOG("Wifi status: unknown");
    break;
  }
}

bool CoolWifi::config() {
  CoolConfig config("/wifiConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read Wifi configuration");
    return (false);
  }
  JsonObject &json = config.get();
  config.set<uint8_t>(json, "wifiCount", this->wifiCount);
  config.set<uint8_t>(json, "timeOut", this->timeOut);

  String ssidList[this->wifiCount];
  String passList[this->wifiCount];
  for (int i = 0; i < this->wifiCount; i++) {
    String key = "Wifi" + String(i);
    if (!json[key].success()) {
      json.createNestedObject(key);
    }
    config.set<String>(json[key], "ssid", ssidList[i]);
    config.set<String>(json[key], "pass", passList[i]);
    this->wifiMulti.addAP(ssidList[i].c_str(), passList[i].c_str());
  }
  INFO_LOG("Wifi configuration loaded");
  this->printConf(ssidList);
  return (true);
}

bool CoolWifi::addWifi(String ssid, String pass) {
  INFO_VAR("Adding new Wifi network:", ssid + String(F("/")) + pass);
  CoolConfig config("/wifiConfig.json");

  if (this->wifiCount >= MAX_WIFI_NETWORKS) {
    ERROR_LOG("Cannot add new network, you have reached the limit of saved "
              "networks");
    return (false);
  }
  if (!config.readFileAsJson()) {
    ERROR_LOG("Cannot add new network, failed to read Wifi configuration");
    return (false);
  }
  JsonObject &json = config.get();

  json["wifiCount"] = this->wifiCount + 1;
  JsonObject &newWifi =
      json.createNestedObject("Wifi" + String(this->wifiCount));
  newWifi["ssid"] = ssid;
  newWifi["pass"] = pass;
  if (!config.writeJsonToFile()) {
    ERROR_LOG("Cannot add new network, failed to save Wifi configuration");
    return (false);
  }
  ++this->wifiCount;
  INFO_VAR("Added new network to Wifi configuration:",
           String(F("SSID:")) + ssid + String(F("PSK:")) + pass);
  return (true);
}

bool CoolWifi::getPublicIp(String &ip) {
  HTTPClient http;

  http.begin("http://api.ipify.org/");
  if (http.GET() == HTTP_CODE_OK) {
    ip = http.getString();
    return (true);
  }
  return (false);
}

void CoolWifi::printConf(String ssidList[]) {
  INFO_LOG("Wifi configuration");
  INFO_VAR("  Wifi count  =", this->wifiCount);
  INFO_VAR("  Timeout     =", this->timeOut);

  for (int i = 0; i < this->wifiCount; i++) {
    INFO_VAR(" Network #", i);
    INFO_VAR("    SSID    =", ssidList[i]);
  }
}
