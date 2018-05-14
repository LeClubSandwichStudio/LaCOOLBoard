/**
 *  Copyright (c) 2018 La Cool Co SAS
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 *
 */

#include <FS.h>

#include "CoolFileSystem.h"
#include "CoolLog.h"

#define JSON_FILE_EXT_SIZE 5

bool CoolFileSystem::begin() {
  bool status = SPIFFS.begin();
  INFO_VAR("SPIFFS starting, status:", status);
  return status;
}

void CoolFileSystem::updateConfigFiles(JsonObject &root) {
  JsonObject &jsonCoolBoard = root["CoolBoard"];
  if (jsonCoolBoard.success()) {
    this->fileUpdate(jsonCoolBoard, "/coolBoardConfig.json");
  }

  JsonObject &jsonSensors = root["CoolSensorsBoard"];
  if (jsonSensors.success()) {
    this->fileUpdate(jsonSensors, "/coolBoardSensorsConfig.json");
  }

  JsonObject &jsonCoolBoardActor = root["CoolBoardActor"];
  if (jsonCoolBoardActor.success()) {
    this->fileUpdate(jsonCoolBoardActor, "/coolBoardActorConfig.json");
  }

  JsonObject &jsonRTC = root["rtc"];
  if (jsonRTC.success()) {
    this->fileUpdate(jsonRTC, "/rtcConfig.json");
  }

  JsonObject &jsonLedBoard = root["led"];
  if (jsonLedBoard.success()) {
    this->fileUpdate(jsonLedBoard, "/coolBoardLedConfig.json");
  }

  JsonObject &jsonJetpack = root["jetPack"];
  if (jsonJetpack.success()) {
    this->fileUpdate(jsonJetpack, "/jetPackConfig.json");
  }

  JsonObject &jsonIrene = root["irene3000"];
  if (jsonIrene.success()) {
    this->fileUpdate(jsonIrene, "/irene3000Config.json");
  }

  JsonObject &jsonExternalSensors = root["externalSensors"];
  if (jsonExternalSensors.success()) {
    this->fileUpdate(jsonExternalSensors, "/externalSensorsConfig.json");
  }

  JsonObject &jsonMQTT = root["mqtt"];
  if (jsonMQTT.success()) {
    this->fileUpdate(jsonMQTT, "/mqttConfig.json");
  }

  JsonObject &jsonWifi = root["wifi"];
  if (jsonWifi.success()) {
    this->fileUpdate(jsonWifi, "/wifiConfig.json");
  }
}

bool CoolFileSystem::fileUpdate(JsonObject &updateJson, const char *path) {
  DEBUG_VAR("Updating config file:", path);

  File configFile = SPIFFS.open(path, "r");

  if (!configFile) {
    ERROR_VAR("Failed to open file for reading:", path);
    return (false);
  }

  String data = configFile.readString();
  DynamicJsonBuffer fileBuffer;
  JsonObject &fileJson = fileBuffer.parseObject(data);

  if (!fileJson.success()) {
    ERROR_VAR("Failed to parse update JSON for config file:", path);
    return (false);
  }

  for (auto kv : fileJson) {
    if (updateJson[kv.key].success()) {
      fileJson[kv.key] = updateJson[kv.key];
    } else {
      fileJson[kv.key] = fileJson[kv.key];
    }
  }

  DEBUG_VAR("Preparing to update config file:", path);
  DEBUG_JSON("With new JSON:", fileJson);
  configFile.close();
  configFile = SPIFFS.open(path, "w");

  if (!configFile) {
    ERROR_VAR("Failed to open file for writing:", path);
    return (false);
  }
  fileJson.printTo(configFile);
  configFile.close();
  return (true);
}

bool CoolFileSystem::saveLogToFile(const char *data) {
  String lastName = "0";
  FSInfo fs_info;

  if (SPIFFS.info(fs_info) == true) {
    DEBUG_LOG("SPIFFS status before saving:");
    DEBUG_VAR("SPIFFS used bytes", fs_info.usedBytes);
    DEBUG_VAR("SPIFFS total bytes", fs_info.totalBytes);
  }

  int nextLog = lastSavedLogNumber();
  nextLog++;
  char nextName[32];
  snprintf(nextName, 32, "/log/%d.json", nextLog);

  DEBUG_VAR("Next saved data file name:", nextName);
  if (!SPIFFS.exists(nextName)) {
    File f = SPIFFS.open(nextName, "w");
    f.print(data);
    f.close();
    DEBUG_VAR("Data file saved as:", nextName);
  } else {
    DEBUG_VAR("Could not save data file (already exists):", nextName);
    return false;
  }
  if (SPIFFS.info(fs_info) == true) {
    DEBUG_LOG("SPIFFS status after saving:");
    DEBUG_VAR("SPIFFS used bytes", fs_info.usedBytes);
    DEBUG_VAR("SPIFFS total bytes", fs_info.totalBytes);
  }
  return true;
}

bool CoolFileSystem::hasSavedLogs() {
  Dir dir = SPIFFS.openDir("/log");
  if (dir.next()) {
    return true;
  } else {
    return false;
  }
}

int CoolFileSystem::lastSavedLogNumber() {
  int next = 0;
  int temp = 0;
  Dir dir = SPIFFS.openDir("/log");
  while (dir.next()) {
    temp = dir.fileName().substring(JSON_FILE_EXT_SIZE).toInt();
    DEBUG_VAR("Saved file data name: ", dir.fileName());
    if (temp >= next) {
      next = temp;
    }
  }
  return next;
}

String CoolFileSystem::getSavedLogAsString(int num) {
  char path[32];
  String data;
  snprintf(path, 32, "/log/%d.json", num);
  File f = SPIFFS.open(path, "r");

  if (!f) {
    ERROR_VAR("Failed to data file:", path);
    return data;
  }
  data = f.readString();
  DEBUG_VAR("Read data file:", path);
  DEBUG_VAR("Data file content:", data);
  f.close();
  yield();
  return data;
}

bool CoolFileSystem::deleteSavedLog(int num) {
  char logName[32];
  snprintf(logName, 32, "/log/%d.json", num);

  if (SPIFFS.remove(logName)){
    INFO_VAR("Deleted log file:", logName);
    return true;
  } else {
    ERROR_VAR("Failed to delete log file:", logName);
    return false;
  }
}