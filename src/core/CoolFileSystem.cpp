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

/**
 *  CoolFileSystem::begin():
 *  This method is provided to start the
 *  SPIFFS object.
 *
 *  \return true if SPIFFS was initialized correctly,
 *  false otherwise
 */
bool CoolFileSystem::begin() {
  bool status = SPIFFS.begin();
  INFO_VAR("SPIFFS starting, status:", status);
  return status;
}

/**
 *  CoolFileSystem::saveSensorDataCSV( data ):
 *  This method is provided to save the data on the local
 *  memory in CSV format.
 *
 *  \return true if the data was saved,
 *  false otherwise
 */
bool CoolFileSystem::saveSensorDataCSV(const char *data) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(data);
  String header = "";
  String values = "";

  if (!root.success()) {
    ERROR_LOG("Failed to parse sensor data as json");
    return (false);
  }
  for (auto kv : root) {
  DEBUG_VAR("Sensor data JSON key:", kv.key);
  DEBUG_VAR("Sensor data JSON value:", kv.value.as<char *>());
    header += kv.key;
    header += ',';
    values += (kv.value.as<char *>());
    values += ',';
  }

  header.remove(header.lastIndexOf(','), 1);
  values.remove(values.lastIndexOf(','), 1);

  DEBUG_JSON("Sensors data JSON:", root);
  DEBUG_VAR("CSV header:", header);
  DEBUG_VAR("CSV values:", values);

  File sensorsData = SPIFFS.open("/sensorsData.csv", "r");

  // FIXME: ouch! this is probably leaking a file handler.
  if ((!sensorsData) || (sensorsData.size() == 0)) {
    DEBUG_LOG("/sensorsData.csv not found, creating file");
    sensorsData = SPIFFS.open("/sensorsData.csv", "w");

    if (!sensorsData) {
      ERROR_LOG("Failed to create /sensorsData.csv");
      return (false);
    }
    sensorsData.println(header);
    sensorsData.println(values);
    sensorsData.close();
    return (true);
  } else {
    DEBUG_LOG("/sensorsData.csv found");
    DEBUG_LOG("Appending to /sensorsData.csv");
    sensorsData = SPIFFS.open("/sensorsData.csv", "a");

    if (!sensorsData) {
      ERROR_LOG("Failed to open /sensorsData.csv for appending");
      return (false);
    }
    sensorsData.println(values);
    sensorsData.close();
    return (true);
  }
}

/**
 *  CoolFileSystem::updateConfigFiles( mqtt answer ):
 *  This method is provided to update the configuration files when
 *  the appropriate mqtt answer is received
 *
 *  \return true if the files are updated correctly,
 *  false otherwise
 */
