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

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ArduinoJson.h>

#include "internals/WiFiManagerReadFileButton.h"

#include "CoolLog.h"
#include "CoolWifi.h"
#include "CoolConfig.h"

/**
 *  CoolWifi::begin():
 *  This method is provided to set the
 *  wifiMulti Access points and the
 *  wifiManager time out
 */
void CoolWifi::begin() {
  DEBUG_LOG("Entering CoolWifi.begin()");
  for (int i = 0; i < this->wifiCount; i++) {
    this->wifiMulti.addAP(this->ssid[i].c_str(), this->pass[i].c_str());
  }
}

/**
 *  CoolWifi::state():
 *  This method is provided to return the
 *  Wifi client's state.
 *  \return wifi client state:
 *    WL_NO_SHIELD        = 255,
 *        WL_IDLE_STATUS      = 0,
 *        WL_NO_SSID_AVAIL    = 1,
 *        WL_SCAN_COMPLETED   = 2,
 *        WL_CONNECTED        = 3,
 *        WL_CONNECT_FAILED   = 4,
 *        WL_CONNECTION_LOST  = 5,
 *    WL_DISCONNECTED = 6
 */
wl_status_t CoolWifi::state() {
  DEBUG_LOG("Entering CoolWifi.state()");
  DEBUG_VAR("Wifi status:", WiFi.status());
  return (WiFi.status());
}

/**
 *  CoolWifi::disconnect():
 *  This method is provided to disconnect
 *   from current WiFi network and returns
 *  the Wifi client's state.
 *  \return wifi client state:
 *    WL_NO_SHIELD        = 255,
 *        WL_IDLE_STATUS      = 0,
 *        WL_NO_SSID_AVAIL    = 1,
 *        WL_SCAN_COMPLETED   = 2,
 *        WL_CONNECTED        = 3,
 *        WL_CONNECT_FAILED   = 4,
 *        WL_CONNECTION_LOST  = 5,
 *    WL_DISCONNECTED = 6
 */
wl_status_t CoolWifi::disconnect() {
  DEBUG_LOG("Entering CoolWifi.disconnect()");
  WiFi.disconnect();
  DEBUG_VAR("Wifi status:", WiFi.status());
  return (WiFi.status());
}

/**
 *  CoolWifi::connect( ):
 *  This method is provided to connect to the strongest WiFi
 *  in the provided list of wiFis.
 *  If none are found , it starts the AP mode.
 *
 *  \return wifi state
 */
wl_status_t CoolWifi::connect() {
  DEBUG_LOG("Entering CoolWifi.connect()");
  DEBUG_LOG("Wifi connecting...");

  if (this->wifiCount != 0) {
    this->connectWifiMulti();
    // if nomad is true, not start AP
    if (this->nomad == true) {
      DEBUG_LOG("Board is in nomad mode");
      DEBUG_VAR("Wifi status:", WiFi.status());
      return (WiFi.status());
    }
  } else {
    WiFiManager wifiManager;
    wifiManager.resetSettings();
  }
  if (WiFi.status() != WL_CONNECTED) {
    ERROR_LOG("No trusted Wifi network in range, starting access point");
    this->connectAP();
  } else {
    DEBUG_VAR("Wifi connected to", WiFi.SSID());
  }
  return (WiFi.status());
}

/**
 *  CoolWifi::connectWifiMulti()
 *  This function is provided to
 *  run the WifiMulti part of the
 *  Wifi connection process
 *
 *  \return wifi state
 */
wl_status_t CoolWifi::connectWifiMulti() {
  DEBUG_LOG("Entering CoolWifi.connectWifiMulti()");

  int i = 0;

  DEBUG_VAR("Current time:", millis());
  while ((this->wifiMulti.run() != WL_CONNECTED) && (i < 500)) {
    i++;
    delay(5);
  }
  DEBUG_VAR("Time after connection:", millis());
  DEBUG_VAR("Wifi status:", WiFi.status());
  return (WiFi.status());
}

/**
 *  CoolWifi::connectAP()
 *  This function is provided to
 *  run the WifiManager part of the
 *  Wifi connection process
 *
 *  \return wifi state
 */
wl_status_t CoolWifi::connectAP() {
  DEBUG_LOG("Entering CoolWifi.connectAP()");

  WiFiManager wifiManager;

  wifiManager.setRemoveDuplicateAPs(true);
  wifiManager.setTimeout(this->timeOut);

  String tempMAC = WiFi.macAddress();
  tempMAC.replace(":", "");

  String name = "CoolBoard-" + tempMAC;

  if (!wifiManager.autoConnect(name.c_str())) {
    ERROR_LOG("Failed to connect AP (timeout)");
    delay(30);
  }

  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_VAR("Connected to AP:", WiFi.SSID());
    this->addWifi(WiFi.SSID(), WiFi.psk());
  } else {
    ERROR_LOG("Failed to connect AP");
  }
  return (WiFi.status());
}

