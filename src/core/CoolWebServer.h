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