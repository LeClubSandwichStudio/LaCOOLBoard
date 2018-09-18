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
#include <EEPROM.h>
#include <FS.h>
#include <memory>

#include "CoolBoard.h"
#include "CoolConfig.h"
#include "CoolLog.h"
#include "libb64/cdecode.h"

void CoolBoard::begin() {
  this->powerCheck();
  WiFi.mode(WIFI_STA);
  if (!SPIFFS.begin()) {
    this->spiffsProblem();
  }
  this->sleep();
  if (!this->coolBoardLed.config()) {
    this->spiffsProblem();
  }
  this->coolBoardLed.begin();
  delay(10);
  this->coolBoardLed.write(YELLOW);
  if (!this->config()) {
    this->spiffsProblem();
  }
  pinMode(ENABLE_I2C_PIN, OUTPUT);
  pinMode(BOOTSTRAP_PIN, INPUT);
  digitalWrite(ENABLE_I2C_PIN, HIGH);
  delay(100);
  if (!this->coolBoardSensors.config()) {
    this->spiffsProblem();
  }
  this->coolBoardSensors.begin();
  delay(100);
  this->printConf();
  this->coolBoardLed.printConf();
  this->coolBoardSensors.printConf();
  if (!this->jetPack.config()) {
    this->spiffsProblem();
  }
  this->jetPack.begin();
  this->jetPack.printConf();
  delay(100);
  if (!this->irene3000.config()) {
    this->spiffsProblem();
  }
  this->irene3000.begin();
  this->irene3000.calibrate(this->coolBoardLed);
  this->irene3000.printConf();
  delay(100);
  if (!this->externalSensors->config()) {
    this->spiffsProblem();
  }
  this->externalSensors->begin();
  delay(100);
  this->mqttsConfig();
  delay(100);
  SPIFFS.end();
}

void CoolBoard::loop() {
  this->powerCheck();
  if (!SPIFFS.begin()) {
    this->spiffsProblem();
  }
  if (!this->isConnected()) {
    this->coolPubSubClient->disconnect();
    INFO_LOG("Connecting...");
    this->connect();
  }
  INFO_LOG("Synchronizing RTC...");
  bool rtcSynced = CoolTime::getInstance().sync();
  if (!rtcSynced) {
    this->clockProblem();
  } else {
    INFO_LOG("Collecting board and sensor data...");
    char *logLoop = this->createLog();
    delay(50);
    if (this->shouldLog()) {
      INFO_LOG("Sending log over MQTT...");
      this->mqttLog(logLoop, 1);
      this->previousLogTime = millis();
    }
    free(logLoop);
    if (CoolFileSystem::hasSavedLogs()) {
      INFO_LOG("Sending saved messages...");
      this->sendSavedMessages();
    }
    INFO_LOG("Listening to update messages...");
    this->mqttListen();
    if (this->updateAnswer != "") {
      if (this->update(this->updateAnswer)) {
        if (this->connection) {
          mqttLog(this->updateAnswer.c_str());
          this->updateAnswer = "";
        } else {
          mqttLog(this->updateAnswer.c_str());
          this->updateAnswer = "";
          SPIFFS.end();
          ESP.restart();
        }
      }
    }
  }
  SPIFFS.end();
  if (this->sleepActive && (!this->shouldLog() || !rtcSynced)) {
    this->sleep();
  }
}

char *CoolBoard::createLog() {
  DynamicJsonBuffer buffer;
  JsonObject &root = buffer.createObject();
  JsonObject &state = root.createNestedObject("state");
  JsonObject &reported = state.createNestedObject("reported");
  this->readBoardData(reported);
  this->readSensors(reported);
  this->handleActuators(reported);
  uint32_t size = CoolMessagePack::sizeJsonToMsgpck(root);
  size = size + (size % 4 == 0 ? 0 : 4 - size % 4);
  DEBUG_VAR("message size format in mpack", size);
  char buf[size];
  bzero(buf, size);
  GString str = buf;
  PrintAdapter streamer = str;
  CoolMessagePack::jsonToMsgpck(streamer, root);
  char *output;
  if (!(output = (char *)malloc((size_t)size * 1.25 + 3))) {
    ERROR_LOG("malloc return null");
  }
  output[0] = '"';
  size_t index = Z85_encode(buf, &output[1], size);
  output[index + 1] = '"';
  output[index + 2] = '\0';
  return (output);
}

