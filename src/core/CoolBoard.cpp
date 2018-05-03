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
  pinMode(ENABLE_I2C_PIN, OUTPUT);
  pinMode(BOOTSTRAP_PIN, INPUT);
  digitalWrite(ENABLE_I2C_PIN, HIGH);
  delay(100);

  rtc.config();
  rtc.offGrid();
  delay(100);

  coolBoardSensors.config();
  coolBoardSensors.begin();
  delay(100);

  onBoardActor.config();
  onBoardActor.begin();
  delay(100);

  wifiManager.config();
  wifiManager.begin();
  delay(100);

  mqtt.config();
  mqtt.begin();
  delay(100);

  this->printConf();
  coolBoardLed.printConf();
  coolBoardSensors.printConf();
  onBoardActor.printConf();
  wifiManager.printConf();
  mqtt.printConf();

  if (jetpackActive) {
    jetPack.config();
    jetPack.begin();
    jetPack.printConf();
    delay(100);
  }

  if (ireneActive) {
    irene3000.config();
    irene3000.begin();
    irene3000.calibrate(this->coolBoardLed);
    irene3000.printConf();
    delay(100);
  }

  if (externalSensorsActive) {
    externalSensors.config();
    externalSensors.begin();
    delay(100);
    externalSensors.printConf();
  }

  this->connect();
  delay(100);
  rtc.begin();

  if (this->sleepActive == 0) {
    this->sendConfig("CoolBoard", "/coolBoardConfig.json");
    this->sendConfig("CoolSensorsBoard", "/coolBoardSensorsConfig.json");
    this->sendConfig("CoolBoardActor", "/coolBoardActorConfig.json");
    this->sendConfig("rtc", "/rtcConfig.json");
    this->sendConfig("led", "/coolBoardLedConfig.json");
    if (jetpackActive) {
      this->sendConfig("jetPack", "/jetPackConfig.json");
    }
    if (ireneActive) {
      this->sendConfig("irene3000", "/irene3000Config.json");
    }
    if (externalSensorsActive) {
      this->sendConfig("externalSensors", "/externalSensorsConfig.json");
    }
    this->sendConfig("mqtt", "/mqttConfig.json");
  }
  rtc.printConf();
  delay(100);
}

bool CoolBoard::isConnected() {
  if (wifiManager.state() != WL_CONNECTED) {
    WARN_LOG("Wifi disconnected");
    return false;
  }

  if (mqtt.state() != 0) {
    WARN_LOG("MQTT disconnected");
    return false;
  }
  return true;
}

int CoolBoard::connect() {
  if (wifiManager.wifiCount > 0) {
    this->coolBoardLed.write(BLUE);
    if (wifiManager.connect() != 3) {
      this->coolBoardLed.blink(RED, 10);
    } else {
      this->coolBoardLed.blink(GREEN, 5);
    }
  } else {
    INFO_LOG("No configured Wifi access point, launching configuration portal");
    wifiManager.disconnect();
    delay(200);
    this->coolBoardLed.write(FUCHSIA);
    wifiManager.connectAP();
  }
  delay(100);

  if (wifiManager.state() == WL_CONNECTED) {
    if (mqtt.connect() != 0) {
      mqttProblem();
    }
    delay(100);
  }
  this->sendPublicIP();
  return (mqtt.state());
}

