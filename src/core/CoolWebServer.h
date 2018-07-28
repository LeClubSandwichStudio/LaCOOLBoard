#include "ArduinoJson.h"
#include "CoolWifi.h"
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

class CoolAsyncWiFiAction {
public:
  static CoolAsyncWiFiAction &getInstance();
  bool manageConnectionPortal();
  bool haveToDo = false;
  String SSID;
  String pass;
  CoolAsyncWiFiAction(CoolAsyncWiFiAction const &) = delete;
  String jsonStringWiFiScan();
  bool isAvailable(String ssid);
  // bool autoConnect();
  void operator=(CoolAsyncWiFiAction const &) = delete;

private:
  void setAPCallback(void (*func)(CoolAsyncWiFiAction *));
  void setSaveConfigCallback(void (*func)(void));
  void setBreakAfterConfig(boolean shouldBreak);
  void (*_apcallback)(CoolAsyncWiFiAction *) = NULL;
  void (*_savecallback)(void) = NULL;
  boolean _shouldBreakAfterConfig = false;
  CoolAsyncWiFiAction() {}
};