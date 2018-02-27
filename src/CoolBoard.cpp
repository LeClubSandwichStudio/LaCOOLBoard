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
#include <memory>

#include <ArduinoJson.h>
#include <Wire.h>

#include "CoolBoard.h"
#include "CoolConfig.h"
#include "CoolLog.h"

/**
 *  CoolBoard::CoolBoard():
 *  This Constructor is provided to start
 *  the I2C interface and init pins
 */
CoolBoard::CoolBoard() {
  Wire.begin(2, 14);      // I2C init
  pinMode(EnI2C, OUTPUT); // Declare I2C Enable pin
}

/**
 *  CoolBoard::begin():
 *  This method is provided to configure and
 *  start the used CoolKit Parts.
 *  It also starts the first connection try
 *  If Serial is enabled,it prints the configuration
 *  of the used parts.
 */
void CoolBoard::begin() {
  this->printConf();
  coolBoardLed.write(255, 128, 0); // orange
  this->initReadI2C();
  delay(100);

  // read RTC config on startup and treat off-grid if the `compile` flag is set
  rtc.config();
  rtc.offGrid();
  delay(200);

  coolBoardSensors.config();
  coolBoardSensors.begin();
  delay(200);

  onBoardActor.config();
  onBoardActor.begin();
  delay(100);

  wifiManager.config();
  wifiManager.begin();
  delay(100);

  mqtt.config();
  mqtt.begin();
  delay(100);

  coolBoardLed.printConf();
  coolBoardSensors.printConf();
  onBoardActor.printConf();
  wifiManager.printConf();
  mqtt.printConf();

  if (jetpackActive) {
    jetPack.config();
    jetPack.begin();

    DEBUG_LOG("Jetpack configuration:");
    jetPack.printConf();
    delay(100);
  }

  if (ireneActive) {
    irene3000.config();
    irene3000.startADC();
    coolBoardLed.write(128, 128, 128);
    delay(2000);

    int bValue = irene3000.readButton();

    if (bValue >= 65000) {
      bValue = 8800;
    }
    bValue = irene3000.readButton();
    if (bValue >= 65000) {
      bValue = 8800;
    }
    if (bValue < 2000) {
      coolBoardLed.write(255, 255, 255);
      delay(5000);
      coolBoardLed.write(0, 50, 0);
      bValue = irene3000.readButton();
      if (bValue >= 65000) {
        bValue = 0;
      }
      while (bValue > 2000) {
        bValue = irene3000.readButton();
        if (bValue >= 65000) {
          bValue = 8800;
        }
        delay(1000);
      }

      coolBoardLed.write(0, 255, 0);
      INFO_LOG("Calibrating the pH probe");
      INFO_LOG("pH 7 calibration for 25 seconds");
      delay(10000);

      irene3000.calibratepH7();
      delay(15000);

      irene3000.calibratepH7();
      coolBoardLed.write(50, 0, 0);
      bValue = irene3000.readButton();

      if (bValue >= 65000) {
        bValue = 8800;
      }

      while (bValue > 2000) {
        bValue = irene3000.readButton();
        if (bValue >= 65000) {
          bValue = 8800;
        }
        delay(1000);
      }

      coolBoardLed.write(255, 0, 0);
      INFO_LOG("pH 4 calibration for 25 seconds");
      delay(10000);

      irene3000.calibratepH4();
      delay(15000);

      irene3000.calibratepH4();
      irene3000.saveParams();

      coolBoardLed.write(50, 0, 50);
      bValue = irene3000.readButton();
      if (bValue >= 65000) {
        bValue = 8800;
      }
      while (bValue > 2000) {
        bValue = irene3000.readButton();
        if (bValue >= 65000) {
          bValue = 8800;
        }
        delay(1000);
      }
    }

    irene3000.printConf();
    delay(100);
  }

  if (externalSensorsActive) {
    externalSensors.config();
    externalSensors.begin();
    delay(100);

    externalSensors.printConf();
  }
  coolBoardLed.fadeOut(255, 128, 0, 0.5); // orange

  this->connect();
  delay(100);

  rtc.begin();

  if (this->sleepActive == 0) {
    coolBoardLed.strobe(255, 0, 230, 0.5); // shade of pink
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
  coolBoardLed.blink(0, 255, 0, 0.5); // green
}

/**
 *  CoolBoard::isConnected()
 *
 *  This method is provided to check
 *  if the card is connected to Wifi and MQTT
 *
 *  \return   0 : connected
 *    -1: Wifi Not Connected
 *    -2: MQTT Not Connected
 *
 */
int CoolBoard::isConnected() {
  if (wifiManager.state() != WL_CONNECTED) {
    WARN_LOG("Wifi disconnected");
    return (-1);
  }

  if (mqtt.state() != 0) {
    WARN_LOG("MQTT disconnected");
  }
  return (0);
}

/**
 *  CoolBoard::connect():
 *  This method is provided to manage the network
 *  connection and the mqtt connection.
 *
 *   \return mqtt client state
 */
int CoolBoard::connect() {
  coolBoardLed.write(0, 0, 255); // blue
  DEBUG_LOG("Starting Wifi");
  wifiManager.connect();
  delay(100);

  // only attempt MQTT connection if Wifi is connected
  if (wifiManager.state() == WL_CONNECTED) {
    mqtt.connect();
    delay(100);
    if (mqtt.state() != 0 && wifiManager.nomad == 1) {
      INFO_LOG("Known Wifi in range but no internet, launching configuration "
               "portal");
      wifiManager.disconnect();
      delay(200);
      coolBoardLed.write(255, 128, 255); // whiteish violet..
      wifiManager.connectAP();
    }
  }
  sendPublicIP();
  DEBUG_VAR("MQTT state is:", mqtt.state());
  delay(100);

  coolBoardLed.blink(0, 0, 255, 0.5); // blue
  return (mqtt.state());
}

/**
 *  CoolBoard::onLineMode():
 *  This method is provided to manage the online
 *  mode:
 *    - update clock
 *    - read sensor
 *    - do actions
 *    - publish data
 *    - read answer
 *    - update config
 */
void CoolBoard::onLineMode() {
  coolBoardLed.fadeIn(128, 255, 50, 0.5); // shade of green

  // check if we hit MQTT timeout
  if (mqtt.state() != 0) {
    INFO_LOG("Reconnecting MQTT...");
    mqtt.connect();
    delay(200);
  }

  data = "";
  answer = "";

  // send saved data if any, check once again if the MQTT connection is OK!
  if (fileSystem.isDataSaved() && isConnected() == 0 && mqtt.state() == 0) {
    coolBoardLed.fadeIn(128, 128, 255, 0.5); // shade of blue
    INFO_LOG("There is saved data on the SPIFFS, sending it over MQTT");
    coolBoardLed.strobe(128, 128, 255, 0.5); // shade of blue
    mqtt.mqttLoop();

    int size = 0;

    std::unique_ptr<String[]> savedData(
        std::move(fileSystem.getSensorSavedData(size)));

    for (int i = 0; i < size; ++i) {
      // format JSON
      String jsonData = "{\"state\":{\"reported\":";

      jsonData += savedData[i];
      jsonData += " } }";
      DEBUG_VAR("Saved message size is", size);
      DEBUG_VAR("Sending line #", i);
      DEBUG_VAR("JSON data:", jsonData);
      coolBoardLed.strobe(128, 128, 255, 0.5); // shade of blue
      mqtt.publish(jsonData.c_str());
      mqtt.mqttLoop();
      coolBoardLed.fadeOut(128, 128, 255, 0.5); // shade of blue
      yield();
    }
    DEBUG_LOG("Saved data sent");
  }
  coolBoardLed.blink(128, 255, 50, 0.5); // shade of green
  // update RTC
  INFO_LOG("Updating RTC...");
  rtc.update();
  // read board data
  data = this->boardData();
  // format JSON
  data.setCharAt(data.lastIndexOf('}'), ',');
  DEBUG_LOG("Collecting sensor data...");
  // read sensors data
  data += this->readSensors();
  // format JSON
  data.remove(data.lastIndexOf('{'), 1);
  coolBoardLed.fadeOut(245, 237, 27, 0.5); // shade of yellow
  // do actions
  if (this->manual == 0) {
    // get time for temporal actors
    tmElements_t tm;
    tm = rtc.getTimeDate();

    if (jetpackActive) {
      DEBUG_LOG("Jetpack is active");
      coolBoardLed.fade(100, 100, 150, 0.5); // dark shade of blue
      // save last value to see if state has changed
      byte lastAction = jetPack.action;
      String jetpackStatus =
          jetPack.doAction(data.c_str(), int(tm.Hour), int(tm.Minute));

      // send a message if an actor has changed
      if (lastAction != jetPack.action) {
        data += jetpackStatus;
        data.remove(data.lastIndexOf('{'), 1);
        data.setCharAt(data.indexOf('}'), ',');

        // send jetpackStatus over MQTT
        String jsonJetpackStatus = "{\"state\":{\"reported\":";

        jsonJetpackStatus += jetpackStatus;
        jsonJetpackStatus += " } }";
        mqtt.publish(jsonJetpackStatus.c_str());
      }
    }

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
      mqtt.publish(jsonOnBoardActorStatus.c_str());
    }
  } else if (this->manual == 1) {
    INFO_LOG("We are in manual mode");
    mqtt.mqttLoop();
    answer = mqtt.read();
    this->update(answer.c_str());
  }

  delay(50);

  coolBoardLed.fadeIn(128, 255, 50, 0.5); // shade of green

  String jsonData = "{\"state\":{\"reported\":";
  jsonData += data;
  jsonData += " } }";

  // MQTT client loop to handle incoming data
  mqtt.mqttLoop();

  coolBoardLed.blink(128, 255, 50, 0.5); // shade of green

  // read mqtt answer
  answer = mqtt.read();

  DEBUG_LOG("Checking if we received an MQTT message");
  DEBUG_VAR("Answer:", answer);

  coolBoardLed.fadeOut(128, 255, 50, 0.5); // shade of green

  // update configuration if needed
  this->update(answer.c_str());

  coolBoardLed.fadeIn(128, 255, 50, 0.5); // shade of green

  // check if we hit MQTT timeout
  if (mqtt.state() != 0) {
    INFO_LOG("Reconnecting MQTT...");
    mqtt.connect();
    delay(200);
  }

  // log interval is in seconds, logInterval*1000 = logInterval in ms.
  unsigned long logIntervalMillis = logInterval * 1000;
  unsigned long millisSinceLastLog = millis() - this->previousLogTime;

  // publish if we hit logInterval.
  // works in sleep mode since previousLogTime is 0 on startup.
  if (millisSinceLastLog >= logIntervalMillis || this->previousLogTime == 0) {
    if (this->sleepActive == 0) {
      coolBoardLed.strobe(255, 0, 230, 0.5); // shade of pink
      // logInterval in seconds
      if (!mqtt.publish(jsonData.c_str())) {
        fileSystem.saveSensorData(data.c_str());
        ERROR_LOG("MQTT publish failed! Data saved on SPIFFS");
      }
      mqtt.mqttLoop();
    } else {
      coolBoardLed.strobe(230, 255, 0, 0.5); // shade of yellow
      if (!mqtt.publish(jsonData.c_str())) {
        fileSystem.saveSensorData(data.c_str());
        ERROR_LOG("MQTT publish failed! Data saved on SPIFFS");
      }
      mqtt.mqttLoop();
      answer = mqtt.read();
      this->update(answer.c_str());
      // logInterval in seconds
      this->sleep(this->getLogInterval());
    }
    this->previousLogTime = millis();
  }

  coolBoardLed.fadeOut(128, 255, 50, 0.5); // shade of green
  mqtt.mqttLoop();
  // read mqtt answer
  answer = mqtt.read();
  this->update(answer.c_str());
  coolBoardLed.blink(128, 255, 50, 0.5); // shade of green
}

