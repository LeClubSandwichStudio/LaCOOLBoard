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

#ifdef ESP32
#include <SPIFFS.H>
#endif
#include "CoolConfig.h"
#include "CoolFileSystem.h"
#include "CoolLog.h"
#include <FS.h>

#define JSON_FILE_EXT_SIZE 5

static constexpr ConfigFile CONFIG_FILES[] = {
    {"CoolBoard", "/coolBoardConfig.json"},
    {"CoolSensorsBoard", "/coolBoardSensorsConfig.json"},
    {"CoolBoardActor", "/coolBoardActorConfig.json"},
    {"externalSensors", "/externalSensorsConfig.json"},
    {"led", "/coolBoardLedConfig.json"},
    {"jetPack", "/jetPackConfig.json"},
    {"irene3000", "/irene3000Config.json"},
    {"mqtt", "/mqttConfig.json"},
    {"wifi", "/wifiConfig.json"}};

static const uint8_t CONFIG_FILES_COUNT = 9;

void CoolFileSystem::updateConfigFiles(JsonObject &root) {
  for (uint8_t i = 0; i < CONFIG_FILES_COUNT; ++i) {
    JsonObject &json = root[CONFIG_FILES[i].code];
    if (json.success()) {
      CoolFileSystem::fileUpdate(json, CONFIG_FILES[i].path);
    }
  }
}

bool CoolFileSystem::fileUpdate(JsonObject &updateJson, const char *path) {
  CoolConfig config(path);

  DEBUG_VAR("Updating config file:", path);
  if (!config.readFileAsJson()) {
    ERROR_VAR("Failed to read configuration file for updating:", path);
    return (false);
  }
  JsonObject &fileJson = config.get();
  for (auto kv : updateJson) {
    fileJson[kv.key] = updateJson[kv.key];
  }
  DEBUG_VAR("Preparing to update config file:", path);
  DEBUG_JSON("With new JSON:", fileJson);
  if (!config.writeJsonToFile()) {
    ERROR_VAR("Failed to update configuration file:", path);
    return (false);
  }
  DEBUG_VAR("Successfully updated configuration file:", path);
  return (true);
}

bool CoolFileSystem::saveLogToFile(const char *data) {
  String lastName = "0";
#ifdef ESP8266
  FSInfo fs_info;
  if (SPIFFS.info(fs_info) == true) {
    DEBUG_LOG("SPIFFS status before saving:");
    DEBUG_VAR("SPIFFS used bytes", fs_info.usedBytes);
    DEBUG_VAR("SPIFFS total bytes", fs_info.totalBytes);
  }
#elif ESP32
  DEBUG_LOG("SPIFFS status before saving:");
  DEBUG_VAR("SPIFFS used bytes", SPIFFS.usedBytes());
  DEBUG_VAR("SPIFFS total bytes", SPIFFS.totalBytes());
#endif

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
#ifdef ESP8266
  if (SPIFFS.info(fs_info) == true) {
    DEBUG_LOG("SPIFFS status before saving:");
    DEBUG_VAR("SPIFFS used bytes", fs_info.usedBytes);
    DEBUG_VAR("SPIFFS total bytes", fs_info.totalBytes);
  }
#elif ESP32
  DEBUG_LOG("SPIFFS status before saving:");
  DEBUG_VAR("SPIFFS used bytes", SPIFFS.usedBytes());
  DEBUG_VAR("SPIFFS total bytes", SPIFFS.totalBytes());
#endif
  return true;
}

bool CoolFileSystem::hasSavedLogs() {
#ifdef ESP8266
  Dir dir = SPIFFS.openDir("/log");
  if (dir.next()) {
    return true;
  } else {
    return false;
  }
#elif ESP32
  File dir = SPIFFS.open("/log");
  if (dir.isDirectory()) {
    return dir.openNextFile();
  } else {
    return false;
  }
#endif
}

int CoolFileSystem::lastSavedLogNumber() {
  int next = 0;
  int temp = 0;
#ifdef ESP8266
  Dir dir = SPIFFS.openDir("/log");
  while (dir.next()) {
    temp = dir.fileName().substring(JSON_FILE_EXT_SIZE).toInt();
    DEBUG_VAR("Saved file data name: ", dir.fileName());
    if (temp >= next) {
      next = temp;
    }
  }
#elif ESP32
  File dir = SPIFFS.open("/log");
  if (dir.isDirectory()) {
    while (dir.openNextFile()) {
      temp = String(dir.name()).substring(JSON_FILE_EXT_SIZE).toInt();
      DEBUG_VAR("Saved file data name: ", dir.name());
      if (temp >= next) {
        next = temp;
      }
    }
  }
#endif
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

  if (SPIFFS.remove(logName)) {
    INFO_VAR("Deleted log file:", logName);
    return true;
  } else {
    ERROR_VAR("Failed to delete log file:", logName);
    return false;
  }
}