void CoolBoard::onLineMode() {
  if (mqtt.state() != 0) {
    WARN_LOG("Reconnecting MQTT...");
    if (mqtt.connect() != 0) {
      mqttProblem();
    }
    delay(200);
  }

  // send saved data if any, check once again if the MQTT connection is OK!
  if (fileSystem.isFileSaved() != 0 && this->isConnected()) {
    for (int i = 1; i <= SEND_MSG_BATCH; i++) {
      int lastLog = fileSystem.lastFileSaved();
      INFO_VAR("Sending saved log number:", lastLog);
      // only send a log if there is a log. 0 means zero files in the SPIFFS
      if (lastLog != 0) {
        mqtt.mqttLoop();
        String jsonData = "{\"state\":{\"reported\":";
        jsonData += fileSystem.getFileString(lastLog);
        jsonData += " }}";

        DEBUG_VAR("Saved JSON data:", jsonData);

        // delete file only if the message was published
        if (mqtt.publish(jsonData.c_str())) {
          fileSystem.deleteLogFile(lastLog);
          messageSent();
        } else {
          mqttProblem();
          break;
        }
      } else {
        mqttProblem();
        break;
      }
    }
    INFO_LOG("Saved data sent");
  }
  if (this->isConnected()) {
    INFO_LOG("Updating RTC...");
    rtc.update();
  }
  String data = this->boardData();
  data.setCharAt(data.lastIndexOf('}'), ',');
  INFO_LOG("Collecting sensor data...");
  data += this->readSensors();
  data.remove(data.lastIndexOf('{'), 1);

  if (this->manual == 0) {
    INFO_LOG("Actuators configuration: automatic");
    tmElements_t tm;
    tm = rtc.getTimeDate();

    if (jetpackActive) {
      DEBUG_LOG("Collecting Jetpack actuators data...");
      byte lastAction = jetPack.action;
      String jetpackStatus =
          jetPack.doAction(data.c_str(), int(tm.Hour), int(tm.Minute));
      if (lastAction != jetPack.action) {
        data += jetpackStatus;
        data.remove(data.lastIndexOf('{'), 1);
        data.setCharAt(data.indexOf('}'), ',');

        String jsonJetpackStatus = "{\"state\":{\"reported\":";

        jsonJetpackStatus += jetpackStatus;
        jsonJetpackStatus += " } }";
        mqtt.publish(jsonJetpackStatus.c_str());
      }
    }

    DEBUG_LOG("Collecting onboard actuator data...");
    bool lastActionB = digitalRead(onBoardActor.pin);
    String onBoardActorStatus =
        onBoardActor.doAction(data.c_str(), int(tm.Hour), int(tm.Minute));

    // send a message if actor has changed
    if (lastActionB != digitalRead(onBoardActor.pin)) {
      data += onBoardActorStatus;
      data.remove(data.lastIndexOf('{'), 1);
      data.setCharAt(data.indexOf('}'), ',');

      // send onBoardActorStatus over MQTT
      String jsonOnBoardActorStatus = "{\"state\":{\"reported\":";

      jsonOnBoardActorStatus += onBoardActorStatus;
      jsonOnBoardActorStatus += " } }";
      if (!mqtt.publish(jsonOnBoardActorStatus.c_str())) {
        mqttProblem();
      }
    }
  } else {
    INFO_LOG("Actuators configuration: manual");
  }

  delay(50);

  if (mqtt.state() != 0) {
    WARN_LOG("Reconnecting MQTT...");
    if (mqtt.connect() != 0) {
      mqttProblem();
    }
    delay(200);
  }

  mqtt.mqttLoop();
  String answer = mqtt.read();
  if (answer.length() > 0) {
    this->update(answer.c_str());
  }

  String jsonData = "{\"state\":{\"reported\":";
  jsonData += data;
  jsonData += " } }";

  // publish if we hit logInterval.
  if (this->shouldLog()) {
    INFO_LOG("Sending log over MQTT");
    if (!mqtt.publish(jsonData.c_str())) {
      fileSystem.saveMessageToFile(data.c_str());
      ERROR_LOG("MQTT publish failed! Data saved on SPIFFS");
      mqttProblem();
    } else {
      messageSent();
    }
    this->previousLogTime = millis();
  }

  this->startAP();
  if (this->sleepActive) {
    this->sleep(this->secondsToNextLog());
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

void CoolBoard::offLineMode() {
  INFO_LOG("COOL Board is in offline mode");
  String data = this->boardData();
  data.setCharAt(data.lastIndexOf('}'), ',');

  INFO_LOG("Collecting sensors data");
  data += this->readSensors();
  data.remove(data.lastIndexOf('{'), 1);

  tmElements_t tm;
  tm = rtc.getTimeDate();
  if (jetpackActive) {
    DEBUG_LOG("Jetpack is active, checking actuators rules");
    data += jetPack.doAction(data.c_str(), int(tm.Hour), int(tm.Minute));
    data.remove(data.lastIndexOf('{'), 1);
    data.setCharAt(data.indexOf('}'), ',');
    delay(50);
  }

  if (onBoardActor.actor.actif == true) {
    data += onBoardActor.doAction(data.c_str(), int(tm.Hour), int(tm.Minute));
    data.remove(data.lastIndexOf('{'), 1);
    data.setCharAt(data.indexOf('}'), ',');
  }

  if (this->shouldLog()) {
    if (this->saveAsJSON == 1) {
      fileSystem.saveMessageToFile(data.c_str());
      INFO_LOG("Saved data as JSON on SPIFFS");
    }
    if (this->saveAsCSV == 1) {
      fileSystem.saveSensorDataCSV(data.c_str());
      INFO_LOG("Saved data as CSV on SPIFFS");
    }
    this->previousLogTime = millis();
  }

  if (wifiManager.state() != WL_CONNECTED) {
    WARN_LOG("Wifi disconnected, retrying...");
    this->connect();
  }

  this->startAP();
  if (this->sleepActive) {
    this->sleep(this->secondsToNextLog());
  }
}

bool CoolBoard::config() {
  INFO_VAR("MAC address is ", WiFi.macAddress());

  fileSystem.begin();
  coolBoardLed.config();
  coolBoardLed.begin();
  delay(10);
  this->coolBoardLed.write(YELLOW);
  File configFile = SPIFFS.open("/coolBoardConfig.json", "r");

  if (!configFile) {
    ERROR_LOG("Failed to read /coolBoardConfig.json");
    spiffsProblem();
    return (false);
  }

  else {
    String data = configFile.readString();
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(data);

    if (!json.success()) {
      ERROR_LOG("Failed to parse COOL Board configuration as JSON");
      spiffsProblem();
      return (false);
    }

    else {
      if (json["logInterval"].success()) {
        this->logInterval = json["logInterval"].as<unsigned long>();
      } else {
        this->logInterval = this->logInterval;
      }
      json["logInterval"] = this->logInterval;
      if (json["ireneActive"].success()) {
        this->ireneActive = json["ireneActive"];
      } else {
        this->ireneActive = this->ireneActive;
      }
      json["ireneActive"] = this->ireneActive;
      if (json["jetpackActive"].success()) {
        this->jetpackActive = json["jetpackActive"];
      } else {
        this->jetpackActive = this->jetpackActive;
      }
      json["jetpackActive"] = this->jetpackActive;
      if (json["externalSensorsActive"].success()) {
        this->externalSensorsActive = json["externalSensorsActive"];
      } else {
        this->externalSensorsActive = this->externalSensorsActive;
      }
      json["externalSensorsActive"] = this->externalSensorsActive;
      if (json["sleepActive"].success()) {
        this->sleepActive = json["sleepActive"];
      } else {
        this->sleepActive = this->sleepActive;
      }
      json["sleepActive"] = this->sleepActive;
      if (json["manual"].success()) {
        this->manual = json["manual"].as<bool>();
      } else {
        this->manual = this->manual;
      }
      json["manual"] = this->manual;
      if (json["saveAsJSON"].success()) {
        this->saveAsJSON = json["saveAsJSON"].as<bool>();
      } else {
        this->saveAsJSON = this->saveAsJSON;
      }
      json["saveAsJSON"] = this->saveAsJSON;
      if (json["saveAsCSV"].success()) {
        this->saveAsCSV = json["saveAsCSV"].as<bool>();
      } else {
        this->saveAsCSV = this->saveAsCSV;
      }
      json["saveAsCSV"] = this->saveAsCSV;
      configFile.close();
      configFile = SPIFFS.open("/coolBoardConfig.json", "w");

      if (!configFile) {
        ERROR_LOG("failed to write to /coolBoardConfig.json");
        spiffsProblem();
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
  INFO_VAR("  Save as JSON active     =", this->saveAsJSON);
  INFO_VAR("  Save as CSV active      =", this->saveAsCSV);
}

void CoolBoard::update(const char *answer) {
  DEBUG_VAR("Update message:", answer);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(answer);
  JsonObject &stateDesired = root["state"];

  DEBUG_JSON("Update message JSON:", root);
  DEBUG_JSON("Desired state JSON:", stateDesired);

  if (stateDesired.success()) {
    DEBUG_LOG("Update message parsing: success");
    if (stateDesired["CoolBoard"]["manual"].success()) {
      this->manual = stateDesired["CoolBoard"]["manual"].as<bool>();
      INFO_VAR("Manual flag received:", this->manual);
    }

    this->coolBoardLed.strobe(BLUE, 0.5);
    String answerDesired;

    stateDesired.printTo(answerDesired);
    DEBUG_VAR("Desired state is:", answerDesired);

    // manual mode check
    if (this->manual) {
      INFO_LOG("Entering actuators manual mode");
      for (auto kv : stateDesired) {
        DEBUG_VAR("Writing to:", kv.key);
        DEBUG_VAR("State:", kv.value.as<bool>());

        if (strcmp(kv.key, "Act0") == 0) {
          jetPack.writeBit(0, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act1") == 0) {
          jetPack.writeBit(1, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act2") == 0) {
          jetPack.writeBit(2, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act3") == 0) {
          jetPack.writeBit(3, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act4") == 0) {
          jetPack.writeBit(4, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act5") == 0) {
          jetPack.writeBit(5, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act6") == 0) {
          jetPack.writeBit(6, kv.value.as<bool>());
        } else if (strcmp(kv.key, "Act7") == 0) {
          jetPack.writeBit(7, kv.value.as<bool>());
        } else if (strcmp(kv.key, "ActB") == 0) {
          onBoardActor.write(kv.value.as<bool>());
        }
      }
    }

    fileSystem.updateConfigFiles(answerDesired);

    String updateAnswer;
    String tempString;

    stateDesired.printTo(tempString);
    updateAnswer = "{\"state\":{\"reported\":";
    updateAnswer += tempString;
    updateAnswer += ",\"desired\":null}}";
    DEBUG_VAR("Preparing answer message: ", updateAnswer);
    mqtt.publish(updateAnswer.c_str());
    mqtt.mqttLoop();
    delay(10);
  } else {
    ERROR_LOG("Failed to parse update message");
  }
}

unsigned long CoolBoard::getLogInterval() { return (this->logInterval); }

String CoolBoard::readSensors() {
  String sensorsData;

  digitalWrite(ENABLE_I2C_PIN, HIGH);
  sensorsData = coolBoardSensors.read();

  if (externalSensorsActive) {
    sensorsData += externalSensors.read();
    sensorsData.setCharAt(sensorsData.lastIndexOf('}'), ',');
    sensorsData.setCharAt(sensorsData.lastIndexOf('{'), ',');
    sensorsData.remove(sensorsData.lastIndexOf('}'), 1);
    sensorsData.setCharAt(sensorsData.lastIndexOf(','), '}');
  }
  if (ireneActive) {
    sensorsData += irene3000.read();
    sensorsData.setCharAt(sensorsData.lastIndexOf('}'), ',');
    sensorsData.setCharAt(sensorsData.lastIndexOf('{'), ',');
    sensorsData.remove(sensorsData.lastIndexOf('}'), 1);
    sensorsData.setCharAt(sensorsData.lastIndexOf(','), '}');
  }

  DEBUG_VAR("Sensors data is:", sensorsData);
  this->coolBoardLed.blink(GREEN, 0.5);
  return (sensorsData);
}

String CoolBoard::boardData() {
  String tempMAC = WiFi.macAddress();
  tempMAC.replace(":", "");
  String boardJson = "{\"timestamp\":\"";
  boardJson += rtc.getESDate(); // "timestamp":"20yy-mm-ddThh:mm:ssZ"
  boardJson += "\",\"mac\":\"";
  boardJson += tempMAC;
  boardJson += "\"";
  if (this->isConnected()) {
    boardJson += ",\"wifiSignal\":";
    boardJson += WiFi.RSSI();
  }
  boardJson += "}";
  DEBUG_VAR("Board data JSON:", boardJson);
  return (boardJson);
}

void CoolBoard::sleep(unsigned long interval) {
  if (interval > 0) {
    INFO_VAR("Going to sleep for seconds:", interval);
    ESP.deepSleep((interval * 1000 * 1000), WAKE_RF_DEFAULT);
  }
}

bool CoolBoard::sendConfig(const char *moduleName, const char *filePath) {
  String result;

  // open file
  File configFile = SPIFFS.open(filePath, "r");
  if (!configFile) {
    ERROR_VAR("Failed to read configuration file:", filePath);
    return (false);
  } else {
    size_t size = configFile.size();

    // allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());

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
      mqtt.publish(result.c_str());
      mqtt.mqttLoop();
      return (true);
    }
  }
}

void CoolBoard::sendPublicIP() {
  if (this->isConnected()) {
    String tempStr = wifiManager.getExternalIP();
    if (tempStr.length() > 6) {
      String publicIP = "{\"state\":{\"reported\":{\"publicIP\":";
      publicIP += tempStr;
      publicIP += "}}}";
      INFO_VAR("Sending public IP address:", tempStr);
      mqtt.publish(publicIP.c_str());
    }
  }
}

void CoolBoard::startAP() {
  if (digitalRead(BOOTSTRAP_PIN) == LOW) {
    INFO_LOG("Bootstrap is in LOAD position, starting AP for further "
             "configuration...");
    wifiManager.disconnect();
    delay(200);
    this->coolBoardLed.write(FUCHSIA); // whiteish violet..
    wifiManager.connectAP();
    yield();
    delay(500);
    ESP.restart();
  }
}

void CoolBoard::mqttProblem() {
  this->coolBoardLed.blink(RED, 0.2);
  delay(200);
  this->coolBoardLed.blink(RED, 0.2);
  delay(200);
}

void CoolBoard::spiffsProblem() {
  this->coolBoardLed.write(RED);
  while (true) {
    yield();
  }
}

void CoolBoard::messageSent() {
  this->coolBoardLed.strobe(WHITE, 0.3);
}
