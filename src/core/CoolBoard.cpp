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
#include "libb64/cdecode.h"

#define SEND_MSG_BATCH 10

void CoolBoard::begin() {
  if (!SPIFFS.begin()) {
    this->spiffsProblem();
  }
  this->led.config();
  this->led.begin();
  delay(10);
  this->led.write(YELLOW);
  this->config();
  pinMode(ENABLE_I2C_PIN, OUTPUT);
  pinMode(BOOTSTRAP_PIN, INPUT);
  digitalWrite(ENABLE_I2C_PIN, HIGH);
  delay(100);

  DEBUG_LOG("Start RTC configuration...");
  this->rtc.config();
  this->rtc.offGrid();
  delay(100);

  this->coolBoardSensors.config();
  this->coolBoardSensors.begin();
  delay(100);

  this->onBoardActuator.config();
  this->onBoardActuator.begin();
  delay(100);
  this->printConf();
  this->led.printConf();
  this->coolBoardSensors.printConf();
  this->onBoardActuator.printConf();
  this->rtc.printConf();

  if (this->jetpackActive) {
    this->jetPack.config();
    this->jetPack.begin();
    this->jetPack.printConf();
    delay(100);
  }

  if (this->ireneActive) {
    this->irene3000.config();
    this->irene3000.begin();
    this->irene3000.calibrate(this->led);
    this->irene3000.printConf();
    delay(100);
  }

  if (this->externalSensorsActive) {
    this->externalSensors->config();
    this->externalSensors->begin();
    delay(100);
  }
  this->rtc.begin();
  delay(100);
  SPIFFS.end();
}

