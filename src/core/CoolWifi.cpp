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

CoolWifi &CoolWifi::getInstance() {
  static CoolWifi instance;
  return instance;
}

// bool CoolWifi::manageConnectionPortal() {
//   String tmp;
//   DynamicJsonBuffer json;
//   JsonObject &root = json.createObject();
//   if (this->haveToDo) {
//     if (this->isAvailable(this->BSSID)) {
//       DEBUG_VAR("CoolWifi: Connecting to new router", this->SSID);
//       DEBUG_VAR("CoolWifi: Entry time to Wifi connection attempt:",
//       millis()); this->connect(this->SSID.c_str(), this->pass.c_str());
//       DEBUG_VAR("CoolWifi: Connected to : ", WiFi.SSID());
//       DEBUG_VAR("Exit time from Wifi connection attempt:", millis());
//     } else {
//       root["desired"] = this->SSID;
//       root["currentSSID"] = WiFi.SSID();
//       root["status"] = this->StringStatus(WL_NO_SSID_AVAIL);
//       root.printTo(tmp);
//       events.send(tmp.c_str(), NULL, millis(), 1000);
//       return false;
//     }
//     this->haveToDo = false;
//     INFO_VAR("CoolWifi: wifi status", WiFi.status());
//     root["desired"] = this->SSID;
//     root["currentSSID"] = WiFi.SSID();
//     root["status"] = this->StringStatus(WiFi.status());
//     root.printTo(tmp);
//     events.send(tmp.c_str(), NULL, millis(), 1000);
//     this->SSID = "";
//     return true;
//   }
//   return false;
// }

#ifdef ESP32
bool CoolWifi::ethernetConnect() {
  return (ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN,
                    ETH_TYPE, ETH_CLK_MODE));
}
#endif

String CoolWifi::jsonStringWiFiScan() {
  DynamicJsonBuffer json;
  JsonObject &root = json.createObject();
  uint8_t n = WiFi.scanNetworks();
  while (WiFi.scanComplete() < 0) {
    delay(10);
  }

  INFO_VAR("Scan status: ", n);
  if (n) {
    for (uint8_t i = 0; i < n; ++i) {
      root.createNestedObject(WiFi.BSSIDstr(i));
      JsonObject &obj = root[WiFi.BSSIDstr(i)];
      obj["bssid"] = WiFi.BSSIDstr(i);
      obj["ssid"] = WiFi.SSID(i);
      obj["rssi"] = WiFi.RSSI(i);
      obj["channel"] = String(WiFi.channel(i));
      obj["secure"] = String(WiFi.encryptionType(i));
#ifdef ESP8266
      obj["hidden"] = String(WiFi.isHidden(i) ? "true" : "false");
#endif
    }
  }
  WiFi.scanDelete();
  String tmp;
  root.printTo(tmp);
  DEBUG_VAR("CoolWifi::jsonStringWiFiScan: ", tmp);
  return (tmp);
}

bool CoolWifi::isAvailable(String bssid) {
  // enhancement: need to verify also the BSSID
  DynamicJsonBuffer json;
  JsonObject &root = json.parseObject(this->jsonStringWiFiScan());
  if (root[bssid].success()) {
    return (true);
  }
  return (false);
}

// bool CoolWifi::autoConnect() {
//   DynamicJsonBuffer json;
//   JsonObject &scan = json.parseObject(this->jsonStringWiFiScan());
//   DynamicJsonBuffer config;
//   File f = SPIFFS.open("/wifiConfig.json", "r");
//   JsonObject &conf = config.parseObject(f.readString());
//   uint8_t wifiCount = conf.get<uint8_t>("wifiCount");
//   int8_t rssi[wifiCount];
//   if (wifiCount == 0) {
//     return false;
//   }
//   for (uint8_t i = 0; i < wifiCount; ++i) {
//     rssi[i] = -127;
//     if (scan[conf["Wifi" + String(i)]["ssid"].asString()].success()) {
//       DEBUG_VAR("Saved network found on scan:",
//                 String(conf["Wifi" + String(i)]["ssid"].asString()));
//       String obj = scan[conf["Wifi" + String(i)]["ssid"].asString()];
//       JsonObject &network = json.parseObject(obj);
//       int8_t pwr = network.get<int8_t>("rssi");
//       DEBUG_VAR("Signal Power:", pwr);
//       rssi[i] = pwr;
//     }
//   }
//   uint8_t n = this->getIndexOfMaximumValue(rssi, wifiCount);
//   DEBUG_VAR("CoolWifi::autoConnect index Connecting to: ", n);
//   DEBUG_VAR("CoolWifi::autoConnect Connecting to: ",
//             String(conf["Wifi" + String(n)]["ssid"].asString()));
//   DEBUG_VAR("CoolWifi: Entry time to Wifi connection attempt:", millis());
//   if (this->connect(conf["Wifi" + String(n)]["ssid"].asString(),
//                     conf["Wifi" + String(n)]["pass"].asString()) !=
//       WL_CONNECTED) {
//     WARN_VAR("There's a problem, reconnecting, WiFiStatus: ", WiFi.status());
//   }
//   DEBUG_VAR("CoolWifi::autoConnect Connected to: ", WiFi.SSID());
//   DEBUG_VAR("Exit time from Wifi connection attempt:", millis());
//   return (true);
// }