bool CoolBoard::isConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }
  if (this->coolPubSubClient->state() != MQTT_CONNECTED) {
    return false;
  }
  return true;
}

void CoolBoard::connect() {
  if (this->coolWifi->wifiCount > 0 && digitalRead(BOOTSTRAP_PIN) == HIGH) {
    this->coolBoardLed.write(BLUE);
    this->coolWifi->connect();
  } else {
    INFO_LOG("Starting Wifi access point and configuration portal");
    this->coolWifi->startAccessPoint(this->coolBoardLed);
  }
  delay(100);
  if (WiFi.status() == WL_CONNECTED) {
    CoolTime::getInstance().begin();
    delay(100);
    this->mqttConnect();
    delay(100);
  }
  if (!this->isConnected()) {
    this->networkProblem();
  }
}

void CoolBoard::sendSavedMessages() {
  int savedLogNumber;
  while ((savedLogNumber = CoolFileSystem::lastSavedLogNumber()) &&
         !this->shouldLog()) {
    INFO_VAR("Sending saved log number:", savedLogNumber);
    String jsonData = CoolFileSystem::getSavedLogAsString(savedLogNumber);
    DEBUG_VAR("Saved JSON data to send:", jsonData);
    if (this->mqttPublish(jsonData)) {
      CoolFileSystem::deleteSavedLog(savedLogNumber);
      this->messageSent();
    } else {
      this->networkProblem();
      ERROR_LOG("MQTT publish failed, kept log on SPIFFS");
      break;
    }
  }
}

void CoolBoard::handleActuators(JsonObject &root) {
  if (this->manual == 0) {
    Date date = CoolTime::getInstance().rtc.getDate();
    root.createNestedArray("actuators");
    INFO_LOG("Actuators configuration: automatic");
    DEBUG_LOG("Updating and recording Jetpack state...");
    this->jetPack.doAction(root, date.getHour(), date.getMinutes());
    DEBUG_LOG("Updating and recording onboard actuator state...");
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
  uint32_t seconds;

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
  CoolConfig config("/general.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to parse /general.json");
    this->spiffsProblem();
  }
  JsonObject &root = config.get();
  this->coolWifi->config();
  this->tryFirmwareUpdate();
  JsonObject &general = root["general"];
  CoolConfig::set<unsigned long>(general, "logInterval", this->logInterval);
  CoolConfig::set<bool>(general, "sleepActive", this->sleepActive);
  CoolConfig::set<bool>(general, "manual", this->manual);
  CoolConfig::set<String>(general, "mqttServer", this->mqttServer);
  INFO_LOG("Main configuration loaded");
  return (true);
}

void CoolBoard::printConf() {
  INFO_LOG("General configuration");
  INFO_VAR("  Log interval            =", this->logInterval);
  INFO_VAR("  Sleep active            =", this->sleepActive);
  INFO_VAR("  Manual active           =", this->manual);
  INFO_VAR("  MQTT server:            =", this->mqttServer);
}

bool CoolBoard::update(String &answer) {
  INFO_LOG("Received new MQTT message");
  if (answer.length() > LITTLE_ANSWER_MAX_SIZE) {
    this->coolPubSubClient->disconnect();
    delay(100);
    this->coolPubSubClient->disconnect();
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(answer);
  answer = "";
  if (root.success()) {
    DEBUG_JSON("Desired state JSON:", root);
    if (root["actuators"]["manual"].success()) {
      this->manual = root["actuators"]["manual"].as<bool>();
      INFO_VAR("Manual flag received:", this->manual);
      this->connection = 1;
    }
    JsonObject &firmwareJson = root["CoolBoard"]["firmwareUpdate"];
    if (firmwareJson.success()) {
      String firmwareVersion = firmwareJson.get<String>("firmwareVersion");
      if (String(COOL_FW_VERSION) == firmwareVersion) {
        INFO_LOG("You firmware version is up to date!");
        root["CoolBoard"]["firmwareUpdate"] = NULL;
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
          INFO_LOG("New firmware update received, your board will now reboot "
                   "to apply...");
          delay(100);
          ESP.restart();
        }
      }
    }
    this->coolBoardLed.strobe(BLUE, 0.5);
    if (this->manual) {
      this->connection = 1;
      INFO_LOG("Entering actuators manual mode");
      JsonArray &actuators = root["actuators"]["enabled"];
      if (actuators.success()) {
        uint8_t i = 0;
        for (auto kv : actuators) {
          DEBUG_VAR("Actuator #", i);
          DEBUG_VAR("State:", actuators.get<bool>(i));
          if (i == 0) {
            this->coolBoardActuator.write(kv);
          } else if (i == 1) {
            this->jetPack.writeBit(0, kv);
          } else if (i == 2) {
            this->jetPack.writeBit(1, kv);
          } else if (i == 3) {
            this->jetPack.writeBit(2, kv);
          } else if (i == 4) {
            this->jetPack.writeBit(3, kv);
          } else if (i == 5) {
            this->jetPack.writeBit(4, kv);
          } else if (i == 6) {
            this->jetPack.writeBit(5, kv);
          } else if (i == 7) {
            this->jetPack.writeBit(6, kv);
          } else if (i == 8) {
            this->jetPack.writeBit(7, kv);
          }
          i++;
        }
      }
    }
    CoolFileSystem::updateConfigFiles(root);
    JsonObject &newRoot = jsonBuffer.createObject();
    JsonObject &state = newRoot.createNestedObject("state");
    state["desired"] = RawJson("null");
    if (this->connection) {
      state["reported"] = root;
    }
    newRoot.printTo(answer);
    DEBUG_VAR("Preparing answer message: ", answer);
    delay(10);
    if (SPIFFS.exists("/configSent.flag")) {
      if (SPIFFS.remove("/configSent.flag")) {
        DEBUG_LOG("Delete /configSent.flag");
      } else {
        ERROR_LOG("Failed to delete /configSent.flag");
      }
    }
    return (1);
  } else {
    ERROR_LOG("Failed to parse update message");
    return (0);
  }
}

