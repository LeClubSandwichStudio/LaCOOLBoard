#include "CoolAsyncEditor.h"
#include "CoolConfig.h"
#include "WiFiUdp.h"
#include <CoolLog.h>
#include <CoolWebServer.h>
#include <ESP8266HTTPClient.h>
#include <stdio.h>
#include <user_interface.h>

AsyncWebServer server(80);
AsyncEventSource events("/events");

String handleMessageReceived;

bool CoolWebServer::begin() {
  DEBUG_LOG("AsyncWebServer begin");

  CoolConfig config("/coolBoardConfig.json");
  JsonObject &json = config.get();
  this->isRunning = true;
  config.set<bool>(json, "webServer", isRunning, true);
  String tempMAC = WiFi.macAddress();
  tempMAC.replace(":", "");
  String name = "CoolBoard-" + tempMAC;
  WiFi.hostname(name.c_str());
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(name.c_str());
  INFO_VAR("Local ip:", WiFi.localIP());
  if (!SPIFFS.begin()) {
    return (false);
  }
  events.onConnect([](AsyncEventSourceClient *client) {
    client->send("hello!", NULL, millis(), 1000);
  });
  server.addHandler(&events);
  this->requestConfiguration();
  if (CoolAsyncEditor::getInstance().beginAdminCredential()) {
    server.serveStatic("/", SPIFFS, "/")
        .setDefaultFile("index.htm")
        .setAuthentication(CoolAsyncEditor::getInstance().HTTPuserName.c_str(),
                           CoolAsyncEditor::getInstance().HTTPpassword.c_str());
  } else {
    server.serveStatic("/", SPIFFS, "/")
        .setDefaultFile("index.htm")
        .setAuthentication(HTTP_USERNAME, HTTP_PASSWORD);
  }
  this->onNotFoundConfig();
#if ASYNC_TCP_SSL_ENABLED
  server.beginSecure(
      CoolAsyncEditor::getInstance().read("/certificate.bin").c_str(),
      CoolAsyncEditor::getInstance().read("/privateKey.bin").c_str(), "admin");
#else
  server.begin();
#endif
  INFO_VAR("CoolBoard WebServer started! with SSID: ", name);
  return (true);
}

void CoolWebServer::end() {
  if (this->isRunning) {
    events.close();
    WiFiUDP udp;
    udp.stopAll();
    WiFi.mode(WIFI_STA);
    this->isRunning = false;
    CoolConfig config("/coolBoardConfig.json");
    JsonObject &json = config.get();
    config.set<bool>(json, "webServer", isRunning, true);
    DEBUG_LOG("AsyncWebServer disconnected!");
  } else {
    DEBUG_LOG("AsyncWebServer is already closed!");
  }
}

