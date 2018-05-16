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
#include "WiFiManagerReadFileButton.h"

#define MAX_WIFI_NETWORKS 10

wl_status_t CoolWifi::state() { return (WiFi.status()); }

wl_status_t CoolWifi::disconnect() {
  WiFi.disconnect();
  DEBUG_VAR("Wifi status:", WiFi.status());
  return (WiFi.status());
}

wl_status_t CoolWifi::connect() {
  INFO_LOG("Wifi connecting...");
  this->connectWifiMulti();
  return (WiFi.status());
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

wl_status_t CoolWifi::connectWifiMulti() {
  int i = 0;

  DEBUG_VAR("Entry time to Wifi connection attempt:", millis());
  while ((this->wifiMulti.run() != WL_CONNECTED) && (i < 300)) {
    i++;
    delay(100);
  }
  DEBUG_VAR("Exit time from Wifi connection attempt:", millis());

  wl_status_t status = WiFi.status();
  printStatus(status);
  return (status);
}

wl_status_t CoolWifi::connectAP() {
  WiFiManager wifiManager;
  String tempMAC = WiFi.macAddress();

  wifiManager.setRemoveDuplicateAPs(true);
  wifiManager.setTimeout(this->timeOut);
  tempMAC.replace(":", "");

  String name = "CoolBoard-" + tempMAC;

  wifiManager.autoConnect(name.c_str());
  wl_status_t status = WiFi.status();

  if (status == WL_CONNECTED) {
    INFO_VAR("Wifi network selected:", WiFi.SSID());
    this->addWifi(WiFi.SSID(), WiFi.psk());
  } else {
    ERROR_LOG("No Wifi network was configured.");
  }
  printStatus(status);
  return (status);
}

bool CoolWifi::config() {
  CoolConfig config("/wifiConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read Wifi configuration");
    return (false);
  }
  JsonObject &json = config.get();
  if (json["wifiCount"].success()) {
    this->wifiCount = json["wifiCount"];
  }
  json["wifiCount"] = this->wifiCount;
  String ssidList[this->wifiCount];
  String passList[this->wifiCount];
  if (json["timeOut"].success()) {
    this->timeOut = json["timeOut"];
  }
  json["timeOut"] = this->timeOut;
  for (uint8_t i = 0; i < this->wifiCount; i++) {
    if (json["Wifi" + String(i)].success()) {
      if (json["Wifi" + String(i)]["ssid"].success()) {
        const char *tempSsid = json["Wifi" + String(i)]["ssid"];
        ssidList[i] = tempSsid;
      }
      json["Wifi" + String(i)]["ssid"] = ssidList[i].c_str();

      if (json["Wifi" + String(i)]["pass"].success()) {
        const char *tempPass = json["Wifi" + String(i)]["pass"];
        passList[i] = tempPass;
      }
      json["Wifi" + String(i)]["pass"] = passList[i].c_str();
    }
    json["Wifi" + String(i)]["ssid"] = ssidList[i].c_str();
    json["Wifi" + String(i)]["pass"] = passList[i].c_str();
    this->wifiMulti.addAP(ssidList[i].c_str(), passList[i].c_str());
  }
  if (!config.writeJsonToFile()) {
    ERROR_LOG("Failed to save Wifi configuration");
    return (false);
  }
  DEBUG_LOG("Wifi configuraton loaded");
  this->printConf(ssidList);
  return (true);
}

bool CoolWifi::addWifi(String ssid, String pass) {
  CoolConfig config("/wifiConfig.json");

  if (this->wifiCount >= MAX_WIFI_NETWORKS) {
    ERROR_LOG("Cannot add new network, you have reached the limit of 50 saved "
              "networks");
    return (false);
  }
  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read Wifi configuration");
    return (false);
  }
  JsonObject &json = config.get();

  json["wifiCount"] = ++this->wifiCount;
  JsonObject &newWifi =
      json.createNestedObject("Wifi" + String(this->wifiCount - 1));
  newWifi["ssid"] = ssid;
  newWifi["pass"] = pass;
  if (!config.writeJsonToFile()) {
    ERROR_LOG("Failed to save Wifi configuration");
    return (false);
  }
  INFO_VAR("Added new network to Wifi configuration:",
           String(F("SSID:")) + ssid + String(F("PSK:")) + pass);
  return (true);
}

String CoolWifi::getExternalIP() {
  WiFiClient client;
  String IP;

  if (!client.connect("api.ipify.org", 80)) {
    DEBUG_LOG("Failed to connect to http://api.ipify.org");
  } else {
    int timeout = millis() + 800;
    client.print("GET /?format=json HTTP/1.1\r\nHost: api.ipify.org\r\n\r\n");
    while (client.available() == 0) {
      if (timeout - millis() < 0) {
        ERROR_LOG("Failed to get public IP (client timeout");
      }
      yield();
    }
    while (client.available()) {
      char msg = client.read();
      IP += msg;
    }
    client.stop();
    DEBUG_VAR("Public IP address:", IP);
  }
  // return only the IP in the string
  return IP.substring(IP.indexOf("{") + 6, IP.lastIndexOf("}"));
}

void CoolWifi::printConf(String ssidList[]) {
  INFO_LOG("Wifi configuration");
  INFO_VAR("  Wifi count = ", this->wifiCount);

  for (int i = 0; i < this->wifiCount; i++) {
    INFO_VAR(" Network #", i);
    INFO_VAR("    SSID    = ", ssidList[i]);
  }
  INFO_VAR("  Timeout =", this->timeOut);
}