bool CoolWifi::autoConnect() {
#ifdef ESP32
  if (this->ethernetConnect()) {
    return true;
  }
#endif
  DynamicJsonBuffer json;
  JsonObject &scan = json.parseObject(this->jsonStringWiFiScan());
  DynamicJsonBuffer config;
  File f = SPIFFS.open("/wifiConfig.json", "r");
  JsonArray &conf = config.parseArray(f.readString());
  uint8_t wifiSize = conf.size();
  int8_t rssi[wifiSize];
  if (wifiSize == 0) {
    return false;
  }
  for (uint8_t i = 0; i < wifiSize; ++i) {
    rssi[i] = -127;
    if (scan[conf[i]["bssid"].asString()].success()) {
      DEBUG_VAR("Saved network found on scan:", conf[i]["bssid"].asString());
      DEBUG_VAR("With SSID:", conf[i]["ssid"].asString());
      DEBUG_VAR("With PSK:", conf[i]["psk"].asString());
      String obj = scan[conf[i]["bssid"].asString()];
      JsonObject &network = json.parseObject(obj);
      int16_t pwr = network.get<int16_t>("rssi");
      DEBUG_VAR("Signal Power:", pwr);
      rssi[i] = pwr;
    }
  }
  uint8_t n = this->getIndexOfMaximumValue(rssi, wifiSize);
  DEBUG_VAR("CoolWifi::autoConnect index Connecting to: ", n);
  DEBUG_VAR("CoolWifi::autoConnect Connecting to: ",
            String(conf[n]["ssid"].asString()));
  DEBUG_VAR("CoolWifi: Entry time to Wifi connection attempt:", millis());
  DEBUG_VAR("With SSID:", conf[n]["ssid"].asString());
  DEBUG_VAR("With Psk:", conf[n]["psk"].asString());
  if (this->connect(conf[n]["ssid"].asString(), conf[n]["psk"].asString()) !=
      WL_CONNECTED) {
    WARN_VAR("There's a problem, reconnecting, WiFiStatus: ", WiFi.status());
    return (false);
  }
  DEBUG_VAR("CoolWifi::autoConnect Connected to: ", WiFi.SSID());
  DEBUG_VAR("Exit time from Wifi connection attempt:", millis());
  return (true);
}

uint8_t CoolWifi::connect(String ssid, String pass) {
#ifdef ESP8266
  ETS_UART_INTR_DISABLE();
  wifi_station_disconnect();
  ETS_UART_INTR_ENABLE();
#elif ESP32
  esp_wifi_disconnect();
#endif
  WiFi.begin(ssid.c_str(), pass.c_str());
  int t = 0;
  while ((WiFi.status() != WL_CONNECTED) &&
         (t < WIFI_CONNECT_TIMEOUT_SECONDS)) {
    t++;
    delay(1000);
  }
  if ((WiFi.status() != WL_CONNECTED) && this->isAvailable(ssid)) {
    WARN_LOG("Something goes wrong, SSID is available but impossible to "
             "connect, please retry");
    WARN_VAR("Reason: ", this->StringStatus(WiFi.status()));
    WiFi.persistent(false);
#ifdef ESP8266
    wifi_station_disconnect();
#elif ESP32
    esp_wifi_disconnect();
#endif
    WiFi.mode(WIFI_OFF);
    delay(1000);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    int t = 0;
    while ((WiFi.status() != WL_CONNECTED) &&
           (t < WIFI_CONNECT_TIMEOUT_SECONDS)) {
      t++;
      delay(1000);
    }
  }
  return WiFi.status();
}

bool CoolWifi::connectToSavedBssidAsync(String bssid) {
  DynamicJsonBuffer config;
  File f = SPIFFS.open("/wifiConfig.json", "r");
  if (!f)
    return false;
  JsonArray &conf = config.parseArray(f.readString());
  uint8_t wifiSize = conf.size();
  DEBUG_VAR("Config Size: ", wifiSize);
  for (uint8_t i = 0; i < wifiSize; ++i) {
    String tmp = conf.get<String>(i);
    JsonObject &obj = config.parseObject(tmp.c_str());
    if (obj.get<String>("bssid") == bssid) {
      this->SSID = obj["ssid"].asString();
      this->pass = obj["psk"].asString();
      this->BSSID = bssid;
      DEBUG_VAR("Setting connection by based BSSID, SSID: ", this->SSID);
      DEBUG_VAR("Setting connection by based BSSID, psk: ", this->pass);
      this->haveToDo = true;
      return true;
    }
  }
  return false;
}

