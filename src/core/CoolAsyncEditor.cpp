#include "CoolAsyncEditor.h"
#include "ArduinoJson.h"
#include "CoolLog.h"
#include <FS.h>
#include <avr/pgmspace.h>
#include <user_interface.h>

CoolAsyncEditor::CoolAsyncEditor(const fs::FS &fs) : _fs(fs) {}

void CoolAsyncEditor::write(String patch, String data) {
  File tmp = _fs.open(patch, "w");
  tmp.write((const uint8_t *)data.c_str(), data.length());
  tmp.close();
}

bool CoolAsyncEditor::remove(String patch) { return (_fs.remove(patch)); }

void CoolAsyncEditor::reWriteWifi(String data) {
  DEBUG_VAR("New wifiConfig: ", data);
  this->write("/wifiConfig.json", data);
  INFO_LOG("wifi reset!");
}

bool CoolAsyncEditor::addNewWifi(String ssid, String pass) {
  DynamicJsonBuffer json;
  JsonObject &jsonBuf =
      json.parseObject(this->read("/wifiConfig.json").c_str());
  uint8_t wifiCount = jsonBuf.get<uint8_t>("wifiCount");
  DEBUG_VAR("Number of recorded networks: ", wifiCount);
  if (jsonBuf["wifiCount"].success()) {
    wifiCount++;
    jsonBuf["wifiCount"] = wifiCount;
    JsonObject &newWifi =
        jsonBuf.createNestedObject("Wifi" + String(wifiCount));

    newWifi["ssid"] = ssid;
    newWifi["pass"] = pass;
    String tmp;
    jsonBuf.printTo(tmp);
    DEBUG_VAR("New wifi Configuration: ", tmp);
    this->write("/wifiConfig.json", tmp);
    return (true);
  }
}

String CoolAsyncEditor::read(String patch) {
  File file = _fs.open(patch, "r");
  uint8_t linebuf[file.size()];
  if (file.size() > 0) {
    uint32_t idx;
    while (file.available()) {
      linebuf[0] = '\0';
      idx = 0;
      uint8_t lastChar;
      do {
        lastChar = file.read();
        linebuf[idx++] = lastChar;
      } while ((lastChar >= 0) && (idx < (file.size() + 1)));
      linebuf[idx - 1] = '\0';
    }
  }
  String buf = String((char *)linebuf);
  DEBUG_VAR("Async read From SPIFFS: ", buf);
  file.close();
  return (buf);
}

char *CoolAsyncEditor::readChars(String patch) {
  File file = _fs.open(patch, "r");
  uint8_t linebuf[file.size()];
  if (file.size() > 0) {
    uint32_t idx;
    while (file.available()) {
      linebuf[0] = '\0';
      idx = 0;
      uint8_t lastChar;
      do {
        lastChar = file.read();
        linebuf[idx++] = lastChar;
      } while ((lastChar >= 0) && (idx < (file.size() + 1)));
      linebuf[idx - 1] = '\0';
    }
  }
  DEBUG_VAR("Async read From SPIFFS: ", (char *)linebuf);
  file.close();
  return ((char *)linebuf);
}

String CoolAsyncEditor::getSdpConfig() {
  IPAddress ip = WiFi.localIP();
  String xml = this->read("/description.xml");
  char buffer[sizeof xml];
  char ipBuffer[sizeof ip.toString()];
  ip.toString().toCharArray(ipBuffer, sizeof ip + 1);
  xml.toCharArray(buffer, sizeof xml + 1);
  sprintf(buffer, ipBuffer);
  DEBUG_VAR("UPnP descriptor: ", buffer);
  return (buffer);
}

void CoolAsyncEditor::configSSDP() {}
