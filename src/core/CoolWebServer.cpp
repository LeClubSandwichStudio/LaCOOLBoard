#include "CoolAsyncEditor.h"
#include <CoolLog.h>
#include <CoolWebServer.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

String handleMessageReceived;

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if (type == WS_EVT_ERROR) {
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(),
                  *((uint16_t *)arg), (char *)data);
  } else if (type == WS_EVT_PONG) {
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len,
                  (len) ? (char *)data : "");
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len) {
      // the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(),
                    client->id(), (info->opcode == WS_TEXT) ? "text" : "binary",
                    info->len);

      if (info->opcode == WS_TEXT) {
        for (size_t i = 0; i < info->len; i++) {
          msg += (char)data[i];
        }
      } else {
        char buff[3];
        for (size_t i = 0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t)data[i]);
          msg += buff;
        }
      }
      Serial.printf("%s\n", msg.c_str());

      if (info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    } else {
      // message is comprised of multiple frames or the frame is split into
      // multiple packets
      if (info->index == 0) {
        if (info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(),
                        client->id(),
                        (info->message_opcode == WS_TEXT) ? "text" : "binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(),
                      client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(),
                    client->id(), info->num,
                    (info->message_opcode == WS_TEXT) ? "text" : "binary",
                    info->index, info->index + len);

      if (info->opcode == WS_TEXT) {
        for (size_t i = 0; i < info->len; i++) {
          msg += (char)data[i];
        }
      } else {
        char buff[3];
        for (size_t i = 0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t)data[i]);
          msg += buff;
        }
      }
      Serial.printf("%s\n", msg.c_str());

      if ((info->index + len) == info->len) {
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(),
                      client->id(), info->num, info->len);
        if (info->final) {
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(),
                        client->id(),
                        (info->message_opcode == WS_TEXT) ? "text" : "binary");
          if (info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

bool CoolWebServer::begin() {
  DEBUG_LOG("AsyncWebServer begin");
  this->isRunning = true;
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
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  events.onConnect([](AsyncEventSourceClient *client) {
    client->send("hello!", NULL, millis(), 1000);
  });
  server.addHandler(&events);
  this->requestConfiguration();
  // CoolAsyncEditor* coolAsyncEditor;
  // if(coolAsyncEditor->beginAdminCredential()){
  server.serveStatic("/", SPIFFS, "/")
      .setDefaultFile("index.htm")
      // .setAuthentication(coolAsyncEditor->HTTPuserName.c_str(),
      // coolAsyncEditor->HTTPpassword.c_str());
      .setAuthentication("admin", "admin");
  // } else {
  //       server.serveStatic("/", SPIFFS, "/")
  //       .setDefaultFile("index.htm");
  // }
  // delete coolAsyncEditor;
  this->onNotFoundConfig();
  server.begin();
  INFO_VAR("CoolBoard WebServer started! with SSID: ", name);
  return (true);
}

void CoolWebServer::end() {
  if (this->isRunning) {
    events.close();
    ws.closeAll();
    WiFi.mode(WIFI_STA);
    this->isRunning = false;
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
                DynamicJsonBuffer jsonBuffer;
                JsonObject &root =
                    jsonBuffer.parseObject(handleMessageReceived);
                if (root["ssid"].success() && root["pass"].success()) {
                  INFO_VAR("New SSID received:", root.get<String>("ssid"));
                  DEBUG_VAR("pass: :", root.get<String>("pass"));
                  CoolAsyncEditor coolAsyncEditor;
                  if (coolAsyncEditor.addNewWifi(root.get<String>("ssid"),
                                                 root.get<String>("pass"))) {
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
                CoolAsyncEditor coolAsyncEditor;
                DEBUG_VAR("handleMessageReceived: ", handleMessageReceived);
                coolAsyncEditor.reWriteWifi(handleMessageReceived);
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
    CoolAsyncEditor coolAsyncEditor;
    int params = request->params();
    if (params == 0) {
      request->send(200, "text/json", coolAsyncEditor.read("/wifiConfig.json"));
    }
    for (int i = 0; i < params; i++) {
      AsyncWebParameter *p = request->getParam(i);
      if (!p->isFile() && !p->isPost()) {
        Serial.printf("GET[%s]: WiFi:%s\n", p->name().c_str(),
                      p->value().c_str());
        CoolAsyncEditor coolAsyncEditor;
        if (coolAsyncEditor.getSavedWifi(p->value()) == "") {
          request->send(404);
        } else {
          request->send(200, "text/json",
                        coolAsyncEditor.getSavedWifi(p->value()));
        }
      }
    }
  });
  // FIX ME:
  server.on("/wifi/connect", HTTP_GET, [](AsyncWebServerRequest *request) {
    CoolAsyncEditor coolAsyncEditor;
    int params = request->params();
    for (int i = 0; i < params; i++) {
      AsyncWebParameter *p = request->getParam(i);
      if (!p->isFile() && !p->isPost()) {
        Serial.printf("GET[%s]: WiFi:%s\n", p->name().c_str(),
                      p->value().c_str());
        CoolAsyncEditor coolAsyncEditor;
        if (coolAsyncEditor.getSavedWifi(p->value()) == "") {
          request->send(404);
        } else {
          CoolAsyncWiFiAction::getInstance().SSID =
              coolAsyncEditor.getSavedCredentialFromIndex(
                  atoi(p->value().c_str()), "ssid");
          CoolAsyncWiFiAction::getInstance().pass =
              coolAsyncEditor.getSavedCredentialFromIndex(
                  atoi(p->value().c_str()), "pass");
          CoolAsyncWiFiAction::getInstance().haveToDo = true;
        }
      }
    }
    request->send(200);
  });

  server.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest *request) {
    CoolAsyncEditor coolAsyncEditor;
    String descriptor = coolAsyncEditor.getSdpConfig();
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
  String coolName = "coolboard-" + this->getCoolMac();
  SSDP.setDeviceType("upnp:rootdevice");
  INFO_VAR("Bonjour service started at: ", coolName);
  INFO_LOG("TCP server started");
  MDNS.addService("http", "tcp", 80);
  MDNS.begin(coolName.c_str());
}

CoolAsyncWiFiAction &CoolAsyncWiFiAction::getInstance() {
  static CoolAsyncWiFiAction instance;
  return instance;
}

bool CoolAsyncWiFiAction::manageConnectionPortal() {
  String tmp;
  DynamicJsonBuffer json;
  JsonObject &root = json.createObject();
  if (this->haveToDo) {
    if (this->SSID != WiFi.SSID() && this->isAvailable(this->SSID)) {
      DEBUG_VAR("CoolAsyncWiFiAction: Connecting to new router", this->SSID);
      DEBUG_VAR("CoolAsyncWiFiAction: Entry time to Wifi connection attempt:",
                millis());
      ETS_UART_INTR_DISABLE();
      wifi_station_disconnect();
      ETS_UART_INTR_ENABLE();
      WiFi.begin(this->SSID.c_str(), this->pass.c_str());
      while ((WiFi.status() != WL_CONNECTED)) {
        delay(10);
      }
      DEBUG_VAR("CoolAsyncWiFiAction: Connected to : ", WiFi.SSID());
      DEBUG_VAR("Exit time from Wifi connection attempt:", millis());
    } else {
      root["desired"] = this->SSID;
      root["currentSSID"] = WiFi.SSID();
      root["status"] = (uint8_t)WiFi.status();
      root.printTo(tmp);
      events.send(tmp.c_str(), NULL, millis(), 1000);
      return false;
    }
    this->haveToDo = false;
    INFO_VAR("CoolAsyncWiFiAction: wifi status", WiFi.status());
    root["desired"] = this->SSID;
    root["currentSSID"] = WiFi.SSID();
    root["status"] = (uint8_t)WiFi.status();
    root.printTo(tmp);
    events.send(tmp.c_str(), NULL, millis(), 1000);
    this->SSID = "";
    return true;
  }
  return false;
}

String CoolAsyncWiFiAction::jsonStringWiFiScan() {
  DynamicJsonBuffer json;
  JsonObject &root = json.createObject();
  uint8_t n = WiFi.scanNetworks();
  INFO_VAR("Scan status: ", n);
  if (n) {
    for (uint8_t i = 0; i < n; ++i) {
      root.createNestedObject(WiFi.SSID(i));
      JsonObject &obj = root[WiFi.SSID(i)];
      obj["BSSID"] = WiFi.BSSIDstr(i);
      obj["RSSI"] = WiFi.RSSI(i);
      obj["channel"] = String(WiFi.channel(i));
      obj["secure"] = String(WiFi.encryptionType(i));
      obj["hidden"] = String(WiFi.isHidden(i) ? "true" : "false");
    }
  }
  if (WiFi.scanComplete() >= 0) {
    WiFi.scanDelete();
  }
  String tmp;
  root.printTo(tmp);
  DEBUG_VAR("CoolAsyncWiFiAction::jsonStringWiFiScan: ", tmp);
  return (tmp);
}

bool CoolAsyncWiFiAction::isAvailable(String ssid) {
  // enhancement: need to verify also the BSSID
  DynamicJsonBuffer json;
  JsonObject &root = json.parseObject(this->jsonStringWiFiScan());
  if (root[ssid].success()) {
    return (true);
  }
  return (false);
}

// bool CoolAsyncWiFiAction::autoConnect(){
//   DynamicJsonBuffer json;
//   JsonObject &scan = json.parseObject(this->jsonStringWiFiScan());
// }

void CoolAsyncWiFiAction::setAPCallback(
    void (*func)(CoolAsyncWiFiAction *myWiFiManager)) {
  _apcallback = func;
}

void CoolAsyncWiFiAction::setSaveConfigCallback(void (*func)(void)) {
  _savecallback = func;
}
void CoolAsyncWiFiAction::setBreakAfterConfig(boolean shouldBreak) {
  _shouldBreakAfterConfig = shouldBreak;
}