/**
 *  CoolBoard::offlineMode():
 *  This method is provided to manage the offLine
 *  mode:
 *    - read sensors
 *    - do actions
 *    - save data in the file system
 *    - if there is WiFi but no Internet : make data available over AP
 *    - if there is no connection : retry to connect
 */
void CoolBoard::offLineMode() {
  INFO_LOG("COOL Board is in offline mode");
  coolBoardLed.fade(51, 100, 50, 0.5); // dark shade of green
  coolBoardLed.fadeIn(245, 237, 27, 0.5); // shade of yellow
  coolBoardLed.blink(245, 237, 27, 0.5);  // shade of yellow
  // read user data
  data = this->boardData();
  // format json
  data.setCharAt(data.lastIndexOf('}'), ',');
  // read sensors data
  INFO_LOG("Collecting sensors data");
  data += this->readSensors();
  // format json correctly
  data.remove(data.lastIndexOf('{'), 1);
  coolBoardLed.fade(51, 100, 50, 0.5); // dark shade of green
  // get time for temporal actors
  tmElements_t tm;
  tm = rtc.getTimeDate();
  // do action

  if (jetpackActive) {
    DEBUG_LOG("Jetpack is active, checking actuators rules");
    coolBoardLed.fade(100, 100, 150, 0.5); // dark shade of blue
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

  coolBoardLed.fade(51, 100, 50, 0.5); // dark shade of green
  // log interval is in seconds, so logInterval * 1000 = logInterval in ms,
  // previousLogTime is 0 on startup so it works when no log was sent until
  // now and when sleep is active
  if ((millis() - (this->previousLogTime)) >= (logInterval * 1000) ||
      (this->previousLogTime == 0)) {
    // saving data in the file system as JSON
    if (this->saveAsJSON == 1) {
      fileSystem.saveSensorData(data.c_str());
      INFO_LOG("Saving data as JSON in memory: OK");
    }

    if (this->saveAsCSV == 1) {
      fileSystem.saveSensorDataCSV(data.c_str());
      INFO_LOG("Saving data as CSV in memory: OK");
    }
    this->previousLogTime = millis();
  }
  coolBoardLed.fadeOut(51, 100, 50, 0.5); // dark shade of green

  if (wifiManager.nomad == 0 || this->sleepActive == 0) {
    // case we have wifi but no internet
    if ((wifiManager.state() == WL_CONNECTED) && (mqtt.state() != 0)) {
      WARN_LOG("Wifi connected, no Internet. Launching AP for raw sensor "
               "data download and Wifi configuration");
      wifiManager.connectAP();
    }

    // case we have no connection at all
    if (wifiManager.state() != WL_CONNECTED) {
      WARN_LOG("Wifi disconnected, retrying...");
      this->connect(); // nomad case: just run wifiMulti
                       // normal case: run wifiMulti + AP
    }
  }

  if (this->sleepActive == 1) {
    this->sleep(this->getLogInterval());
  }
}

/**
 *  CoolBoard::config():
 *  This method is provided to configure
 *  the CoolBoard :  -log interval
 *      -irene3000 activated/deactivated
 *      -jetpack activated/deactivated
 *      -external Sensors activated/deactivated
 *      -mqtt server timeout
 *
 *  \return true if configuration is done,
 *  false otherwise
 */
bool CoolBoard::config() {
  INFO_VAR("MAC address is ", WiFi.macAddress());

  // open file system
  fileSystem.begin();

  // start the led
  coolBoardLed.config();
  coolBoardLed.begin();
  delay(10);
  coolBoardLed.blink(243, 171, 46, 0.5); // shade of orange

  CoolConfig config("/coolBoardConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read COOL board general config");
    coolBoardLed.blink(255, 0, 0, 0.5); // shade of red
    return (false);
  }
  JsonObject &json = config.get();

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

  if (json["saveAsJSON"].success()) {
    this->saveAsJSON = json["saveAsJSON"].as<bool>();
  }
  json["saveAsJSON"] = this->saveAsJSON;

  if (json["saveAsCSV"].success()) {
    this->saveAsCSV = json["saveAsCSV"].as<bool>();
  }
  json["saveAsCSV"] = this->saveAsCSV;

  if (!config.writeJsonToFile()) {
    ERROR_LOG("failed to write to /coolBoardConfig.json");
    coolBoardLed.blink(255, 0, 0, 0.5); // shade of red
    return (false);
  }
  coolBoardLed.blink(0, 255, 0, 0.5); // green blink
  return (true);
}