void CoolWebServer::requestConfiguration() {
  server.on("/add/wifi", HTTP_POST,
            [](AsyncWebServerRequest *request) { request->send(200); },
            [](AsyncWebServerRequest *request, String filename, size_t index,
               uint8_t *data, size_t len, bool final) {
              if (!index) {
                DEBUG_VAR("BodyStart: ", filename.length());
              }
              for (size_t i = 0; i < len; i++) {
                handleMessageReceived += (const char)data[i];
              }
              if (index + len == filename.length()) {
                Serial.printf("BodyEnd: %u B\n", filename.length());
              }
              if (final) {
                INFO_VAR("File len: ", (uint32_t)len);
                INFO_VAR("File name: ", filename.c_str());
                INFO_VAR("File data: ", String((char *)data));
                INFO_VAR("File data: ", handleMessageReceived);
                handleMessageReceived = "";
              }
            },
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
               size_t index, size_t total) {
              if (!index) {
                DEBUG_VAR("BodyStart: ", total);
              }
              for (size_t i = 0; i < len; i++) {
                handleMessageReceived += (const char)data[i];
              }
              if (index + len == total) {
                DEBUG_VAR("BodyEnd: %u B\n", total);
                StaticJsonBuffer<256> jsonBuffer;
                JsonObject &root =
                    jsonBuffer.parseObject(handleMessageReceived);
                if (root["ssid"].success() && root["pass"].success()) {
                  INFO_VAR("New SSID received:", root.get<String>("ssid"));
                  DEBUG_VAR("pass: :", root.get<String>("pass"));
                  if (CoolAsyncEditor::getInstance().addNewWifi(
                          root.get<String>("ssid"), root.get<String>("pass"))) {
                    request->send(201);
                  } else {
                    request->send(500);
                  }
                } else {
                  request->send(415);
                }
                handleMessageReceived = "";
              }
            });

  server.on("/reset/wifi", HTTP_POST,
            [](AsyncWebServerRequest *request) { request->send(200); },
            [](AsyncWebServerRequest *request, String filename, size_t index,
               uint8_t *data, size_t len, bool final) {
              if (!index) {
                DEBUG_VAR("BodyStart: ", filename.length());
              }
              for (size_t i = 0; i < len; i++) {
                handleMessageReceived += (const char)data[i];
              }
              if (index + len == filename.length()) {
                Serial.printf("BodyEnd: %u B\n", filename.length());
              }
              if (final) {
                INFO_VAR("File len: ", (uint32_t)len);
                INFO_VAR("File name: ", filename.c_str());
                INFO_VAR("File data: ", String((char *)data));
                INFO_VAR("File data: ", handleMessageReceived);
                handleMessageReceived = "";
              }
            },
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
               size_t index, size_t total) {
              if (!index) {
                DEBUG_VAR("total: ", total);
              }
              for (size_t i = 0; i < len; i++) {
                handleMessageReceived += (const char)data[i];
              }
              if (index + len == total) {
                DEBUG_VAR("BodyEnd: ", total);
                DEBUG_VAR("handleMessageReceived: ", handleMessageReceived);
                CoolAsyncEditor::getInstance().reWriteWifi(
                    handleMessageReceived);
                request->send(201);
                handleMessageReceived = "";
              }
            });

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
    INFO_LOG("Scanning WiFi networks");
    String json = "[";
    uint8_t n = WiFi.scanComplete();
    if (n == -2) {
      WiFi.scanNetworks(true);
    } else if (n) {
      for (uint8_t i = 0; i < n; ++i) {
        if (i)
          json += ",";
        json += "{";
        json += "\"rssi\":" + String(WiFi.RSSI(i));
        json += ",\"ssid\":\"" + WiFi.SSID(i) + "\"";
        json += ",\"bssid\":\"" + WiFi.BSSIDstr(i) + "\"";
        json += ",\"channel\":" + String(WiFi.channel(i));
        json += ",\"secure\":" + String(WiFi.encryptionType(i));
        json += ",\"hidden\":" + String(WiFi.isHidden(i) ? "true" : "false");
        json += "}";
      }
      WiFi.scanDelete();
      if (WiFi.scanComplete() == -2) {
        WiFi.scanNetworks(true);
      }
    }
    json += "]";
    DEBUG_VAR("WiFi scan results: ", json);
    request->send(200, "text/json", json);
    json = String();
  });

  server.on("/wifi/saved", HTTP_GET, [](AsyncWebServerRequest *request) {
    int params = request->params();
    if (params == 0) {
      request->send(200, "text/json",
                    CoolAsyncEditor::getInstance().read("/wifiConfig.json"));
    }
    for (int i = 0; i < params; i++) {
      AsyncWebParameter *p = request->getParam(i);
      if (!p->isFile() && !p->isPost()) {
        Serial.printf("GET[%s]: WiFi:%s\n", p->name().c_str(),
                      p->value().c_str());
        if (CoolAsyncEditor::getInstance().getSavedWifi(p->value()) == "") {
          request->send(404);
        } else {
          request->send(
              200, "text/json",
              CoolAsyncEditor::getInstance().getSavedWifi(p->value()));
        }
      }
    }
  });

  server.on("/wifi/connect", HTTP_GET, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for (int i = 0; i < params; i++) {
      AsyncWebParameter *p = request->getParam(i);
      if (!p->isFile() && !p->isPost()) {
        Serial.printf("GET[%s]: WiFi:%s\n", p->name().c_str(),
                      p->value().c_str());
        if (CoolAsyncEditor::getInstance().getSavedWifi(p->value()) == "") {
          request->send(404);
        } else {
          CoolWifi::getInstance().SSID =
              CoolAsyncEditor::getInstance().getSavedCredentialFromIndex(
                  atoi(p->value().c_str()), "ssid");
          CoolWifi::getInstance().pass =
              CoolAsyncEditor::getInstance().getSavedCredentialFromIndex(
                  atoi(p->value().c_str()), "pass");
          CoolWifi::getInstance().haveToDo = true;
        }
      }
    }
    request->send(200);
  });

  // server.on("/wifi/discover", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   int params = request->params();
  //   String service = "cool-api";
  //   for (int i = 0; i < params; i++) {
  //     AsyncWebParameter *p = request->getParam(i);
  //     if (!p->isFile() && !p->isPost()) {
  //       service = p->value();
  //     }
  //   }

  //   DEBUG_VAR("Service: ", service);
  //   if (CoolWifi::getInstance().mdnsState) {
  //     DynamicJsonBuffer json;
  //     JsonObject &root = json.createObject();
  //     uint8_t n = MDNS.queryService(service, "tcp");
  //     DEBUG_VAR("Devices queried: ", n);
  //     for (uint8_t i = 0; i < n; ++i) {
  //       // if (n)
  //       root.createNestedObject(MDNS.hostname(i));
  //       JsonObject &obj = root[MDNS.hostname(i)];
  //       obj["ip"] = String(MDNS.IP(i));
  //       obj["port"] = MDNS.port(i);
  //       obj["service"] = service;
  //       DEBUG_VAR("ip: ", String(MDNS.IP(i)));
  //       DEBUG_VAR("port: ", MDNS.port(i));
  //     }
  //     String buf;react-native link react-native-zeroconf
  //     root.printTo(buf);
  //     DEBUG_VAR("Json result:", buf);
  //     request->send(200, "text/json", buf);
  //   } else {
  //     DEBUG_LOG("Bonjour Service never initialized, can't discover devices on
  //     "
  //               "network");
  //                request->send(404);
  //   }
  // });

  server.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest *request) {
    String descriptor = CoolAsyncEditor::getInstance().getSdpConfig();
    request->send(200, "text/xml", descriptor);
  });
}

