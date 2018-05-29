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

#include <ArduinoJson.h>
#include <FS.h>
#include <memory>

#include "CoolBoard.h"
#include "CoolConfig.h"
#include "CoolLog.h"

#define SEND_MSG_BATCH 10

void CoolBoard::begin() {
  this->config();
  this->led->write(50, 25, 0);
  delay(1000);
  this->startAP();
  this->wifiManager->config();
  if (!doOta) {
    pinMode(ENABLE_I2C_PIN, OUTPUT);
    pinMode(BOOTSTRAP_PIN, INPUT);
    digitalWrite(ENABLE_I2C_PIN, HIGH);
    delay(100);

    this->rtc->config();
    this->rtc->offGrid();
    delay(100);

    this->onBoardActuator->config();
    this->onBoardActuator->begin();
    delay(100);
    this->coolBoardSensors->config();
    this->coolBoardSensors->begin();
    delay(100);

    this->printConf();
    this->led->printConf();
    this->coolBoardSensors->printConf();
    this->onBoardActuator->printConf();
    this->rtc->printConf();

    if (this->jetpackActive) {
      this->jetPack->config();
      this->jetPack->begin();
      this->jetPack->printConf();
      delay(100);
    }

    if (this->ireneActive) {
      this->irene3000->config();
      this->irene3000->begin();
      this->irene3000->calibrate(*this->led);
      this->irene3000->printConf();
      delay(100);
    }

    if (this->externalSensorsActive) {
      this->externalSensors->config();
      this->externalSensors->begin();
      delay(100);
    }

    this->connect();
    this->sendPublicIP();
    delay(100);
    this->rtc->begin();

    if (!this->sleepActive) {
      sendAllConfig();
    }
  } else {
    this->otaUpdate();
  }
  delay(100);
}

void CoolBoard::loop() {
  INFO_LOG("Connecting...");
  if (!this->isConnected()) {
    this->connect();
    INFO_LOG("Sending public IP...");
    this->sendPublicIP();
  }
  INFO_LOG("Updating RTC...");
  this->rtc->update();

  INFO_LOG("Listening to saved messages...");
  this->mqttListen();


  if (CoolFileSystem::hasSavedLogs()) {
    INFO_LOG("Sending saved messages...");
    this->sendSavedMessages();
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  JsonObject &state = root.createNestedObject("state");
  JsonObject &reported = state.createNestedObject("reported");

  reported["Firmware Version"] = (String)COOL_FW_VERSION;

  INFO_LOG("Collecting board and sensor data...");
  this->readBoardData(reported);
  this->readSensors(reported);

  INFO_LOG("Setting actuators and reporting their state...");
  this->handleActuators(reported);

  delay(50);
  if (this->mqttClient->state() != 0) {
    WARN_LOG("Reconnecting MQTT...");
    if (this->mqttConnect() != 0) {
      this->mqttProblem();
    }
    delay(200);
  }

  // publish if we hit logInterval.
  if (this->shouldLog()) {
    INFO_LOG("Sending log over MQTT...");
    String data;
    root.printTo(data);
    if (!this->mqttPublish(data.c_str())) {
      CoolFileSystem::saveLogToFile(data.c_str());
      ERROR_LOG("MQTT publish failed! Data saved on SPIFFS");
      CoolFileSystem::saveLogToFile(data.c_str());
      this->mqttProblem();
    } else {
      this->messageSent();
    }
    this->previousLogTime = millis();
  }

  this->startAP();
  if (this->sleepActive) {
    this->sleep(this->secondsToNextLog());
  }
}

bool CoolBoard::isConnected() {
  if (this->wifiManager->state() != WL_CONNECTED) {
    WARN_LOG("Wifi disconnected");
    return false;
  }
  if (this->mqttClient->state() != 0) {
    WARN_LOG("MQTT disconnected");
    return false;
  }
  return true;
}

int CoolBoard::connect() {

  if (this->wifiManager->wifiCount > 0) {
    this->led->write(BLUE);
    if (this->wifiManager->connect() != 3) {
      this->led->blink(RED, 10);
    } else {
      this->led->blink(GREEN, 5);
    }
  } else {
    INFO_LOG("No configured Wifi access point, launching configuration portal");
    this->wifiManager->disconnect();
    delay(200);
    this->led->write(FUCHSIA);
    this->wifiManager->connectAP();
  }
  delay(100);

  if (this->wifiManager->state() == WL_CONNECTED) {
    delay(100);
    if (this->mqttConnect() != 0) {
      this->mqttProblem();
    }
    delay(100);
  }
  return (this->mqttClient->state());
}

String getSavedLogAsMessage(int logNumber) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  JsonObject &state = root.createNestedObject("state");
  state["reported"] =
      jsonBuffer.parseObject(CoolFileSystem::getSavedLogAsString(logNumber));

  String jsonData;
  root.printTo(jsonData);
  return jsonData;
}