/**
 *  CoolWifi::config():
 *  This method is provided to set
 *  the wifi parameters :  -ssid
 *        -pass
 *        -AP timeOut
 *        -wifiCount
 *
 *  \return true if successful,false otherwise
 */
bool CoolWifi::config() {
  DEBUG_LOG("Entering CoolWifi.config()");

  CoolConfig config("/wifiConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to configure Wifi");
    return (false);
  }
  JsonObject &json = config.get();

  if (json["wifiCount"].success()) {
    this->wifiCount = json["wifiCount"];
  }
  json["wifiCount"] = this->wifiCount;

  if (json["timeOut"].success()) {
    this->timeOut = json["timeOut"];
  }
  json["timeOut"] = this->timeOut;

  if (json["nomad"].success()) {
    this->nomad = json["nomad"];
  }
  json["nomad"] = this->nomad;

  for (int i = 0; i < this->wifiCount; i++) {
    if (json["Wifi" + String(i)].success()) {
      if (json["Wifi" + String(i)]["ssid"].success()) {
        const char *tempSsid = json["Wifi" + String(i)]["ssid"];
        this->ssid[i] = tempSsid;
      }
      json["Wifi" + String(i)]["ssid"] = this->ssid[i].c_str();

      if (json["Wifi" + String(i)]["pass"].success()) {
        const char *tempPass = json["Wifi" + String(i)]["pass"];
        this->pass[i] = tempPass;
      }
      json["Wifi" + String(i)]["pass"] = this->pass[i].c_str();
    }
    json["Wifi" + String(i)]["ssid"] = this->ssid[i].c_str();
    json["Wifi" + String(i)]["pass"] = this->pass[i].c_str();
  }
  if (!config.writeJsonToFile()) {
    ERROR_LOG("Failed to save Wifi configuration");
    return (false);
  }
  return (true);
}

/**
 *  CoolWifi::printConf():
 *  This method is provided to print the
 *  configuration to the Serial Monitor
 */
void CoolWifi::printConf() {
  DEBUG_LOG("Entering CoolWifi.printConf()");
  INFO_LOG("Wifi configuration");
  INFO_VAR("Wifi count:", this->wifiCount);

  for (int i = 0; i < this->wifiCount; i++) {
    INFO_VAR("SSID:", this->ssid[i]);
  }
  INFO_VAR("Timeout:", this->timeOut);
  INFO_VAR("Nomad  :", this->nomad);
}

/**
 *  CoolWifi::addWifi(ssid,pass)
 *  This method is provided to add new WiFi
 *  detected by the WiFiManager to
 *  the jsonConfig(if used )
 *
 *  \return true if successfull , false otherwise
 */
bool CoolWifi::addWifi(String ssid, String pass) {
  DEBUG_LOG("Entering CoolWifi.addWifi()");
  this->wifiCount++;

  if (this->wifiCount >= 50) {
    DEBUG_LOG("You have reached the limit of 50 networks");
    return (false);
  }
  this->ssid[this->wifiCount - 1] = ssid;
  this->pass[this->wifiCount - 1] = pass;
  File configFile = SPIFFS.open("/wifiConfig.json", "r");

  if (!configFile) {
    ERROR_LOG("Failed to read /wifiConfig.json");
  } else {
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());

    if (!json.success()) {
      ERROR_LOG("failed to parse Wifi config from file");
    } else {
      DEBUG_JSON("Wifi config JSON:", json);
      DEBUG_VAR("JSON buffer size:", jsonBuffer.size());

      if (json["wifiCount"].success()) {
        json["wifiCount"] = this->wifiCount;
      }
      json["wifiCount"] = this->wifiCount;

      if (json["timeOut"].success()) {
        this->timeOut = json["timeOut"];
      }
      json["timeOut"] = this->timeOut;
      JsonObject &newWifi =
          json.createNestedObject("Wifi" + String(this->wifiCount - 1));
      newWifi["ssid"] = this->ssid[this->wifiCount - 1];
      newWifi["pass"] = this->pass[this->wifiCount - 1];
      configFile.close();
      configFile = SPIFFS.open("/wifiConfig.json", "w");

      if (!configFile) {
        ERROR_LOG("failed to write to /wifiConfig.json");
      }
      json.printTo(configFile);
      configFile.close();
      DEBUG_LOG("Saved Wifi config to /wifiConfig.json");
      return (true);
    }
  }
  return (true);
}

/**
 *  CoolWifi::getExternalIP():
 *  This method is provided to print the
 *  public IP of a device to a String
 */
String CoolWifi::getExternalIP() {
  DEBUG_LOG("Entering CoolWifi.getExternalIP()");

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