void CoolWebServer::onNotFoundConfig() {
  server.onNotFound([](AsyncWebServerRequest *request) {
    Serial.printf("NOT_FOUND: ");
    if (request->method() == HTTP_GET)
      Serial.printf("GET");
    else if (request->method() == HTTP_POST)
      Serial.printf("POST");
    else if (request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if (request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if (request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if (request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if (request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(),
                  request->url().c_str());

    if (request->contentLength()) {
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }
    int headers = request->headers();
    int i;
    for (i = 0; i < headers; i++) {
      AsyncWebHeader *h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }
    int params = request->params();
    for (i = 0; i < params; i++) {
      AsyncWebParameter *p = request->getParam(i);
      if (p->isFile()) {
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(),
                      p->value().c_str(), p->size());
      } else if (p->isPost()) {
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());

      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
}
void CoolWebServer::ssdpBegin() {
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("CoolBoard");
  SSDP.setModelName("CoolBoard");
  SSDP.setURL("/index.htm");
  SSDP.begin();
  String coolName = "CoolBoard-" + this->getCoolMac();
  SSDP.setDeviceType("upnp:rootdevice");
  MDNS.addService("cool-api", "tcp", 80);
  MDNS.addServiceTxt("cool-api", "tcp", "Firmware", COOL_FW_VERSION);
  MDNS.addServiceTxt("cool-api", "tcp", "coreVersion", ESP.getCoreVersion());
  MDNS.addServiceTxt("cool-api", "tcp", "sdkVersion", ESP.getSdkVersion());
  MDNS.addServiceTxt("cool-api", "tcp", "firwmareMD5", ESP.getSketchMD5());
  MDNS.addServiceTxt("cool-api", "tcp", "fullVersion", ESP.getFullVersion());
  if (MDNS.begin(coolName.c_str())) {
    INFO_VAR("Bonjour service started at: ", coolName);
  } else {
    WARN_LOG("Fail to start Bonjour service");
  }
}

CoolWifi &CoolWifi::getInstance() {
  static CoolWifi instance;
  return instance;
}

bool CoolWifi::manageConnectionPortal() {
  String tmp;
  StaticJsonBuffer<256> json;
  JsonObject &root = json.createObject();
  if (this->haveToDo) {
    if (this->SSID != WiFi.SSID() && this->isAvailable(this->SSID)) {
      DEBUG_VAR("CoolWifi: Connecting to new router", this->SSID);
      DEBUG_VAR("CoolWifi: Entry time to Wifi connection attempt:", millis());
      this->connect(this->SSID.c_str(), this->pass.c_str());
      DEBUG_VAR("CoolWifi: Connected to : ", WiFi.SSID());
      DEBUG_VAR("Exit time from Wifi connection attempt:", millis());
    } else {
      root["desired"] = this->SSID;
      root["currentSSID"] = WiFi.SSID();
      root["status"] = this->StringStatus(WL_NO_SSID_AVAIL);
      root.printTo(tmp);
      events.send(tmp.c_str(), NULL, millis(), 1000);
      return false;
    }
    this->haveToDo = false;
    INFO_VAR("CoolWifi: wifi status", WiFi.status());
    root["desired"] = this->SSID;
    root["currentSSID"] = WiFi.SSID();
    root["status"] = this->StringStatus(WiFi.status());
    root.printTo(tmp);
    events.send(tmp.c_str(), NULL, millis(), 1000);
    this->SSID = "";
    return true;
  }
  return false;
}

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
      root.createNestedObject(WiFi.SSID(i));
      JsonObject &obj = root[WiFi.SSID(i)];
      obj["bssid"] = WiFi.BSSIDstr(i);
      obj["rssi"] = WiFi.RSSI(i);
      obj["channel"] = String(WiFi.channel(i));
      obj["secure"] = String(WiFi.encryptionType(i));
      obj["hidden"] = String(WiFi.isHidden(i) ? "true" : "false");
    }
  }
  WiFi.scanDelete();
  String tmp;
  root.printTo(tmp);
  DEBUG_VAR("CoolWifi::jsonStringWiFiScan: ", tmp);
  return (tmp);
}

bool CoolWifi::isAvailable(String ssid) {
  // enhancement: need to verify also the BSSID
  DynamicJsonBuffer json;
  JsonObject &root = json.parseObject(this->jsonStringWiFiScan());
  if (root[ssid].success()) {
    return (true);
  }
  return (false);
}

bool CoolWifi::autoConnect() {
  DynamicJsonBuffer json;
  JsonObject &scan = json.parseObject(this->jsonStringWiFiScan());
  DynamicJsonBuffer config;
  File f = SPIFFS.open("/wifiConfig.json", "r");
  JsonObject &conf = config.parseObject(f.readString());
  uint8_t wifiCount = conf.get<uint8_t>("wifiCount");
  int16_t rssi[wifiCount];
  if (wifiCount == 0) {
    return false;
  }
  for (uint8_t i = 0; i < wifiCount; ++i) {
    rssi[i] = -100;
    if (scan[conf["Wifi" + String(i)]["ssid"].asString()].success()) {
      DEBUG_VAR("Saved network found on scan:",
                String(conf["Wifi" + String(i)]["ssid"].asString()));
      String obj = scan[conf["Wifi" + String(i)]["ssid"].asString()];
      JsonObject &network = json.parseObject(obj);
      int16_t pwr = network.get<int16_t>("rssi");
      DEBUG_VAR("Signal Power:", pwr);
      rssi[i] = pwr;
    }
  }
  uint8_t n = this->getIndexOfMaximumValue(rssi, wifiCount);
  DEBUG_VAR("CoolWifi::autoConnect index Connecting to: ", n);
  DEBUG_VAR("CoolWifi::autoConnect Connecting to: ",
            String(conf["Wifi" + String(n)]["ssid"].asString()));
  DEBUG_VAR("CoolWifi: Entry time to Wifi connection attempt:", millis());
  if (this->connect(conf["Wifi" + String(n)]["ssid"].asString(),
                    conf["Wifi" + String(n)]["pass"].asString()) !=
      WL_CONNECTED) {
    WARN_VAR("There's a problem, reconnecting, WiFiStatus: ", WiFi.status());
  }
  DEBUG_VAR("CoolWifi::autoConnect Connected to: ", WiFi.SSID());
  DEBUG_VAR("Exit time from Wifi connection attempt:", millis());
  return (true);
}

uint8_t CoolWifi::connect(String ssid, String pass) {
  ETS_UART_INTR_DISABLE();
  wifi_station_disconnect();
  ETS_UART_INTR_ENABLE();
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
    wifi_station_disconnect();
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

uint8_t CoolWifi::getWifiCount() {
  DynamicJsonBuffer config;
  File f = SPIFFS.open("/wifiConfig.json", "r");
  JsonObject &conf = config.parseObject(f.readString());
  return (conf.get<uint8_t>("wifiCount"));
}
int CoolWifi::getIndexOfMaximumValue(int16_t *array, int size) {
  int16_t maxIndex = 0;
  int16_t max = array[maxIndex];
  for (int i = 0; i < size; i++) {
    if (max < array[i]) {
      max = array[i];
      maxIndex = i;
    }
  }
  if (array[maxIndex] < WIFI_POOR_SIGNAL_THREESHOLD) {
    DEBUG_LOG("Best signal to low, enhance WiFi power");
    WiFi.setOutputPower(WIFI_MAX_POWER);
  }
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
}
