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

#ifndef COOLWIFI_H
#define COOLWIFI_H

#define WIFI_POOR_SIGNAL_THREESHOLD -75
#define WIFI_MAX_POWER 20.5
#define WIFI_CONNECT_TIMEOUT_SECONDS 25
#define SSID_MAX_LENGHT 32

#include <Arduino.h>
#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>
#define BOOTSTRAP_PIN 0
#elif ESP32
#include <ETH.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <WiFiMulti.h>
#include <esp_wifi.h>
#define BOOTSTRAP_PIN A0 // need to nefine in HW specs

#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN 5
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_ADDR PHY1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18
#endif
// #include "CoolBoardLed.h"

class CoolWifi {
public:
  static CoolWifi &getInstance();
  bool manageConnectionPortal();
  bool haveToDo = false;
  String SSID;
  String pass;
  String BSSID;
  CoolWifi(CoolWifi const &) = delete;
  String jsonStringWiFiScan();
  bool isAvailable(String bssid);
  bool autoConnect();
  void operator=(CoolWifi const &) = delete;
  uint8_t getWifiCount();
  bool getPublicIp(String &ip);
  String StringStatus(wl_status_t status);
#ifdef ESP8266
  WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
#elif ESP32
  static void WiFiEthEvent(WiFiEvent_t event);
#endif
  bool ethConnected = false;
  uint8_t getIndexOfMaximumValue(int8_t *array, int size);
  void setupHandlers();
  bool mdnsState = false;
  uint8_t connect(String ssid, String pass);
  bool connectToSavedBssidAsync(String bssid);
  int lostConnections = -1;
  String getStatusAsjsonString();
#ifdef ESP32
  bool ethernetConnect();
#endif
private:
  CoolWifi() {}
};

#endif
