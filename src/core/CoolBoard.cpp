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
#include "CoolLog.h"

#define SEND_MSG_BATCH 10

void CoolBoard::begin() {
  this->config();
  pinMode(ENABLE_I2C_PIN, OUTPUT);
  pinMode(BOOTSTRAP_PIN, INPUT);
  digitalWrite(ENABLE_I2C_PIN, HIGH);
  delay(100);

  this->rtc.config();
  this->rtc.offGrid();
  delay(100);

  this->coolBoardSensors.config();
  this->coolBoardSensors.begin();
  delay(100);

  this->onBoardActor.config();
  this->onBoardActor.begin();
  delay(100);

  this->wifiManager.config();

  delay(100);

  this->printConf();
  this->led.printConf();
  this->coolBoardSensors.printConf();
  this->onBoardActor.printConf();
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
    this->externalSensors.config();
    this->externalSensors.begin();
    delay(100);
  }

  this->connect();
  this->sendPublicIP();
  delay(100);
  this->rtc.begin();

  if (!this->sleepActive) {
    sendAllConfig();
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
  this->rtc.update();

  INFO_LOG("Listening to saved messages...");
  this->mqttListen();

  if (this->fileSystem.hasSavedLogs()) {
    INFO_LOG("Sending saved messages...");
    this->sendSavedMessages();
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  JsonObject &state = root.createNestedObject("state");
  JsonObject &reported = state.createNestedObject("reported");

  INFO_LOG("Collecting board and sensor data...");
  this->readBoardData(reported);
  this->readSensors(reported);

  INFO_LOG("Setting actuators and reporting their state...");
  this->handleActuators(reported);

  delay(50);

  if (this->mqttClient.state() != 0) {
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
      this->fileSystem.saveLogToFile(data.c_str());
      ERROR_LOG("MQTT publish failed! Data saved on SPIFFS");
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
  if (this->wifiManager.state() != WL_CONNECTED) {
    WARN_LOG("Wifi disconnected");
    return false;
  }

  if (this->mqttClient.state() != 0) {
    WARN_LOG("MQTT disconnected");
    return false;
  }
  return true;
}

int CoolBoard::connect() {
  if (this->wifiManager.wifiCount > 0) {
    this->wifiManager.config();
    this->led.write(BLUE);
    if (this->wifiManager.connect() != 3) {
      this->led.blink(RED, 10);
    } else {
      this->led.blink(GREEN, 5);
    }
  } else {
    INFO_LOG("No configured Wifi access point, launching configuration portal");
    this->wifiManager.disconnect();
    delay(200);
    this->led.write(FUCHSIA);
    this->wifiManager.connectAP();
  }
  delay(100);

  if (this->wifiManager.state() == WL_CONNECTED) {
    delay(100);
    if (this->mqttConnect() != 0) {
      this->mqttProblem();
    }
    delay(100);
  }
  return (this->mqttClient.state());
}

void CoolBoard::sendSavedMessages() {
  for (int i = 0; i < SEND_MSG_BATCH; i++) {
    int lastLog = this->fileSystem.lastSavedLogNumber();
    INFO_VAR("Sending saved log number:", lastLog);
    // only send a log if there is a log. 0 means zero files in the SPIFFS
    if (lastLog != 0) {
      DynamicJsonBuffer jsonBuffer;
      JsonObject &root = jsonBuffer.createObject();
      JsonObject &state = root.createNestedObject("state");
      state["reported"] =
          jsonBuffer.parseObject(fileSystem.getSavedLogAsString(lastLog));

      String jsonData;
      root.printTo(jsonData);
      DEBUG_VAR("Saved JSON data to send:", jsonData);
      if (this->mqttPublish(jsonData)) {
        this->fileSystem.deleteSavedLog(lastLog);
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
    tm = this->rtc.getTimeDate();

    if (this->jetpackActive) {
      DEBUG_LOG("Collecting Jetpack actuators data...");
      this->jetPack.doAction(reported, int(tm.Hour), int(tm.Minute));
    }

    DEBUG_LOG("Collecting onboard actuator data...");
    this->onBoardActor.doAction(reported, int(tm.Hour), int(tm.Minute));
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

bool CoolBoard::config() {
  INFO_VAR("MAC address is ", WiFi.macAddress());

  this->fileSystem.begin();
  this->led.config();
  this->led.begin();
  delay(10);
  this->led.write(YELLOW);
  File configFile = SPIFFS.open("/coolBoardConfig.json", "r");

  if (!configFile) {
    ERROR_LOG("Failed to read /coolBoardConfig.json");
    this->spiffsProblem();
    return (false);
  }

  else {
    String data = configFile.readString();
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(data);

    if (!json.success()) {
      ERROR_LOG("Failed to parse COOL Board configuration as JSON");
      this->spiffsProblem();
      return (false);
    } else {
      if (json["logInterval"].success()) {
        this->logInterval = json["logInterval"].as<unsigned long>();
      }
      json["logInterval"] = this->logInterval;
      if (json["ireneActive"].success()) {
        this->ireneActive = json["ireneActive"];
      }
      json["ireneActive"] = this->ireneActive;
      if (json["jetpackActive"].success()) {
        this->jetpackActive = json["jetpackActive"];
      }
      json["jetpackActive"] = this->jetpackActive;
      if (json["externalSensorsActive"].success()) {
        this->externalSensorsActive = json["externalSensorsActive"];
      }
      json["externalSensorsActive"] = this->externalSensorsActive;
      if (json["sleepActive"].success()) {
        this->sleepActive = json["sleepActive"];
      }
      json["sleepActive"] = this->sleepActive;
      if (json["manual"].success()) {
        this->manual = json["manual"].as<bool>();
      }
      json["manual"] = this->manual;
      if (json["mqttServer"].success()) {
        this->mqttServer = json["mqttServer"].as<String>();
      }
      json["mqttServer"] = this->mqttServer;

      this->mqttClient.setClient(this->wifiClient);
      this->mqttClient.setServer(this->mqttServer.c_str(), 1883);
      this->mqttClient.setCallback(
          [this](char *topic, byte *payload, unsigned int length) {
            this->mqttCallback(topic, payload, length);
          });
      this->mqttId = WiFi.macAddress();
      this->mqttId.replace(F(":"), F(""));
      this->mqttOutTopic = String(F("$aws/things/")) + this->mqttId +
                           String(F("/shadow/update"));
      this->mqttInTopic = String(F("$aws/things/")) + this->mqttId +
                          String(F("/shadow/update/delta"));
      configFile.close();
      configFile = SPIFFS.open("/coolBoardConfig.json", "w");

      if (!configFile) {
        ERROR_LOG("failed to write to /coolBoardConfig.json");
        this->spiffsProblem();
        return (false);
      }

      json.printTo(configFile);
      configFile.close();
      INFO_LOG("Configuration loaded");
      return (true);
    }
  }
}

void CoolBoard::printConf() {
  INFO_LOG("COOL Board configuration");
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
          this->onBoardActor.write(kv.value.as<bool>());
        }
      }
    }

    this->fileSystem.updateConfigFiles(stateDesired);

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
    this->externalSensors.read(reported);
  }
  if (this->ireneActive) {
    irene3000.read(reported);
  }
  this->led.blink(GREEN, 0.5);
}

void CoolBoard::readBoardData(JsonObject &reported) {
  reported["timestamp"] = rtc.getESDate();
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
  String result;

  File configFile = SPIFFS.open(filePath, "r");
  if (!configFile) {
    ERROR_VAR("Failed to read configuration file:", filePath);
    return (false);
  } else {
    DynamicJsonBuffer jsonBuffer;
    String data = configFile.readString();
    JsonObject &json = jsonBuffer.parseObject(data);

    if (!json.success()) {
      ERROR_LOG("Failed to parse JSON object");
      return (false);
    } else {
      String temporary;
      DEBUG_JSON("Configuration JSON:", json);
      DEBUG_VAR("JSON buffer size:", jsonBuffer.size());

      json.printTo(temporary);
      result = "{\"state\":{\"reported\":{\"";
      result += moduleName;
      result += "\":";
      result += temporary;
      result += "} } }";
      return (this->mqttPublish(result.c_str()));
    }
  }
}

void CoolBoard::sendPublicIP() {
  if (this->isConnected()) {
    String tempStr = this->wifiManager.getExternalIP();
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
    this->wifiManager.disconnect();
    delay(200);
    this->led.write(FUCHSIA);
    this->wifiManager.connectAP();
    yield();
    delay(500);
    ESP.restart();
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
  while (!this->mqttClient.connected() && i < MQTT_RETRIES) {
    if (this->mqttClient.connect(this->mqttId.c_str())) {
      this->mqttClient.subscribe(this->mqttInTopic.c_str());
      INFO_LOG("Subscribed to MQTT input topic");
    } else {
      WARN_LOG("MQTT connection failed, retrying");
    }
    delay(5);
    i++;
  }
  int state = this->mqttClient.state();
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
        this->mqttClient.publish(this->mqttOutTopic.c_str(),
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
    success = this->mqttClient.loop();
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
