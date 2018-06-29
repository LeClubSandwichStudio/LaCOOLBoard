#include "CoolAsyncEditor.h"
#include "ArduinoJson.h"
#include "CoolLog.h"
#include <FS.h>

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
  StaticJsonBuffer<256> json;
  JsonObject &root = json.parseObject(this->read("/wifiConfig.json"));
  uint8_t wifiCount = root.get<uint8_t>("wifiCount");
  DEBUG_VAR("Number of recorded networks: ", wifiCount);
  if (root["wifiCount"].success()) {
    wifiCount++;
    JsonObject &newWifi = root.createNestedObject("Wifi" + String(wifiCount));
    newWifi["ssid"] = ssid;
    newWifi["pass"] = pass;
    String tmp;
    root.printTo(tmp);
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
      } while ((lastChar >= 0) && (idx < file.size()));
      linebuf[idx - 1] = '\0';
    }
  }
  String buf = String((char *)linebuf);
  DEBUG_VAR("Async read From SPIFFS: ", buf);
  file.close();
  return (buf);
}
