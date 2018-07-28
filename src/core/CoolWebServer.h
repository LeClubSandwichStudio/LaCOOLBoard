#include "ArduinoJson.h"
#include <ESP8266SSDP.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Hash.h>

#define HTTP_USERNAME "admin"
#define HTTP_PASSWORD "admin"

#define SSID_MAX_LENGHT 32

class CoolWebServer {
public:
  bool begin();
  void end();
  bool isRunning = false;
  void ssdpBegin();

private:
  void requestConfiguration();
  void onNotFoundConfig();
  String getCoolMac() {
    String tempMAC = WiFi.macAddress();
    tempMAC.replace(":", "");
    return (tempMAC);
  }
};

class CoolWifi {
public:
  static CoolWifi &getInstance();
  bool manageConnectionPortal();
  bool haveToDo = false;
  String SSID;
  String pass;
  CoolWifi(CoolWifi const &) = delete;
  String jsonStringWiFiScan();
  bool isAvailable(String ssid);
  bool autoConnect();
  void operator=(CoolWifi const &) = delete;
  uint8_t getWifiCount();
  bool getPublicIp(String &ip);
  static void printStatus(wl_status_t status);
  WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
  int getIndexOfMaximumValue(int16_t *array, int size);
  void setupHandlers();
  bool mdnsState = false;
  
private:
  CoolWifi() {}
};