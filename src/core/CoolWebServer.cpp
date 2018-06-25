#include <CoolLog.h>
#include <CoolWebServer.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

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

bool CoolWebServer::begin(const char *currentSSID, const char *currentPASS) {
  DEBUG_LOG("AsyncWebServer begin");
  this->isRunnig = true;
  String tempMAC = WiFi.macAddress();
  tempMAC.replace(":", "");
  String name = "CoolBoard-" + tempMAC;
  WiFi.hostname(name.c_str());
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(name.c_str());
  this->doWithSta(currentPASS, currentSSID);
  MDNS.addService("http", "tcp", 80);
  if (!SPIFFS.begin()) {
    return (false);
  }
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  events.onConnect([](AsyncEventSourceClient *client) {
    client->send("hello!", NULL, millis(), 1000);
  });
  server.addHandler(&events);
  server.addHandler(new SPIFFSEditor(http_username, http_password));
  this->requestConfiguration();
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");
  server.begin();
  INFO_VAR("CoolBoard WebServer started! with SSID: ", name);
  return (true);
}

void CoolWebServer::end() {
  if (this->isRunnig) {
    events.close();
    ws.closeAll();
    WiFi.mode(WIFI_STA);
    this->isRunnig = false;
    DEBUG_LOG("AsyncWebServer disconnected!");
  } else {
    DEBUG_LOG("AsyncWebServer is already closed!");
  }
}

void CoolWebServer::requestConfiguration() {
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
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
}

void CoolWebServer::doWithSta(const char *ssid, const char *pass) {
  if (ssid != NULL) {
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      WARN_LOG("STA: Failed!\n");
      WiFi.disconnect(false);
      delay(100);
      WiFi.begin(ssid, pass);
    }
  }
}