unsigned long CoolBoard::getLogInterval() { return (this->logInterval); }

void CoolBoard::readSensors(JsonObject &root) {
  JsonObject &sample = root.createNestedObject("sample");
  digitalWrite(ENABLE_I2C_PIN, HIGH);
  this->coolBoardSensors.read(sample);
  this->externalSensors->read(sample);
  this->irene3000.read(sample);
  this->coolBoardLed.blink(GREEN, 0.5);
}

void CoolBoard::readBoardData(JsonObject &root) {
  root["timestamp"] = CoolTime::getInstance().getIso8601DateTime();
  JsonObject &stat = root.createNestedObject("static");
  JsonObject &general = root.createNestedObject("system");
  if (WiFi.status() == WL_CONNECTED) {
    String ip;
    if (this->coolWifi->getPublicIp(ip)) {
      DEBUG_VAR("Public IP address:", ip);
      general["publicIp"] = ip;
    }
  }
  general["fwVersion"] = COOL_FW_VERSION;
  if (WiFi.status() == WL_CONNECTED) {
    general["wifiSignal"] = WiFi.RSSI();
  }
  stat["macAddress"] = this->mqttId;
}

void CoolBoard::sleep() {
  EEPROM.begin(5);
  uint8_t val[4];
  rst_info *resetInfo;
  resetInfo = ESP.getResetInfoPtr();
  if (!EEPROM.read(0x04) || resetInfo->reason != REASON_DEEP_SLEEP_AWAKE) {
    INFO_LOG("Reset of the logInterval");
    for (int i = 0; i < 5; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.write(0x04, 1);
  }
  uint32_t value = ((uint32_t)EEPROM.read(0x00)) +
                   (((uint32_t)EEPROM.read(0x01)) << 8) +
                   (((uint32_t)EEPROM.read(0x02)) << 16) +
                   (((uint32_t)EEPROM.read(0x03)) << 24);
  if ((value) || (!this->shouldLog())) {
    if (!value) {
      value = secondsToNextLog();
    }
    if (value > MAX_SLEEP_TIME) {
      INFO_VAR("Going to sleep for:", MAX_SLEEP_TIME);
      INFO_VAR("And need to sleep again for", value - MAX_SLEEP_TIME);
      for (uint8_t i = 0; i < 4; i++) {
        val[i] = ((value - MAX_SLEEP_TIME) >> i * 8);
        EEPROM.write(i, val[i]);
      }
      EEPROM.end();
      ESP.deepSleep((uint64_t(MAX_SLEEP_TIME) * 1000000ULL), WAKE_RF_DEFAULT);
    } else {
      INFO_VAR("Going to sleep for:", value);
      for (uint8_t i = 0; i < 4; i++) {
        EEPROM.write(i, 0);
      }
      EEPROM.end();
      ESP.deepSleep((uint64_t(value) * 1000000ULL), WAKE_RF_DEFAULT);
    }
  }
  EEPROM.end();
}

void CoolBoard::parseJsonConfig(const char *filePath, JsonObject &send) {
  JsonObject &state = send.createNestedObject("state");
  JsonObject &reported = state.createNestedObject("reported");
  reported.createNestedObject("config");
  CoolConfig configuration(filePath);

  if (!configuration.readFileAsJson()) {
    ERROR_VAR("Failed to read ", filePath);
    this->spiffsProblem();
  }
  reported["config"] = configuration.get();
}

void CoolBoard::sendConfig(const char *path) {
  DynamicJsonBuffer buffer;
  JsonObject &json = buffer.createObject();
  parseJsonConfig(path, json);
  size_t size = CoolMessagePack::sizeJsonToMsgpck(json);
  size = size + (size % 4 == 0 ? 0 : 4 - size % 4);
  DEBUG_VAR("message size format in mpack", size);
  char buf[size];
  GString str = buf;
  PrintAdapter streamer = str;
  CoolMessagePack::jsonToMsgpck(streamer, json);
  char output[(size_t)(size * 1.25 + 3)];
  output[0] = '"';
  int index = Z85_encode(buf, &output[1], size);
  output[index + 1] = '"';
  output[index + 2] = '\0';
  this->mqttLog(output, 1);
}

void CoolBoard::sendAllConfig() {
  this->sendConfig("/general.json");
  delay(50);
  this->sendConfig("/sensors.json");
  delay(50);
  this->sendConfig("/actuators.json");
}

void CoolBoard::networkProblem() {
  WARN_LOG("Network unreachable");
  CoolWifi::printStatus(WiFi.status());
  this->printMqttState(this->coolPubSubClient->state());
  for (uint8_t i = 0; i < 8; i++) {
    this->coolBoardLed.blink(ORANGE, 0.2);
  }
}

void CoolBoard::clockProblem() {
  ERROR_LOG("NTP failed and RTC has stopped, not doing anything!");
  for (uint8_t i = 0; i < 8; i++) {
    this->coolBoardLed.blink(RED, 0.2);
  }
}

void CoolBoard::spiffsProblem() {
  CRITICAL_LOG("Filesystem failed, please contact support!");
  this->coolBoardLed.write(RED);
  while (true) {
    yield();
  }
}

void CoolBoard::lowBattery() {
  WiFi.mode(WIFI_OFF);
  SPIFFS.end();
  ESP.deepSleep((uint64_t(LOW_POWER_SLEEP) * 1000000ULL), WAKE_RF_DEFAULT);
}

void CoolBoard::powerCheck() {
  float batteryVoltage = this->coolBoardSensors.readVBat();
  if (!(batteryVoltage < NOT_IN_CHARGING || batteryVoltage > MIN_BAT_VOLTAGE)) {
    DEBUG_VAR("Battery voltage:", batteryVoltage);
    WARN_LOG("Battery Power is low! Need to charge!");
    this->lowBattery();
  }
}

void CoolBoard::messageSent() { this->coolBoardLed.strobe(WHITE, 0.3); }

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

void CoolBoard::mqttConnect() {
  uint8_t i = 0;

  INFO_LOG("MQTT connecting...");
  DEBUG_VAR("MQTT client id:", this->mqttId);
  while (!this->coolPubSubClient->connected() && i < MQTT_RETRIES) {
    this->mqttsConfig();
    INFO_LOG("start mqtt connection");
    if (this->coolPubSubClient->connect(this->mqttId.c_str())) {
      this->coolPubSubClient->subscribe(this->mqttInTopic.c_str());
      INFO_LOG("Subscribed to MQTT input topic");
      mqttRetries = 0;
    } else {
      WARN_LOG("MQTT connection failed, retrying");
      mqttRetries++;
    }
    delay(5);
    i++;
  }
  if (mqttRetries >= MAX_MQTT_RETRIES) {
    SPIFFS.end();
    ESP.restart();
  }
}

void CoolBoard::mqttLog(String data, bool mpack) {
  bool messageSent = false;

  DEBUG_VAR("Message to log:", data);
  DEBUG_VAR("Message size:", data.length());
  if (this->isConnected()) {
    messageSent = this->mqttPublish(data, mpack);
  }
  if (!messageSent) {
    CoolFileSystem::saveLogToFile(data.c_str());
    this->networkProblem();
    WARN_LOG("Log not sent, saved on SPIFFS");
  } else {
    INFO_LOG("MQTT publish successful");
    this->messageSent();
  }
}

bool CoolBoard::mqttPublish(String data, bool mpack) {
  if (!mpack) {
    return (this->coolPubSubClient->publish(this->mqttOutTopic.c_str(),
                                            (byte *)(data.c_str()),
                                            data.length(), false));
  }
  return (this->coolPubSubClient->publish(this->mqttOutMpackTopic.c_str(),
                                          (byte *)(data.c_str()), data.length(),
                                          false));
}

bool CoolBoard::mqttListen() {
  bool success = false;
  unsigned long lastTime = millis();
  while ((millis() - lastTime) < 2000) {
    success = this->coolPubSubClient->loop();
    yield();
  }
  return (success);
}

void CoolBoard::mqttCallback(char *topic, byte *payload, unsigned int length) {
  this->updateAnswer = (char)payload[0];
  for (unsigned int i = 1; i < length; i++) {
    this->updateAnswer += (char)payload[i];
  }
  this->updateAnswer += '\0';
}

void CoolBoard::mqttsConvert(String cert) {
  File bin = SPIFFS.open(cert, "r");
  String certficateString = bin.readString();
  bin.close();
  DEBUG_LOG("Closing bin file");
  uint8_t binaryCert[certficateString.length()];
  uint16_t len = this->b64decode(certficateString, binaryCert);
  if (strstr(cert.c_str(), "certificate")) {
    this->wifiClientSecure->setCertificate(binaryCert, len);
  } else {
    this->wifiClientSecure->setPrivateKey(binaryCert, len);
  }
}

void CoolBoard::mqttsConfig() {
  if (SPIFFS.exists("/certificate.bin") && SPIFFS.exists("/privateKey.bin")) {
    DEBUG_LOG("Loading X509 certificate");
    this->mqttsConvert("/certificate.bin");
    DEBUG_LOG("Loading X509 private key");
    this->mqttsConvert("/privateKey.bin");
    DEBUG_LOG("Configuring MQTT");
    this->coolPubSubClient->setClient(*this->wifiClientSecure);
    this->coolPubSubClient->setServer(this->mqttServer.c_str(), 8883);
    this->coolPubSubClient->setCallback(
        [this](char *topic, byte *payload, unsigned int length) {
          this->mqttCallback(topic, payload, length);
        });
    this->mqttId = WiFi.macAddress();
    this->mqttId.replace(F(":"), F(""));
    this->mqttOutTopic =
        String(F("$aws/things/")) + this->mqttId + String(F("/shadow/update"));
    this->mqttInTopic =
        String(F("things/")) + this->mqttId + String(F("/shadow/update/delta"));
    this->mqttOutMpackTopic = String(F("BoardMessage/record"));
  } else {
    ERROR_LOG("Certificate & Key binaries not found");
    DEBUG_VAR("/certificate.bin exist return: ",
              SPIFFS.exists("/certificate.bin"));
    DEBUG_VAR("/privateKey.bin exist return: ",
              SPIFFS.exists("/privateKey.bin"));
    this->spiffsProblem();
  }
  DEBUG_LOG("end mqtt config");
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
    INFO_LOG("No firmware update pending");
  }
}