void CoolBoard::sendSavedMessages() {
  for (int i = 0; i < SEND_MSG_BATCH; i++) {
    int savedLogNumber = CoolFileSystem::lastSavedLogNumber();
    INFO_VAR("Sending saved log number:", savedLogNumber);
    // only send a log if there is a log. 0 means zero files in the SPIFFS
    if (savedLogNumber != 0) {
      String jsonData = getSavedLogAsMessage(savedLogNumber);
      DEBUG_VAR("Saved JSON data to send:", jsonData);
      if (this->mqttPublish(jsonData)) {
        CoolFileSystem::deleteSavedLog(savedLogNumber);
        this->messageSent();
      } else {
        this->mqttProblem();
        break;
      }
    }
  }
}

void CoolBoard::handleActuators(JsonObject &reported) {
  if (this->manual == 0) {
    INFO_LOG("Actuators configuration: automatic");
    tmElements_t tm;
    tm = this->rtc->getTimeDate();

    if (this->jetpackActive) {
      DEBUG_LOG("Collecting Jetpack actuators data...");
      this->jetPack->doAction(reported, int(tm.Hour), int(tm.Minute));
    }
    DEBUG_LOG("Collecting onboard actuator data...");
    if (this->onBoardActuator->doAction(reported, int(tm.Hour),
                                        int(tm.Minute))) {
      this->onBoardActuator->write(1);
      reported["ActB"] = 1;
    } else {
      this->onBoardActuator->write(0);
      reported["ActB"] = 0;
    }
  } else {
    INFO_LOG("Actuators configuration: manual");
  }
}

bool CoolBoard::shouldLog() {
  unsigned long logIntervalMillis = this->logInterval * 1000;
  unsigned long millisSinceLastLog = millis() - this->previousLogTime;

  return (millisSinceLastLog >= logIntervalMillis ||
          this->previousLogTime == 0);
}

unsigned long CoolBoard::secondsToNextLog() {
  unsigned long seconds;

  seconds = this->logInterval - ((millis() - this->previousLogTime) / 1000);
  return (seconds > this->logInterval ? this->logInterval : seconds);
}

void CoolBoard::otaUpdateConfig() {
  File config = SPIFFS.open("/UpdateOTA.json", "r");
  if (config) {
    INFO_LOG("OTA JSON EXIST");
    String data = config.readString();
    DynamicJsonBuffer buffer;
    JsonObject &json = buffer.parseObject(data);
    if (json["fingerPrint"].success() && json["url"].success() &&
        json["version"].success()) {
      this->fwVersion = json.get<String>("version");
      this->fwURL = json.get<String>("url");
      this->fingerPrintURL = json.get<String>("fingerPrint");
    }
    INFO_VAR("OTA json found: ", data);
    if (this->fwURL.substring(0, 8) == "https://" &&
        this->fingerPrintURL.length() == 40 &&
        (String)COOL_FW_VERSION != this->fwVersion) {
      INFO_LOG("COOLBoard Start HTTPS OTA");
      this->doOta = true;
    } else if (this->fwURL.substring(0, 7) == "http://" &&
               this->fingerPrintURL.length() < 40 &&
               (String)COOL_FW_VERSION != this->fwVersion) {
      INFO_LOG("COOLBoard Start HTTP OTA");
      this->doOta = true;
    } else {
      this->doOta = false;
    }
    config.close();
    SPIFFS.remove("/UpdateOTA.json");
  } else {
    ERROR_LOG("Failed to read /UpdateOTA.json");
    this->doOta = false;
  }
}

