#include "CoolAsyncEditor.h"
#include "ArduinoJson.h"
#include "CoolLog.h"
#include <FS.h>
#include <Stream.h>
#include <avr/pgmspace.h>
#include <user_interface.h>

CoolAsyncEditor::CoolAsyncEditor(const fs::FS &fs) : _fs(fs) {}

CoolAsyncEditor &CoolAsyncEditor::getInstance() {
  static CoolAsyncEditor instance;
  return instance;
}

bool CoolAsyncEditor::write(String patch, String data) {
  File tmp = _fs.open(patch, "w");
  if (!tmp)
    return false;
  DEBUG_VAR("Writing on spiffs: ", data);
  tmp.write((const uint8_t *)data.c_str(), data.length());
  tmp.close();
  return true;
}

File CoolAsyncEditor::loadFile(String file) {
  File tmp = _fs.open(file, "r");
  return (tmp);
}

uint32_t CoolAsyncEditor::size(String file) {
  File tmp = _fs.open(file, "r");
  uint32_t size = tmp.size();
  tmp.close();
  return (size);
}

bool CoolAsyncEditor::remove(String patch) { return (_fs.remove(patch)); }

void CoolAsyncEditor::reWriteWifi(String data) {
  DEBUG_VAR("New wifiConfig: ", data);
  this->write("/wifiConfig.json", data);
  INFO_LOG("wifi reset!");
}

bool CoolAsyncEditor::addNewWifi(String bssid, String ssid, String pass) {
  DynamicJsonBuffer json;
  JsonArray &jsonBuf = json.parseArray(this->read("/wifiConfig.json").c_str());
  JsonObject &nested = jsonBuf.createNestedObject();
  nested["bssid"] = bssid;
  nested["ssid"] = ssid;
  nested["psk"] = pass;
  String tmp;
  jsonBuf.printTo(tmp);
  this->write("/wifiConfig.json", tmp);
  return true;
}

String CoolAsyncEditor::getSavedWifi(String bssid) {
  DynamicJsonBuffer json;
  JsonArray &jsonBuf = json.parseArray(this->read("/wifiConfig.json").c_str());
  uint8_t wifiSize = jsonBuf.size();
  for (uint8_t i = 0; i < wifiSize; ++i) {
    String tmp = jsonBuf.get<String>(i);
    JsonObject &obj = json.parseObject(tmp.c_str());
    if (obj.get<String>("bssid") == bssid) {
      DEBUG_VAR("Saved Wifi Json obj requested: ", tmp);
      return (tmp);
    }
  }
  return ("{}");
}

bool CoolAsyncEditor::removeSavedWifi(String bssid) {
  DynamicJsonBuffer json;
  JsonArray &jsonBuf = json.parseArray(this->read("/wifiConfig.json").c_str());
  uint8_t wifiSize = jsonBuf.size();
  for (uint8_t i = 0; i < wifiSize; ++i) {
    DynamicJsonBuffer tmpBuffer;
    String tmpObj = jsonBuf.get<String>(i);
    JsonObject &obj = tmpBuffer.parseObject(tmpObj.c_str());
    if (obj.get<String>("bssid") == bssid) {
      DEBUG_VAR("Removing WiFi: ", obj[bssid].asString());
      jsonBuf.remove(i);
      String tmp;
      jsonBuf.printTo(tmp);
      this->write("/wifiConfig.json", tmp);
      return (true);
    }
  }
  return (false);
}

bool CoolAsyncEditor::removeAllWifi() {
  DynamicJsonBuffer json;
  JsonArray &jsonBuf = json.parseArray(this->read("/wifiConfig.json").c_str());
  INFO_LOG("Removing All Saved WiFi! ");
  uint8_t wifiSize = jsonBuf.size();
  for (uint8_t i = 0; i < wifiSize; ++i) {
    jsonBuf.remove(i);
  }
  String tmp;
  jsonBuf.printTo(tmp);
  this->write("/wifiConfig.json", tmp);
  return (true);
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
  String friendlyName = "CoolBoard-" + this->getMAC();
  String ip = WiFi.localIP().toString();
  String xml = this->read("/description.xml");
  int outputLength = xml.length() + ip.length() + friendlyName.length() +
                     (this->getMAC().length() * 2) + this->getUUID().length() +
                     1;
  char buffer[outputLength];
  buffer[outputLength - 1] = (char)NULL;
  sprintf(buffer, xml.c_str(), ip.c_str(), friendlyName.c_str(),
          this->getMAC().c_str(), this->getMAC().c_str(),
          this->getUUID().c_str());
  DEBUG_VAR("UPnP descriptor: ", buffer);
  return String(buffer);
}

String CoolAsyncEditor::getFlashID() {
  return (String(ESP.getFlashChipId(), HEX));
}

String CoolAsyncEditor::getMAC() {
  String tempMAC = WiFi.macAddress();
  tempMAC.replace(":", "");
  return (tempMAC);
}

String CoolAsyncEditor::getUUID() {
  uint32_t chipId = ESP.getChipId();
  char uuid[SSDP_UUID_SIZE];
  sprintf(uuid, "38323636-4558-4dda-9188-cda0e6%02x%02x%02x",
          (uint16_t)((chipId >> 16) & 0xff), (uint16_t)((chipId >> 8) & 0xff),
          (uint16_t)chipId & 0xff);
  return (uuid);
}

String CoolAsyncEditor::getSavedCredentialType(String bssid, String type) {
  DynamicJsonBuffer json;
  JsonArray &jsonBuf = json.parseArray(this->read("/wifiConfig.json"));
  uint8_t wifiSize = jsonBuf.size();
  for (uint8_t i = 0; i < wifiSize; ++i) {
    if (jsonBuf[i][bssid].success()) {
      if (type == "psk") {
        DEBUG_VAR("Credential required: ", jsonBuf[i]["psk"].asString());
        return (jsonBuf[i]["psk"].asString());
      } else if (type == "ssid") {
        DEBUG_VAR("Credential required: ", jsonBuf[i]["psk"].asString());
        return (jsonBuf[i]["ssid"].asString());
      }
    }
  }
  return ("");
}

bool CoolAsyncEditor::beginAdminCredential() {
  DynamicJsonBuffer json;
  JsonObject &jsonBuf =
      json.parseObject(this->read("/webServerCredentials.json").c_str());
  if (jsonBuf["username"].success() && jsonBuf["password"].success()) {
    HTTPuserName = jsonBuf.get<String>("username");
    HTTPpassword = jsonBuf.get<String>("password");
    return true;
  } else {
    ERROR_LOG("Impossible to get HTTP credential, need to reset.");
    this->resetAdminCredential();
    return false;
  }
}

bool CoolAsyncEditor::configureAdminCredential(String userName,
                                               String password) {
  DynamicJsonBuffer json;
  JsonObject &root = json.createObject();
  root["username"] = userName;
  root["password"] = password;
  String tmp;
  root.printTo(tmp);
  this->write("/webServerCredentials.json", tmp);
  DEBUG_LOG("New Administrator Credential Saved!");
  DEBUG_VAR("user: ", userName);
  DEBUG_VAR("password: ", password);
  return true;
}

bool CoolAsyncEditor::resetAdminCredential() {
  DynamicJsonBuffer json;
  JsonObject &root = json.createObject();
  root["username"] = "admin";
  root["password"] = "admin";
  String tmp;
  root.printTo(tmp);
  this->write("/webServerCredentials.json", tmp);
  WARN_LOG("HTTP credential reset done.");
}