void CoolBoard::updateFirmware(String firmwareVersion, String firmwareUrl,
                               String firmwareUrlFingerprint) {
  this->coolBoardLed.write(ORANGE);
  delete this->coolPubSubClient;
  delete this->externalSensors;
  delete this->wifiClientSecure;
  SPIFFS.remove("/otaUpdateConfig.json");
  SPIFFS.end();
  delay(100);
  this->coolWifi->connect();
  if (WiFi.status() == WL_CONNECTED) {
    delete this->coolWifi;
    INFO_LOG("Starting firmware update...");
    t_httpUpdate_return ret =
        ESPhttpUpdate.update(firmwareUrl, "", firmwareUrlFingerprint);
    switch (ret) {
    case HTTP_UPDATE_FAILED:
      ERROR_VAR("HTTP Update failed, code:", ESPhttpUpdate.getLastError());
      ERROR_VAR("Error message:", ESPhttpUpdate.getLastErrorString().c_str());
      this->coolBoardLed.write(RED);
      break;
    case HTTP_UPDATE_OK:
      INFO_LOG("HTTP update succeeded!");
      break;
    case HTTP_UPDATE_NO_UPDATES:
      break;
    }
  } else {
    ERROR_LOG("Cannot connect to Wifi, cancelling firmware update...");
  }
  ESP.restart();
}