bool CoolBoard::config() {
  INFO_VAR("MAC address is:", WiFi.macAddress());
  INFO_VAR("Firmware version is:", COOL_FW_VERSION);

  CoolFileSystem::begin();
  this->led->config();
  this->led->begin();

  delay(10);
  this->led->write(YELLOW);

  this->otaUpdateConfig();

  if (!this->doOta) {
    CoolConfig config("/coolBoardConfig.json");
    if (!config.readFileAsJson()) {
      ERROR_LOG("Failed to parse main configuration");
      this->spiffsProblem();
      return (false);
    }
    JsonObject &json = config.get();
    config.set<unsigned long>(json, "logInterval", this->logInterval);
    config.set<bool>(json, "ireneActive", this->ireneActive);
    config.set<bool>(json, "jetpackActive", this->jetpackActive);
    config.set<bool>(json, "externalSensorsActive",
                     this->externalSensorsActive);
    config.set<bool>(json, "sleepActive", this->sleepActive);
    config.set<bool>(json, "manual", this->manual);
    config.set<String>(json, "mqttServer", this->mqttServer);
    this->mqttClient->setClient(*this->wifiClient);
    this->mqttClient->setServer(this->mqttServer.c_str(), 1883);
    this->mqttClient->setCallback(
        [this](char *topic, byte *payload, unsigned int length) {
          this->mqttCallback(topic, payload, length);
        });
    this->mqttId = WiFi.macAddress();
    this->mqttId.replace(F(":"), F(""));
    this->mqttOutTopic =
        String(F("$aws/things/")) + this->mqttId + String(F("/shadow/update"));
    this->mqttInTopic = String(F("$aws/things/")) + this->mqttId +
                        String(F("/shadow/update/delta"));
    if (!config.writeJsonToFile()) {
      ERROR_LOG("Failed to save main configuration");
      this->spiffsProblem();
      return (false);
    }
    INFO_LOG("Main configuration loaded");
    return (true);
  } else {
    return true;
  }
}

void CoolBoard::printConf() {
  INFO_LOG("General configuration");
  INFO_VAR("  Log interval            =", this->logInterval);
  INFO_VAR("  Irene active            =", this->ireneActive);
  INFO_VAR("  Jetpack active          =", this->jetpackActive);
  INFO_VAR("  External sensors active =", this->externalSensorsActive);
  INFO_VAR("  Sleep active            =", this->sleepActive);
  INFO_VAR("  Manual active           =", this->manual);
  INFO_VAR("  MQTT server:            =", this->mqttServer);
}