void CoolBoard::loop() {
  if (!SPIFFS.begin()) {
    this->spiffsProblem();
  }
  if (!this->isConnected()) {
    INFO_LOG("Connecting...");
    this->connect();
  }
  INFO_LOG("Updating RTC...");
  this->rtc.update();
  if (!SPIFFS.exists("/configSent.flag")) {
    sendAllConfig();
    File f;
    if (!(f = SPIFFS.open("/configSent.flag", "w"))) {
      ERROR_LOG("Can't create file configSent.flag in SPIFFS");
    } else {
      f.close();
    }
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  JsonObject &state = root.createNestedObject("state");
  JsonObject &reported = state.createNestedObject("reported");
  INFO_LOG("Collecting board and sensor data...");
  this->readPublicIP(reported);
  this->readBoardData(reported);
  this->readSensors(reported);
  INFO_LOG("Setting actuators and reporting their state...");
  this->handleActuators(reported);
  delay(50);
  if (this->shouldLog()) {
    INFO_LOG("Sending log over MQTT...");
    String data;
    root.printTo(data);
    this->mqttLog(data);
    this->previousLogTime = millis();
  }
  this->startAP();
  INFO_LOG("Listening to saved messages...");
  this->mqttListen();
  if (CoolFileSystem::hasSavedLogs()) {
    INFO_LOG("Sending saved messages...");
    this->sendSavedMessages();
  }
  SPIFFS.end();
  if (this->sleepActive && !this->shouldLog()) {
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
    this->led.write(BLUE);
    this->wifiManager->config();
    this->wifiManager->connect();
  } else {
    INFO_LOG("No configured Wifi access point, launching configuration portal");
    this->wifiManager->disconnect();
    delay(200);
    this->led.write(FUCHSIA);
    this->wifiManager->connectAP();
  }
  delay(100);
  if (this->wifiManager->state() == WL_CONNECTED) {
    delay(100);
    this->led.blink(GREEN, 5);
    if (this->mqttConnect() != 0) {
      this->mqttProblem();
    }
    delay(100);
  } else {
    this->led.blink(RED, 10);
  }
  return (this->mqttClient->state());
}

void CoolBoard::sendSavedMessages() {
  int savedLogNumber;
  while ((savedLogNumber = CoolFileSystem::lastSavedLogNumber()) && !this->shouldLog()) {
    INFO_VAR("Sending saved log number:", savedLogNumber);
    String jsonData = CoolFileSystem::getSavedLogAsString(savedLogNumber);
    DEBUG_VAR("Saved JSON data to send:", jsonData);
    if (this->mqttPublish(jsonData)) {
      CoolFileSystem::deleteSavedLog(savedLogNumber);
      this->messageSent();
    } else {
      ERROR_LOG("MQTT publish failed, data stay on SPIFFS");
      this->mqttProblem();
      break;
    }
  }
}

void CoolBoard::handleActuators(JsonObject &reported) {
  if (this->manual == 0) {
    INFO_LOG("Actuators configuration: automatic");
    tmElements_t tm;
    tm = this->rtc.getTimeDate();
    if (this->jetpackActive) {
      DEBUG_LOG("Collecting Jetpack actuators data...");
      this->jetPack.doAction(reported, int(tm.Hour), int(tm.Minute));
    }
    DEBUG_LOG("Collecting onboard actuator data...");
    if (this->onBoardActuator.doAction(reported, int(tm.Hour),
                                       int(tm.Minute))) {
      this->onBoardActuator.write(1);
      reported["ActB"] = 1;
    } else {
      this->onBoardActuator.write(0);
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

int CoolBoard::b64decode(String b64Text, uint8_t *output) {
  base64_decodestate s;
  base64_init_decodestate(&s);
  int cnt = base64_decode_block(b64Text.c_str(), b64Text.length(),
                                (char *)output, &s);
  return cnt;
}

bool CoolBoard::config() {
  INFO_VAR("MAC address is:", WiFi.macAddress());
  INFO_VAR("Firmware version is:", COOL_FW_VERSION);
  this->wifiManager->config();
  this->tryFirmwareUpdate();
  CoolConfig config("/coolBoardConfig.json");
  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to parse main configuration");
    this->spiffsProblem();
    return (false);
  }
  this->wifiManager->config();
  JsonObject &json = config.get();
  config.set<unsigned long>(json, "logInterval", this->logInterval);
  config.set<bool>(json, "ireneActive", this->ireneActive);
  config.set<bool>(json, "jetpackActive", this->jetpackActive);
  config.set<bool>(json, "externalSensorsActive", this->externalSensorsActive);
  config.set<bool>(json, "sleepActive", this->sleepActive);
  config.set<bool>(json, "manual", this->manual);
  config.set<String>(json, "mqttServer", this->mqttServer);
  this->mqttsConfig();
  INFO_LOG("Main configuration loaded");
  return (true);
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
    JsonObject &firmwareJson = stateDesired["CoolBoard"]["firmwareUpdate"];
    if (firmwareJson.success()) {
      String firmwareVersion = firmwareJson.get<String>("firmwareVersion");
      if (String(COOL_FW_VERSION) == firmwareVersion) {
        INFO_LOG("You firmware version is up to date!");
        stateDesired["CoolBoard"]["firmwareUpdate"] = NULL;
      } else {
        File otaUpdateConfig = SPIFFS.open("/otaUpdateConfig.json", "w");
        if (!otaUpdateConfig) {
          ERROR_LOG("Failed to create firmware update configuration file!");
        } else {
          DEBUG_VAR("Firmware update scheduled, target version:",
                    firmwareVersion);
          firmwareJson.printTo(otaUpdateConfig);
          otaUpdateConfig.close();
          DEBUG_JSON("Saved OTA HTTPS update configuration to: ", firmwareJson);
          INFO_LOG("New firmaware update received, your board will now reboot to apply...");
          delay(100);
          ESP.restart();
        }
      }
    }

    this->led.strobe(BLUE, 0.5);

    // manual mode check
    if (this->manual) {
      INFO_LOG("Entering actuators manual mode");
      for (auto kv : stateDesired) {
        DEBUG_VAR("Writing to:", kv.key);
        DEBUG_VAR("State:", kv.value.as<bool>());

        if (strcmp(kv.key, "Act0") == 0) {
          this->jetPack.writeBit(0, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act1") == 0) {
          this->jetPack.writeBit(1, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act2") == 0) {
          this->jetPack.writeBit(2, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act3") == 0) {
          this->jetPack.writeBit(3, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act4") == 0) {
          this->jetPack.writeBit(4, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act5") == 0) {
          this->jetPack.writeBit(5, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act6") == 0) {
          this->jetPack.writeBit(6, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act7") == 0) {
          this->jetPack.writeBit(7, kv.value.as<bool>());
        } else if (strcmp(kv.key, "ActB") == 0) {
          this->onBoardActuator.write(kv.value.as<bool>());
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
  } else {
    ERROR_LOG("Failed to parse update message");
  }
}

unsigned long CoolBoard::getLogInterval() { return (this->logInterval); }

void CoolBoard::readSensors(JsonObject &reported) {

  digitalWrite(ENABLE_I2C_PIN, HIGH);
  this->coolBoardSensors.read(reported);

  if (this->externalSensorsActive) {
    this->externalSensors->read(reported);
  }
  if (this->ireneActive) {
    this->irene3000.read(reported);
  }
  this->led.blink(GREEN, 0.5);
}

void CoolBoard::readBoardData(JsonObject &reported) {
  reported["timestamp"] = this->rtc.getESDate();
  reported["mac"] = this->mqttId;
  reported["firmwareVersion"] = COOL_FW_VERSION;
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

void CoolBoard::sendConfig(const char *moduleName, const char *filePath) {
  CoolConfig config(filePath);

  if (!config.readFileAsJson()) {
    ERROR_VAR("Failed to read configuration file:", filePath);
    this->spiffsProblem();
  }
  String message;
  DynamicJsonBuffer buffer;
  JsonObject &root = buffer.createObject();
  JsonObject &state = root.createNestedObject("state");
  JsonObject &reported = state.createNestedObject("reported");
  reported[moduleName] = config.get();
  root.printTo(message);
  DEBUG_VAR("JSON configuration message:", message);
  mqttLog(message);
}

void CoolBoard::readPublicIP(JsonObject &reported) {
  if (this->isConnected()) {
    reported["publicIp"] = this->wifiManager->getExternalIP();
  }
}

void CoolBoard::startAP() {
  if (digitalRead(BOOTSTRAP_PIN) == LOW) {
    INFO_LOG("Bootstrap is in LOAD position, starting AP for further "
             "configuration...");
    this->wifiManager->disconnect();
    delay(200);
    this->led.write(FUCHSIA);
    this->wifiManager->connectAP();
    yield();
    if (this->wifiManager->state() == WL_CONNECTED) {
    this->led.blink(GREEN, 5);
    } else {
      this->led.blink(RED, 10);
    }
    delay(500);
  }
}

void CoolBoard::mqttProblem() {
  this->led.blink(RED, 0.2);
  delay(200);
  this->led.blink(RED, 0.2);
  delay(200);
}

void CoolBoard::spiffsProblem() {
  this->led.write(RED);
  while (true) {
    yield();
  }
}

void CoolBoard::messageSent() { this->led.strobe(WHITE, 0.3); }

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
    this->mqttsConfig();
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

void CoolBoard::mqttLog(String data) {
  DEBUG_VAR("Message to publish:", data);
  DEBUG_VAR("Message size:", data.length());
  bool messageSent = false;
  if (this->isConnected()) {
    messageSent = this->mqttPublish(data);
  }
  if (!this->isConnected() || !messageSent) {
    ERROR_LOG("MQTT publish failed, data saved on SPIFFS");
    CoolFileSystem::saveLogToFile(data.c_str());
    this->mqttProblem();
  } else {
    INFO_LOG("MQTT publish successful");
    this->messageSent();
  }
}

bool CoolBoard::mqttPublish(String data) {
  return (this->mqttClient->publish(this->mqttOutTopic.c_str(),
                                    (byte *)(data.c_str()), data.length(),
                                    false));
}

bool CoolBoard::mqttListen() {
  bool success = false;
  unsigned long lastTime = millis();
  while ((millis() - lastTime) < 2000) {
    success = this->mqttClient->loop();
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

void CoolBoard::mqttsConvert(String cert) {
  File bin = SPIFFS.open(cert, "r");
  String certficateString = bin.readString();
  bin.close();
  DEBUG_LOG("Closing bin file");
  uint8_t binaryCert[certficateString.length()];
  uint16_t len = this->b64decode(certficateString, binaryCert);
  if (strstr(cert.c_str(), "certificate")) {
    this->wifiClient->setCertificate(binaryCert, len);
  } else {
    this->wifiClient->setPrivateKey(binaryCert, len);
  }
}

bool CoolBoard::mqttsConfig() {
  if (SPIFFS.exists("/certificate.bin") && SPIFFS.exists("/privateKey.bin")) {
    DEBUG_LOG("Loading Certificate");
    this->mqttsConvert("/certificate.bin");
    DEBUG_LOG("Loading privateKey");
    this->mqttsConvert("/privateKey.bin");
    DEBUG_LOG("Setting wifiClient");
    this->mqttClient->setClient(*this->wifiClient);
    this->mqttClient->setServer(this->mqttServer.c_str(), 8883);
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
    return (true);
  } else {
    this->spiffsProblem();
    ERROR_LOG("Certificate & Key binaries not found");
    DEBUG_VAR("/certificate.bin exist return: ",
              SPIFFS.exists("/certificate.bin"));
    DEBUG_VAR("/privateKey.bin exist return: ",
              SPIFFS.exists("/privateKey.bin"));
    return (false);
  }
}

void CoolBoard::tryFirmwareUpdate() {
  File config = SPIFFS.open("/otaUpdateConfig.json", "r");
  if (config) {
    INFO_LOG("Parsing firmware update configuration...");
    DynamicJsonBuffer buffer;
    JsonObject &json = buffer.parseObject(config.readString());
    DEBUG_JSON("Firmware update json:", json);
    INFO_VAR("Target version:", json.get<String>("firmwareVersion"));
    DEBUG_VAR("Firmware URL:", json.get<String>("firmwareUrl"));
    DEBUG_VAR("Server fingerprint: ",
              json.get<String>("firmwareUrlFingerprint"));
    config.close();
    if (json["firmwareUrlFingerprint"].success() &&
        json["firmwareUrl"].success() && json["firmwareVersion"].success()) {
      INFO_VAR("Firmware has to be updated from version:", COOL_FW_VERSION);
      this->updateFirmware(json.get<String>("firmwareVersion"),
                           json.get<String>("firmwareUrl"),
                           json.get<String>("firmwareUrlFingerprint"));
    } else {
      INFO_LOG("Failed to prepare firmware update (missing configuration)");
    }
  } else {
    ERROR_LOG("Failed to read firmware update configuration");
  }
}

void CoolBoard::updateFirmware(String firmwareVersion, String firmwareUrl,
                               String firmwareUrlFingerprint) {
  this->led.write(BROWN);
  delete this->mqttClient;
  delete this->externalSensors;
  delete this->wifiClient;
  SPIFFS.remove("/otaUpdateConfig.json");
  SPIFFS.end();
  delay(100);
  if (this->wifiManager->connect() == WL_CONNECTED) {
    Serial.flush();
    Serial.setDebugOutput(true);
    delete this->wifiManager;
    INFO_LOG("Starting firmware update...");
    t_httpUpdate_return ret =
        ESPhttpUpdate.update(firmwareUrl, "", firmwareUrlFingerprint, true);
    switch (ret) {
    case HTTP_UPDATE_FAILED:
      ERROR_VAR("HTTP Update failed, code:", ESPhttpUpdate.getLastError());
      ERROR_VAR("Error message:", ESPhttpUpdate.getLastErrorString().c_str());
      this->led.write(RED);
      break;
    case HTTP_UPDATE_OK:
      INFO_LOG("HTTP update succeeded!");
      break;
    }
  } else {
    ERROR_LOG("Cannot connect to Wifi, cancelling firmware update...");
  }
  ESP.restart();
}
