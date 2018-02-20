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

#include <Arduino.h>
#include <FS.h>

#include "CoolFileSystem.h"
#include "CoolLog.h"

/**
 *  CoolFileSystem::begin():
 *  This method is provided to start the
 *  SPIFFS object.
 *
 *  \return true if SPIFFS was initialized correctly,
 *  false otherwise
 */
bool CoolFileSystem::begin() {
  DEBUG_LOG("Entering CoolFileSystem.begin()");
  bool status = SPIFFS.begin();
  DEBUG_VAR("SPIFFS status:", status);
  this->printSavedDataCount();
  return (status);
}

/**
 *  CoolFileSystem::saveSensorData( data ):
 *  This method is provided to save the data on the local
 *  memory when there is no internet available
 *
 *  increments the saved data flag  when successful
 *
 *  \return true if the data was saved,
 *  false otherwise
 */
bool CoolFileSystem::saveSensorData(const char *data) {
  DEBUG_LOG("Entering CoolFileSystem.saveSensorData()");

  File sensorsData = SPIFFS.open("/sensorsData.json", "a");

  if (!sensorsData) {
    ERROR_LOG("Failed to open /sensorsData.json for appending");
    this->savedData = savedData;
    return (false);
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(data);

  if (root.success()) {
    DEBUG_JSON("Sensors data JSON", root);
    DEBUG_VAR("JSON buffer size:", jsonBuffer.size());

    root.printTo(sensorsData);
    sensorsData.close();

    this->savedData++;
    this->incrementSavedDataCount();
    return (true);
  } else {
    ERROR_LOG("Failed to parse sensor data JSON");
    return (false);
  }
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
  DEBUG_LOG("Entering CoolFileSystem.saveSensorDataCSV()");

  // parsing json
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(data);
  String header = "", values = "";

  // if json parse success
  if (!root.success()) {
    ERROR_LOG("Failed to parse sensor data as json");
    return (false);
  }
  for (auto kv : root) {
    DEBUG_VAR("Sensor data JSON key:", kv.key);
    DEBUG_VAR("Sensor data JSON value:", kv.value.as<char *>());

    header += kv.key;
    header += ',';

    // print the values to header string
    values += (kv.value.as<char *>());
    values += ',';
  }

  header.remove(header.lastIndexOf(','), 1);
  values.remove(values.lastIndexOf(','), 1);
  DEBUG_JSON("Sensors data JSON:", root);
  DEBUG_VAR("JSON buffer size:", jsonBuffer.size());
  DEBUG_VAR("CSV Header:", header);
  DEBUG_VAR("CSV values:", values);

  // check if file exists
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
  DEBUG_LOG("Entering CoolFileSystem.updateConfigFiles");

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(answer.c_str());

  if (!(root.success())) {
    ERROR_LOG("Failed to parse configuration update message");
    return (false);
  }
  DEBUG_JSON("Configuration update JSON:", root);
  DEBUG_VAR("JSON buffer size:", jsonBuffer.size());

  String temp;

  JsonObject &jsonCoolBoard = root["CoolBoard"];
  if (jsonCoolBoard.success()) {
    this->fileUpdate(jsonCoolBoard, "/coolBoardConfig.json");
  }

  JsonObject &jsonSensorsBoard = root["CoolSensorsBoard"];
  if (jsonSensorsBoard.success()) {
    this->fileUpdate(jsonSensorsBoard, "/coolBoardSensorsConfig.json");
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
    this->fileUpdate(jsonJetpack, "/irene3000Config.json");
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
  return true;
}

/**
 *  CoolFileSystem::isDataSaved():
 *  This method is provided to report
 *  wether there is sensor data saved in the
 *  File System.
 *
 *  \return true if there is data saved, false
 *  otherwise
 */
int CoolFileSystem::isDataSaved() {
  DEBUG_LOG("Entering CoolFileSystem.isDataSaved()");

  FSInfo fsInfo;

  if (SPIFFS.info(fsInfo) == true) {
    DEBUG_VAR("FS used bytes:", fsInfo.usedBytes);
    DEBUG_VAR("FS total bytes:", fsInfo.totalBytes);
  }

  File sensorsData = SPIFFS.open("/sensorsData.json", "r");
  File sensorsDataCSV = SPIFFS.open("/sensorsData.csv", "r");

  if ((!sensorsData) || (!sensorsDataCSV)) {
    ERROR_LOG("Failed to open sensors data files");
  } else {
    DEBUG_VAR("Sensors data JSON file size (B):", sensorsData.size());
    DEBUG_VAR("Sensors data CSV file size (B):", sensorsDataCSV.size());
  }
  DEBUG_VAR("Number of saved sensor data lines:", this->savedData);
  return (this->savedData);
}

/**
 *  CoolFileSystem::getSensorData(int &lines):
 *  This method is provided to return the
 *  sensor data saved in the File System
 *  50 lines at a time
 *  \return String array containing
 *  50 first lines from the file
 *  modifies tge lines argument to reflect the number of
 *  lines left
 */
String *CoolFileSystem::getSensorSavedData(int &lines) {
  DEBUG_LOG("Entering CoolFileSystem.getSensorSavedData()");
  int maxString = 50;
  String *sensorsDataArray = new String[maxString];
  lines = 0;

  // open sensors data file
  File sensorsData = SPIFFS.open("/sensorsData.json", "r");

  if (!sensorsData) {
    // FIXME: don't return a "magic" line, find another error handling method
    DEBUG_LOG("Failed to read /sensorsData.json");
    sensorsDataArray[0] = "Failed to read /sensorsData.json";
    lines++;
    return (sensorsDataArray);
  } else {
    String temp;
    while (sensorsData.available()) {
      yield();
      temp = sensorsData.readStringUntil('\r');

      if (linesToSkip > 0) {
        linesToSkip--;
      } else {
        sensorsDataArray[lines] = temp;
        sensorsData.read();
        DEBUG_VAR("Read line #", lines);
        DEBUG_VAR("Value:", sensorsDataArray[lines]);
        lines++;

        if (lines >= maxString) {
          break;
        }
        yield();
      }
    }

    sensorsData.close();

    this->savedData = savedData - lines;
    this->linesToSkip = lines;

    // delete data in the file only if savedData<=0
    if (this->savedData <= 0) {
      File sensorsData = SPIFFS.open("/sensorsData.json", "w");
      File sensorsDataCSV = SPIFFS.open("/sensorsData.csv", "w");
      if ((!sensorsData) || (!sensorsDataCSV)) {
        // FIXME: don't return a "magic" line, find another error handling
        // method
        ERROR_LOG("Failed to delete data in the file");
        lines++;
        sensorsDataArray[lines] = "Failed to delete data in the file";
        return (sensorsDataArray);
      }
      sensorsData.close();
      sensorsDataCSV.close();
      this->savedData = 0;
      this->linesToSkip = 0;
    }

    // save the changes to linesToSkip and savedData in the file system
    this->incrementSavedDataCount();
    return (sensorsDataArray);
  }
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
  DEBUG_LOG("Entering CoolFileSystem.fileUpdate()");
  DEBUG_VAR("Updating config file:", path);

  // open file in read mode
  File configFile = SPIFFS.open(path, "r");

  if (!configFile) {
    ERROR_VAR("Failed to open file for reading:", path);
    return (false);
  }

  // copy file to a json
  size_t size = configFile.size();

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer fileBuffer;
  JsonObject &fileJson = fileBuffer.parseObject(buf.get());

  if (!fileJson.success()) {
    ERROR_VAR("Failed to parse update JSON for config file:", path);
    return (false);
  }

  for (auto kv : fileJson) {
    if (updateJson[kv.key].success()) {
      fileJson[kv.key] = updateJson[kv.key];
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
 *  CoolFileSystem::incrementSavedDataCount():
 *  This method is provided to increment the
 *  savedData flag
 *
 *  \return true if successful , false otherwise
 *
 */
bool CoolFileSystem::incrementSavedDataCount() {
  DEBUG_LOG("Entering CoolFileSystem.incrementSavedDataCount()");

  File file = SPIFFS.open("/savedDataFlag.txt", "w");
  if (!file) {
    ERROR_LOG("Failed to open savedDataFlag.txt");
    return (false);
  } else {
    file.print(this->savedData);
    file.print(" ");
    file.println(this->linesToSkip);
    file.close();
    DEBUG_VAR("Number of lines to read:", this->savedData);
    DEBUG_VAR("Number of lines to skip:", this->linesToSkip);
    return (true);
  }
}

/**
 *  CoolFileSystem::printSavedDataCount():
 *  This method is provided to get the
 *  savedData flag from the file system
 *
 */
void CoolFileSystem::printSavedDataCount() {
  DEBUG_LOG("Entering CoolFileSystem.printSavedDataCount()");

  File file = SPIFFS.open("/savedDataFlag.txt", "r");
  if (!file) {
    ERROR_LOG("Failed to read savedDataFlag.txt");
  } else {
    String temp = file.readStringUntil(' ');

    this->savedData = temp.toInt();
    temp = file.readStringUntil('\n');
    this->linesToSkip = temp.toInt();
    file.close();
  }
  INFO_VAR("Number of lines to read:", this->savedData);
  INFO_VAR("Number of lines to skip:", this->linesToSkip);
}