bool CoolFileSystem::updateConfigFiles(String answer) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(answer.c_str());

  if (!(root.success())) {
    ERROR_LOG("Failed to parse configuration update message");
    return (false);
  }
  DEBUG_JSON("Configuration update JSON:", root);

  JsonObject &jsonCoolBoard = root["CoolBoard"];
  if (jsonCoolBoard.success()) {
    this->fileUpdate(jsonCoolBoard, "/coolBoardConfig.json");
  } else {
    ERROR_LOG("Failed to parse COOL Board configuration update");
  }

  JsonObject &jsonSensors = root["CoolSensorsBoard"];
  if (jsonSensors.success()) {
    this->fileUpdate(jsonSensors, "/coolBoardSensorsConfig.json");
  } else {
    ERROR_LOG("Failed to parse onboard sensors configuration update");
  }

  JsonObject &jsonCoolBoardActor = root["CoolBoardActor"];
  if (jsonCoolBoardActor.success()) {
    this->fileUpdate(jsonCoolBoardActor, "/coolBoardActorConfig.json");
  } else {
    ERROR_LOG("Failed to parse onboard actuator configuration update");
  }

  JsonObject &jsonRTC = root["rtc"];
  if (jsonRTC.success()) {
    this->fileUpdate(jsonRTC, "/rtcConfig.json");
  } else {
    ERROR_LOG("Failed to parse clock configuration update");
  }

  JsonObject &jsonLedBoard = root["led"];
  if (jsonLedBoard.success()) {
    this->fileUpdate(jsonLedBoard, "/coolBoardLedConfig.json");
  } else {
    ERROR_LOG("Failed to parse LED configuration update");
  }

  JsonObject &jsonJetpack = root["jetPack"];
  if (jsonJetpack.success()) {
    this->fileUpdate(jsonJetpack, "/jetPackConfig.json");
  } else {
    ERROR_LOG("Failed to parse Jetpack configuration update");
  }

  JsonObject &jsonIrene = root["irene3000"];
  if (jsonIrene.success()) {
    this->fileUpdate(jsonIrene, "/irene3000Config.json");
  } else {
    ERROR_LOG("Failed to parse IRN3000 configuration update");
  }

  JsonObject &jsonExternalSensors = root["externalSensors"];
  if (jsonExternalSensors.success()) {
    this->fileUpdate(jsonExternalSensors, "/externalSensorsConfig.json");
  } else {
    ERROR_LOG("Failed to parse external sensors configuration update");
  }

  JsonObject &jsonMQTT = root["mqtt"];
  if (jsonMQTT.success()) {
    this->fileUpdate(jsonMQTT, "/mqttConfig.json");
  } else {
    ERROR_LOG("Failed to parse MQTT configuration update");
  }

  JsonObject &jsonWifi = root["wifi"];
  if (jsonWifi.success()) {
    this->fileUpdate(jsonWifi, "/wifiConfig.json");
  } else {
    ERROR_LOG("Failed to parse Wifi configuration update");
  }

  return true;
}

/**
 *  CoolFileSystem::fileUpdate( update msg, file path):
 *  This method is provided to ensure the
 *  correct update for each configuration file in the
 *  File system
 *
 *  \return true if successful , false otherwise
 *
 */
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

/**
 *  CoolFileSystem::saveMessageToFile(():
 *  This method is provided to get the
 *  savedData flag from the file system
 *
 */
bool CoolFileSystem::saveMessageToFile(const char *data) {
  String lastName = "0";
  FSInfo fs_info;

  if (SPIFFS.info(fs_info) == true) {
    DEBUG_LOG("SPIFFS status before saving:");
    DEBUG_VAR("SPIFFS used bytes", fs_info.usedBytes);
    DEBUG_VAR("SPIFFS total bytes", fs_info.totalBytes);
  }

  int nextLog = lastFileSaved();
  nextLog++;
  char nextName[32];
  snprintf(nextName, 32, "/log/%ld.json", nextLog);

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

/**
 *  CoolFileSystem::getsavedData():
 *  This method is provided to get the
 *  savedData flag from the file system
 *
 */
bool CoolFileSystem::isFileSaved() {
  Dir dir = SPIFFS.openDir("/log");
  if (dir.next()) {
    return true;
  } else {
    return false;
  }
}

/**
 *  CoolFileSystem::getsavedData():
 *  This method is provided to get the
 *  savedData flag from the file system
 *
 */
int CoolFileSystem::lastFileSaved() {
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

/**
 *  CoolFileSystem::getFileString(int):
 *  This method is provided to get the
 *  content of a saved log from SPIFFS
 *
 */
String CoolFileSystem::getFileString(int num) {
  char path[32];
  String data;
  snprintf(path, 32, "/log/%ld.json", num);
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

/**
 *  CoolFileSystem::deleteLogFile(int):
 *  This method is provided to erase a log
 *  from the memory. It returns true or false
 *  if ever there is a SPIFFS problem at this
 *  level
 *
 */
bool CoolFileSystem::deleteLogFile(int num) {
  char logName[32];
  snprintf(logName, 32, "/log/%ld.json", num);

  if (SPIFFS.remove(logName)){
    INFO_VAR("Deleted log file:", logName);
    return true;
  } else {
    ERROR_VAR("Failed to delete log file:", logName);
    return false;
  }
}