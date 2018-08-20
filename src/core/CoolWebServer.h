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
#define WIFI_POOR_SIGNAL_THREESHOLD -75
#define WIFI_MAX_POWER 20.5
#define WIFI_CONNECT_TIMEOUT_SECONDS 25
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
  String BSSID;
  CoolWifi(CoolWifi const &) = delete;
  String jsonStringWiFiScan();
  bool isAvailable(String bssid);
  bool autoConnect();
  void operator=(CoolWifi const &) = delete;
  uint8_t getWifiCount();
  bool getPublicIp(String &ip);
  String StringStatus(wl_status_t status);
  WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
  uint8_t getIndexOfMaximumValue(int8_t *array, int size);
  void setupHandlers();
  bool mdnsState = false;
  uint8_t connect(String ssid, String pass);
  bool connectToSavedBssidAsync(String bssid);
  int lostConnections = -1;
  String getStatusAsjsonString();
private:
  CoolWifi() {}
};