uint8_t CoolWifi::getWifiCount() {
  DynamicJsonBuffer config;
  File f = SPIFFS.open("/wifiConfig.json", "r");
  JsonArray &conf = config.parseArray(f.readString());
  f.close();
  return (conf.size());
}
uint8_t CoolWifi::getIndexOfMaximumValue(int8_t *array, int size) {
  int8_t maxIndex = 0;
  int8_t max = array[maxIndex];
  for (int i = 0; i < size; i++) {
    if (max < array[i]) {
      max = array[i];
      maxIndex = i;
    }
  }
#ifdef ESP8266
  if (array[maxIndex] < WIFI_POOR_SIGNAL_THREESHOLD) {
    DEBUG_LOG("Best signal to low, enhance WiFi power");
    WiFi.setOutputPower(WIFI_MAX_POWER);
  }
#endif
  return maxIndex;
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

String CoolWifi::getStatusAsjsonString() {
  DynamicJsonBuffer json;
  JsonObject &root = json.createObject();
  root["state"] = this->StringStatus(WiFi.status());
  root.createNestedObject("network");
  JsonObject &network = root["network"];
  network["ssid"] = WiFi.SSID();
  network["bssid"] = WiFi.BSSIDstr();
  network["rssi"] = WiFi.RSSI();
  // network["security"] = WiFi.encryptionType();
  // network["hidden"] = String(WiFi.isHidden() ? "true" : "false");
  String tmp;
  root.printTo(tmp);
  return tmp;
}

String CoolWifi::StringStatus(wl_status_t status) {
  switch (status) {
  case WL_NO_SHIELD:
    return "Wifi status: no shield";
    break;
  case WL_IDLE_STATUS:
    return "Wifi status: idle";
    break;
  case WL_NO_SSID_AVAIL:
    return "Wifi status: no SSID available";
    break;
  case WL_SCAN_COMPLETED:
    return "Wifi status: scan completed";
    break;
  case WL_CONNECTED:
    return "Wifi status: connected";
    break;
  case WL_CONNECT_FAILED:
    return "Wifi status: connection failed";
    break;
  case WL_DISCONNECTED:
    return "Wifi status: disconnected";
    break;
  default:
    return "Wifi status: unknown";
    break;
  }
}

void CoolWifi::setupHandlers() {
#ifdef ESP8266
  gotIpEventHandler =
      WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP &event) {
        INFO_VAR("Coolboard Connected, Local IP: ", WiFi.localIP());
        CoolWifi::getInstance().lostConnections = 0;
      });
  disconnectedEventHandler = WiFi.onStationModeDisconnected(
      [](const WiFiEventStationModeDisconnected &event) {
        INFO_LOG("WiFi connection lost");
        CoolWifi::getInstance().lostConnections++;
        if (CoolWifi::getInstance().lostConnections >= 10) {
          ERROR_LOG("impossible to establish connection, need to reboot");
          ESP.restart();
        }
      });
#elif ESP32
  WiFi.onEvent(this->WiFiEthEvent);
#endif
}
#ifdef ESP32
void CoolWifi::WiFiEthEvent(WiFiEvent_t event) {
  switch (event) {
  case SYSTEM_EVENT_ETH_START:
    Serial.println("ETH Started");
    ETH.setHostname("esp32-ethernet");
    break;
  case SYSTEM_EVENT_ETH_CONNECTED:
    Serial.println("ETH Connected");
    break;
  case SYSTEM_EVENT_ETH_GOT_IP:
    Serial.print("ETH MAC: ");
    Serial.print(ETH.macAddress());
    Serial.print(", IPv4: ");
    Serial.print(ETH.localIP());
    if (ETH.fullDuplex()) {
      Serial.print(", FULL_DUPLEX");
    }
    Serial.print(", ");
    Serial.print(ETH.linkSpeed());
    Serial.println("Mbps");
    CoolWifi::getInstance().ethConnected = true;
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:
    Serial.println("ETH Disconnected");
    CoolWifi::getInstance().ethConnected = false;
    break;
  case SYSTEM_EVENT_ETH_STOP:
    Serial.println("ETH Stopped");
    CoolWifi::getInstance().ethConnected = false;
    break;
  default:
    break;
  }
}
#endif