/**
 *  CoolBoard::printConf():
 *  This method is provided to print
 *  the configuration to the Serial
 *  Monitor.
 */
void CoolBoard::printConf() {
  INFO_LOG("COOL Board configuration:");
  INFO_VAR("  Log interval            =", this->logInterval);
  INFO_VAR("  Irene active            =", this->ireneActive);
  INFO_VAR("  Jetpack active          =", this->jetpackActive);
  INFO_VAR("  External sensors active =", this->externalSensorsActive);
  INFO_VAR("  Sleep active            =", this->sleepActive);
  INFO_VAR("  Manual active           =", this->manual);
  INFO_VAR("  Save as JSON active     =", this->saveAsJSON);
  INFO_VAR("  Save as CSV active      =", this->saveAsCSV);
}

/**
 *  CoolBoard::update(mqtt answer):
 *  This method is provided to handle the
 *  configuration update of the different parts
 */
void CoolBoard::update(const char *answer) {
  DEBUG_VAR("Update message:", answer);

  coolBoardLed.fadeIn(153, 76, 0, 0.5); // shade of brown

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(answer);
  JsonObject &stateDesired = root["state"];

  DEBUG_JSON("Root JSON:", root);
  DEBUG_JSON("Desired state JSON:", stateDesired);

  if (stateDesired["CoolBoard"]["manual"].success()) {
    this->manual = stateDesired["CoolBoard"]["manual"].as<bool>();
    DEBUG_VAR("Manual flag received:", this->manual);
  }

  if (stateDesired.success()) {
    String answerDesired;

    DEBUG_LOG("Update message parsing: success");
    stateDesired.printTo(answerDesired);
    DEBUG_VAR("Desired state is:", answerDesired);
    DEBUG_VAR("Manual flag value:", this->manual);

    // manual mode check
    if (this->manual == 1) {
      INFO_LOG("Entering actuators manual mode");
      // actuators JSON parsing
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

    // Irene calibration through update message
    if (stateDesired["calibration"].success()) {
      INFO_LOG("Starting Irene calibration from MQTT update");
      delay(2000);
      INFO_LOG("pH 7 calibration for 25 seconds");
      delay(10000);
      irene3000.calibratepH7();
      delay(15000);
      irene3000.calibratepH7();
      delay(1000);
      INFO_LOG("pH 7 calibration OK");
      INFO_LOG("pH 4 calibration for 25 seconds");
      delay(10000);
      irene3000.calibratepH4();
      delay(15000);
      irene3000.calibratepH4();
      delay(1000);
      INFO_LOG("pH 4 calibration OK");
      irene3000.saveParams();
    }

    // saving the new configuration
    fileSystem.updateConfigFiles(answerDesired);

    // answer the update msg
    // reported = received configuration
    // desired=null
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

    if (manual == 0) {
      // restart La COOL Board to apply the new configuration
      ESP.restart();
    }
  } else {
    DEBUG_LOG("Failed to parse update message (or no message received)");
  }

  coolBoardLed.strobe(153, 76, 0, 0.5);  // shade of brown
  coolBoardLed.fadeOut(153, 76, 0, 0.5); // shade of brown
}

/**
 *  CoolBoard::getLogInterval():
 *  This method is provided to get
 *  the log interval
 *
 *  \return interval value in s
 */
unsigned long CoolBoard::getLogInterval() {
  DEBUG_VAR("Log interval value:", this->logInterval);
  return (this->logInterval);
}

/**
 *  CoolBoard::readSensors():
 *  This method is provided to read and
 *  format all the sensors data in a
 *  single json.
 *
 *  \return  json string of all the sensors read.
 */
String CoolBoard::readSensors() {
  String sensorsData;

  coolBoardLed.fadeIn(128, 255, 0, 0.5); // light shade of green
  coolBoardLed.strobe(128, 255, 0, 0.5); // light shade of green

  this->initReadI2C();
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
  coolBoardLed.fadeOut(128, 255, 0, 0.5); // light shade of green
  return (sensorsData);
}

/**
 *  CoolBoard::initReadI2C():
 *  This method is provided to enable the I2C
 *  Interface.
 */
void CoolBoard::initReadI2C() {
  digitalWrite(EnI2C, HIGH); // HIGH = I2C enabled
}

/**
 *  CoolBoard::boardData():
 *  This method is provided to
 *  return the board's data.
 *
 *  \return json string of the data's data
 */
String CoolBoard::boardData() {
  String tempMAC = WiFi.macAddress();
  String boardJson = "{\"timestamp\":\"";

  tempMAC.replace(":", "");
  boardJson += rtc.getESDate(); // "timestamp":"20yy-mm-ddThh:mm:ssZ"
  boardJson += "\",\"mac\":\"";
  boardJson += tempMAC;
  boardJson += "\"";
  if (isConnected() == 0) {
    boardJson += ",\"wifiSignal\":";
    boardJson += WiFi.RSSI();
  }
  boardJson += "}";
  DEBUG_VAR("Board data JSON:", boardJson);
  return (boardJson);
}

/**
 *  CoolBoard::sleep(int interval):
 *  This method is provided to allow the
 *  board to enter deepSleep mode for
 *  a period of time equal to interval in s
 */
void CoolBoard::sleep(unsigned long interval) {
  INFO_VAR("Going to sleep for", interval);

  // interval is in seconds, interval*1000*1000 in µS
  ESP.deepSleep((interval * 1000 * 1000), WAKE_RF_DEFAULT);
}

/**
 *  CoolBoard::sendConfig( module Name,file path ):
 *  This method is provided to send
 *  all the configuration files
 *  over MQTT
 *
 *  \return true if successful, false if not
 */
bool CoolBoard::sendConfig(const char *moduleName, const char *filePath) {
  String result;
  CoolConfig config(filePath);

  if (!config.readFileAsJson()) {
    ERROR_VAR("Failed to read configuration file:", filePath);
    return (false);
  }
  JsonObject &json = config.get();
  String temporary;
  json.printTo(temporary);

  // format string
  result = "{\"state\":{\"reported\":{\"";
  result += moduleName;
  result += "\":";
  result += temporary;
  result += "} } }";

  mqtt.publish(result.c_str());
  mqtt.mqttLoop();
  return (true);
}

/**
 *  CoolBoard::sendPublicIP():
 *  This method is provided to send
 *  the public IP of a device to the
 *  CoolMenu over MQTT
 *
 *  \return true if successful, false if not
 */
bool CoolBoard::sendPublicIP() {
  if (isConnected() == 0) {
    String publicIP = "{\"state\":{\"reported\":{\"publicIP\":";
    publicIP += wifiManager.getExternalIP();
    publicIP += "}}}";
    mqtt.publish(publicIP.c_str());
  }
}