void CoolBoard::update(const char *answer) {
  INFO_LOG("Received new MQTT message");

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(answer);
  JsonObject &stateDesired = root["state"];

  if (stateDesired.success()) {
    DEBUG_JSON("Desired state JSON:", stateDesired);
    if (stateDesired["CoolBoard"]["manual"].success()) {
      this->manual = stateDesired["CoolBoard"]["manual"].as<bool>();
      INFO_VAR("Manual flag received:", this->manual);
    }
    if (stateDesired["CoolBoard"]["firmwareUpdate"].success()) {
      String FW_obj = stateDesired["CoolBoard"]["firmwareUpdate"];
      JsonObject &FWroot = jsonBuffer.parseObject(FW_obj);
      this->fwVersion = FWroot.get<String>("version");

      if (!SPIFFS.exists("/UpdateOTA.json")) {
        INFO_LOG("Creating new OTA update configuration");
      }
      File receivedOTAConf = SPIFFS.open("/UpdateOTA.json", "w");
      if (!receivedOTAConf) {
        ERROR_LOG("failed to Create Ota Configuration File!");
      } else {
        INFO_VAR("Firmware version:", this->fwVersion);
        INFO_VAR("Firmware URL:", FWroot.get<String>("url"));
        INFO_VAR("URL fingerprint:", FWroot.get<String>("fingerPrint"));
        if ((String)COOL_FW_VERSION == this->fwVersion) {
          INFO_LOG("You are up to date!");
          stateDesired["CoolBoard"]["firmwareUpdate"] = NULL;
        } else {
          if (FWroot.get<String>("url").substring(0, 8) == "https://" &&
              FWroot.get<String>("fingerPrint").length() == 40) {
            INFO_LOG("HTTPS OTA object received:");
            this->fwVersion = FWroot.get<String>("version");
            this->fwURL = FWroot.get<String>("url");
            this->fingerPrintURL = FWroot.get<String>("fingerPrint");
            receivedOTAConf.print(FW_obj);
            INFO_VAR("Saved OTA HTTPS Update Configuration: ", FW_obj);
            receivedOTAConf.close();
            delay(100);
            stateDesired["CoolBoard"]["firmwareUpdate"] = NULL;
            this->doOta = true;
          }
          if (FWroot.get<String>("url").substring(0, 7) == "http://") {
            INFO_LOG("HTTP OTA Object received:");
            this->fwVersion = FWroot.get<String>("version");
            this->fwURL = FWroot.get<String>("url");
            this->fingerPrintURL = FWroot.get<String>("fingerPrint");
            receivedOTAConf.print(FW_obj);
            INFO_VAR("Saved OTA HTTP Update Configuration: ", FW_obj);
            stateDesired["CoolBoard"]["firmwareUpdate"] = NULL;
            receivedOTAConf.close();
            delay(100);
            this->doOta = true;
          }
        }
      }
    }

    this->led->strobe(BLUE, 0.5);

    // manual mode check
    if (this->manual) {
      INFO_LOG("Entering actuators manual mode");
      for (auto kv : stateDesired) {
        DEBUG_VAR("Writing to:", kv.key);
        DEBUG_VAR("State:", kv.value.as<bool>());

        if (strcmp(kv.key, "Act0") == 0) {
          this->jetPack->writeBit(0, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act1") == 0) {
          this->jetPack->writeBit(1, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act2") == 0) {
          this->jetPack->writeBit(2, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act3") == 0) {
          this->jetPack->writeBit(3, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act4") == 0) {
          this->jetPack->writeBit(4, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act5") == 0) {
          this->jetPack->writeBit(5, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act6") == 0) {
          this->jetPack->writeBit(6, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act7") == 0) {
          this->jetPack->writeBit(7, kv.value.as<bool>());
        } else if (strcmp(kv.key, "ActB") == 0) {
          this->onBoardActuator->write(kv.value.as<bool>());
        }
      }
    }

    CoolFileSystem::updateConfigFiles(stateDesired);
    JsonObject &newRoot = jsonBuffer.createObject();
    JsonObject &state = newRoot.createNestedObject("state");
    state["reported"] = stateDesired;
    state["desired"] = RawJson("null");
    String updateAnswer;
    newRoot.printTo(updateAnswer);
    DEBUG_VAR("Preparing answer message: ", updateAnswer);
    this->mqttPublish(updateAnswer.c_str());
    delay(10);
    if (doOta) {
      ESP.reset();
    }
  } else {
    ERROR_LOG("Failed to parse update message");
  }
}

unsigned long CoolBoard::getLogInterval() { return (this->logInterval); }

void CoolBoard::readSensors(JsonObject &reported) {

  digitalWrite(ENABLE_I2C_PIN, HIGH);
  this->coolBoardSensors->read(reported);

  if (this->externalSensorsActive) {
    this->externalSensors->read(reported);
  }
  if (this->ireneActive) {
    this->irene3000->read(reported);
  }
  this->led->blink(GREEN, 0.5);
}

void CoolBoard::readBoardData(JsonObject &reported) {
  reported["timestamp"] = this->rtc->getESDate();
  reported["mac"] = this->mqttId;
  if (this->isConnected()) {
    reported["wifiSignal"] = WiFi.RSSI();
  }
}

void CoolBoard::sleep(unsigned long interval) {
  if (interval > 0) {
    INFO_VAR("Going to sleep for seconds:", interval);
    ESP.deepSleep((uint64(interval) * 1000000ULL), WAKE_RF_DEFAULT);
  }
}

void CoolBoard::sendAllConfig() {
  this->sendConfig("CoolBoard", "/coolBoardConfig.json");
  this->sendConfig("CoolSensorsBoard", "/coolBoardSensorsConfig.json");
  this->sendConfig("CoolBoardActor", "/coolBoardActorConfig.json");
  this->sendConfig("rtc", "/rtcConfig.json");
  this->sendConfig("led", "/coolBoardLedConfig.json");
  if (this->jetpackActive) {
    this->sendConfig("jetPack", "/jetPackConfig.json");
  }
  if (this->ireneActive) {
    this->sendConfig("irene3000", "/irene3000Config.json");
  }
  if (this->externalSensorsActive) {
    this->sendConfig("externalSensors", "/externalSensorsConfig.json");
  }
}

bool CoolBoard::sendConfig(const char *moduleName, const char *filePath) {
  CoolConfig config(filePath);

  if (!config.readFileAsJson()) {
    ERROR_VAR("Failed to read configuration file:", filePath);
    return (false);
  }
  String message;
  DynamicJsonBuffer buffer;
  JsonObject &root = buffer.createObject();
  JsonObject &state = root.createNestedObject("state");
  JsonObject &reported = state.createNestedObject("reported");
  reported[moduleName] = config.get();
  root.printTo(message);
  DEBUG_VAR("JSON configuration message:", message);
  return (this->mqttPublish(message.c_str()));
}

void CoolBoard::sendPublicIP() {
  if (this->isConnected()) {
    String tempStr = this->wifiManager->getExternalIP();
    if (tempStr.length() > 6) {
      String publicIP = "{\"state\":{\"reported\":{\"publicIP\":";
      publicIP += tempStr;
      publicIP += "}}}";
      INFO_VAR("Sending public IP address:", tempStr);
      this->mqttPublish(publicIP.c_str());
    }
  }
}

void CoolBoard::startAP() {
  if (digitalRead(BOOTSTRAP_PIN) == LOW) {
    INFO_LOG("Bootstrap is in LOAD position, starting AP for further "
             "configuration...");
    this->wifiManager->disconnect();
    delay(200);
    this->led->write(FUCHSIA);
    this->wifiManager->connectAP();
    yield();
    delay(500);
    ESP.restart();
  }
}

void CoolBoard::mqttProblem() {
  this->led->blink(RED, 0.2);
  delay(200);
  this->led->blink(RED, 0.2);
  delay(200);
}

void CoolBoard::spiffsProblem() {
  this->led->write(RED);
  while (true) {
    yield();
  }
}

void CoolBoard::messageSent() { this->led->strobe(WHITE, 0.3); }

void CoolBoard::printMqttState(int state) {
  switch (state) {
  case MQTT_CONNECTION_TIMEOUT:
    ERROR_LOG("MQTT state: connection timeout");
    break;
  case MQTT_CONNECTION_LOST:
    ERROR_LOG("MQTT state: connection lost");
    break;
  case MQTT_CONNECT_FAILED:
    ERROR_LOG("MQTT state: connection failed");
    break;
  case MQTT_DISCONNECTED:
    ERROR_LOG("MQTT state: disconnected");
    break;
  case MQTT_CONNECTED:
    INFO_LOG("MQTT state: connected");
    break;
  case MQTT_CONNECT_BAD_PROTOCOL:
    ERROR_LOG("MQTT state: connection failed, bad protocol version");
    break;
  case MQTT_CONNECT_BAD_CLIENT_ID:
    ERROR_LOG("MQTT state: connection failed, bad client ID");
    break;
  case MQTT_CONNECT_UNAVAILABLE:
    ERROR_LOG("MQTT state: connection failed, server rejected client");
    break;
  case MQTT_CONNECT_BAD_CREDENTIALS:
    ERROR_LOG("MQTT state: connection failed, bad credentials");
    break;
  case MQTT_CONNECT_UNAUTHORIZED:
    ERROR_LOG("MQTT state: connection failed, client unauthorized");
    break;
  default:
    ERROR_LOG("MQTT state: connection status unknown");
  }
}

int CoolBoard::mqttConnect() {
  int i = 0;
  INFO_LOG("MQTT connecting...");
  DEBUG_VAR("MQTT client id:", this->mqttId);

  while (!this->mqttClient->connected() && i < MQTT_RETRIES) {
    if (this->mqttClient->connect(this->mqttId.c_str())) {
      this->mqttClient->subscribe(this->mqttInTopic.c_str());
      INFO_LOG("Subscribed to MQTT input topic");
    } else {
      WARN_LOG("MQTT connection failed, retrying");
    }
    delay(5);
    i++;
  }
  int state = this->mqttClient->state();
  this->printMqttState(state);
  return (state);
}

bool CoolBoard::mqttPublish(String data) {
  DEBUG_VAR("Message to publish:", data);
  DEBUG_VAR("Message size:", data.length());

  uint8_t retries = 0;
  bool published = false;
  do {
    published =
        this->mqttClient->publish(this->mqttOutTopic.c_str(),
                                  (byte *)(data.c_str()), data.length(), false);
    if (!published) {
      WARN_LOG("MQTT publish failed, retrying...");
      if (!this->isConnected()) {
        WARN_LOG("Wifi offline, reconnecting...");
        this->connect();
      }
    }
    retries++;
  } while (!published && retries < MQTT_RETRIES);
  if (published) {
    INFO_LOG("MQTT publish successful");
  } else {
    ERROR_LOG("MQTT publish failed, no more retries left!");
  }
  return (published);
}

bool CoolBoard::mqttListen() {
  bool success = false;
  unsigned long lastTime = millis();
  while ((millis() - lastTime) < 2000) {
    success = this->mqttClient->loop();
    if (!success) {
      this->connect();
    }
    yield();
  }
  return (success);
}

void CoolBoard::mqttCallback(char *topic, byte *payload, unsigned int length) {
  char temp[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    temp[i] = (char)payload[i];
  }
  temp[length] = '\0';
  this->update(temp);
}

void CoolBoard::otaUpdate() {
  this->led->write(255,128,80);
  delete this->led;
  delete this->irene3000;
  delete this->jetPack;
  delete this->mqttClient;
  delete this->rtc;
  delete this->coolBoardSensors;
  delete this->onBoardActuator;
  yield();
  if (this->wifiManager->wifiMulti.run() == WL_CONNECTED) {
    Serial.flush();
    Serial.setDebugOutput(true);
    delete this->wifiManager;
    if (this->fwURL.substring(0, 8) == "https://" &&
        this->fingerPrintURL.length() == 40) {
      DEBUG_LOG("START OTA HTTPS UPDATE");
      DEBUG_VAR("Url: ", this->fwURL);
      DEBUG_VAR("FingerPrint: ", this->fingerPrintURL);
      t_httpUpdate_return ret =
          ESPhttpUpdate.update(this->fwURL, "", this->fingerPrintURL, true);
      switch (ret) {
      case HTTP_UPDATE_FAILED:
        failCount++;
        if (failCount >= 6) {
          ERROR_LOG("To much tentative...");
          ERROR_LOG("Aborting OTA, Rebooting...");
          SPIFFS.begin();
          SPIFFS.remove("/UpdateOTA.json");
          ESP.restart();
          break;
        }
        ERROR_VAR("HTTP_UPDATE_FAILD Error (%d): %s",
                  ESPhttpUpdate.getLastError());
        Serial.printf(ESPhttpUpdate.getLastErrorString().c_str());

      case HTTP_UPDATE_OK:
        DEBUG_LOG("HTTP_UPDATE_OK");
        break;
      }
    } else if (this->fwURL.substring(0, 7) == "http://") {
      DEBUG_LOG("START OTA HTTP UPDATE");
      DEBUG_VAR("Url: ", this->fwURL);
      t_httpUpdate_return ret = ESPhttpUpdate.update(this->fwURL);
      switch (ret) {
      case HTTP_UPDATE_FAILED:
        failCount++;
        if (failCount >= 6) {
          ERROR_LOG("To much tentative...");
          ERROR_LOG("Aborting OTA, Rebooting...");
          ESP.restart();
          break;
        }
        ERROR_VAR("HTTP_UPDATE_FAILD Error: ", ESPhttpUpdate.getLastError());
        Serial.printf(ESPhttpUpdate.getLastErrorString().c_str());
        break;
      case HTTP_UPDATE_OK:
        DEBUG_LOG("HTTP_UPDATE_OK");
        break;
      }
    }
  }
}