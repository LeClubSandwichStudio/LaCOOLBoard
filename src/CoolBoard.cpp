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

#include "CoolBoard.h"
#include "Arduino.h"
#include "ArduinoJson.h"
#include "FS.h"
#include <Wire.h>
#include <memory>

#define DEBUG 0
#define SEND_MSG_BATCH 10

/**
 *  CoolBoard::CoolBoard():
 *  This Constructor is provided to start
 *  the I2C interface and init pins
 */
CoolBoard::CoolBoard() {

#if DEBUG == 1
  Serial.println(F("Entering CoolBoard Constructor"));
  Serial.println();
#endif

  Wire.begin(2, 14);      // I2C init
  pinMode(enI2C, OUTPUT); // Declare I2C Enable pin
  pinMode(bootstrap, INPUT);  //Declare Bootstrap pin
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

  Serial.println(F("Starting La COOL Board..."));
#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.begin() "));
  Serial.println();
#endif

  delay(100);

  coolBoardLed.write(255, 128, 0); // orange
  this->initReadI2C();
  delay(100);

  // read RTC config on startup and treat off grid if the compile flag is set
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

#if DEBUG == 1
  coolBoardLed.printConf();
  coolBoardSensors.printConf();
  onBoardActor.printConf();
  wifiManager.printConf();
  mqtt.printConf();
#endif

  if (jetpackActive) {
    jetPack.config();
    jetPack.begin();

#if DEBUG == 1
    jetPack.printConf();
#endif

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
    Serial.print(F("Irene Button : "));
    Serial.println(bValue);
    bValue = irene3000.readButton();
    if (bValue >= 65000) {
      bValue = 8800;
    }
    if (bValue < 2000) {
      Serial.println("Enter Irene config.");
      coolBoardLed.write(255, 255, 255);
      delay(5000);
      coolBoardLed.write(0, 50, 0);
      bValue = irene3000.readButton();
      if (bValue >= 65000) {
        bValue = 0;
      }
      while (bValue > 2000) {
        bValue = irene3000.readButton();
        Serial.println(irene3000.readButton());
        if (bValue >= 65000) {
          bValue = 8800;
        }
        delay(1000);

        yield();
        Serial.println(F("while 1"));
      }

      coolBoardLed.write(0, 255, 0);
      Serial.println(F("calibrating the Ph probe "));
      Serial.println(F("ph7 calibration for 25 seconds"));
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
        Serial.println(irene3000.readButton());
        if (bValue >= 65000) {
          bValue = 8800;
        }
        delay(1000);

        yield();
        Serial.println(F("while 2"));
      }

      coolBoardLed.write(255, 0, 0);
      Serial.println(F("calibrating the Ph probe "));
      Serial.println(F("ph4 calibration for 25 seconds"));
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
        Serial.println(irene3000.readButton());
        if (bValue >= 65000) {
          bValue = 8800;
        }
        delay(1000);
        yield();
        Serial.println(F("while 3"));
      }
    }

#if DEBUG == 1
    irene3000.printConf();
#endif

    delay(100);
  }

  if (externalSensorsActive) {
    externalSensors.config();
    externalSensors.begin();
    delay(100);

#if DEBUG == 1
    externalSensors.printConf();
#endif

  }
  coolBoardLed.fadeOut(255, 128, 0, 0.5); // orange

  this->connect();
  delay(100);

  // RTC must be configured at startup to ensure RTC in off-grid configuration
  rtc.begin();

  if (this->sleepActive == 0) {
    coolBoardLed.strobe(255, 0, 230, 0.5); // shade of pink

    // send all config over mqtt
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

#if DEBUG == 1
  rtc.printConf();
#endif

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

#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.isConnected "));
  Serial.println();
#endif

  if (wifiManager.state() != WL_CONNECTED) {
    Serial.println(F("Wifi Not Connected"));

#if DEBUG == 1
    Serial.println(F("Wifi State is "));
    Serial.println(wifiManager.state());
#endif

    return (-1);
  }

  if (mqtt.state() != 0) {
    Serial.println(F("MQTT not Connected, reconnecting later..."));

#if DEBUG == 1
    Serial.println(F("mqtt state is :"));
    Serial.println(mqtt.state());
#endif
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

#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.connect "));
  Serial.println();
  Serial.println(F("Connecting the CoolBoard  "));
  delay(100);
#endif

  delay(10);
  if (wifiManager.wifiCount > 0){   //we have a WiFi in Memory -> blue light and connect
    coolBoardLed.write(0, 0, 127);

#if DEBUG == 1
    Serial.println(F("Launching CoolWifi"));
    Serial.println();
#endif
  
    if (wifiManager.connect() != 3) {
      coolBoardLed.blink(255, 0, 0, 1);    //Light the led in RED to say that you are not happy
    } else {
      coolBoardLed.blink(0, 255, 255, 0.8);  
    }

  } else if (wifiManager.wifiCount == 0) {    //we have no Memory -> violet light and start AP

#if DEBUG == 1
    Serial.println(F("no WiFi in memory, launching AP for configuration"));
    Serial.println();
#endif
    
    wifiManager.disconnect();
    delay(200);
    coolBoardLed.write(255, 128, 255); // whiteish violet..
    wifiManager.connectAP();
  }


  delay(100);

  // only attempt MQTT connection if Wifi is connected
  if (wifiManager.state() == WL_CONNECTED) {

#if DEBUG == 1
    Serial.println(F("Launching mqtt.connect()"));
    Serial.println();
#endif

    // blink twice in red if there is no mqtt
    if (mqtt.connect() != 0) {
      mqttProblem();
    }
    delay(100);

  }
  sendPublicIP();

#if DEBUG == 1
  Serial.println(F("mqtt state is :"));
  Serial.println(mqtt.state());
  Serial.println();
  delay(100);
#endif

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

#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.onLineMode() "));
  Serial.println();
#endif

#if DEBUG == 0
  Serial.println(F("CoolBoard is in Online Mode"));
#endif

  // check if we hit MQTT timeout
  if (mqtt.state() != 0) {
    Serial.println(F("reconnecting MQTT..."));

    if (mqtt.connect() != 0) {
      mqttProblem();
    }
    delay(200);
  }

  data = "";
  answer = "";

  // send saved data if any, check once again if the MQTT connection is OK!
  if (fileSystem.isFileSaved() != 0 && isConnected() == 0 && mqtt.state() == 0) {
    for (int i = 1; i <= SEND_MSG_BATCH; i++) {
      int lastLog = fileSystem.lastFileSaved();
      // only send a log IF there is a log. 0 means zero files in the SPIFFS
      if (lastLog != 0) {
        mqtt.mqttLoop();
        String jsonData = "{\"state\":{\"reported\":";
        jsonData += fileSystem.getFileString(lastLog);
        jsonData += " } }";
        coolBoardLed.strobe(128, 128, 255, 0.5); // shade of blue

#if DEBUG == 1
        Serial.print("Formatted Data from file : ");
        Serial.println(jsonData);
#endif
        //delete file only if the message was published
        if (mqtt.publish(jsonData.c_str())) {
          fileSystem.deleteLogFile(lastLog); 
        } else {
          mqttProblem();
          break;     // just break
        }
      } else {
        mqttProblem();
        break;       // don't insist if you got a bad connection, it's not your day ;)
      }
    }

#if DEBUG == 1
    Serial.println(F("Saved data sent "));
    Serial.println();
#endif
  }

  coolBoardLed.blink(128, 255, 50, 0.5); // shade of green

  // update RTC only if wifi is connected
  if (isConnected() == 0) {
    Serial.println(F("Re-checking RTC..."));
    rtc.update();
  }

  // read board data
  data = this->boardData();

  // format JSON
  data.setCharAt(data.lastIndexOf('}'), ',');

#if DEBUG == 1
  Serial.println(F("Collecting sensors data "));
  Serial.println();
#endif

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

#if DEBUG == 1
      Serial.println(F("jetpack is Active "));
      Serial.println();
#endif

      Serial.println(F("jetpack is active"));
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
      if (!mqtt.publish(jsonOnBoardActorStatus.c_str())) {
        mqttProblem();
      }
    }
  } else if (this->manual == 1) {
    Serial.println(F("we are in manual mode"));
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

#if DEBUG == 1
  Serial.println(F("checking if there's an MQTT message "));
  Serial.println(F("answer is : "));
  Serial.println(answer);
  Serial.println();
#endif

  coolBoardLed.fadeOut(128, 255, 50, 0.5); // shade of green

  // update configuration if needed
  this->update(answer.c_str());

  coolBoardLed.fadeIn(128, 255, 50, 0.5); // shade of green

  // check if we hit MQTT timeout
  if (mqtt.state() != 0) {
    Serial.println(F("reconnecting MQTT..."));
    if (mqtt.connect() != 0) {
      mqttProblem();
    }
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
        fileSystem.saveMessageToFile(data.c_str());
        Serial.println(F("MQTT publish failed! Saved Data as JSON in Memory : OK"));
        mqttProblem();
      }
      mqtt.mqttLoop();
      
    } else {
      coolBoardLed.strobe(230, 255, 0, 0.5); // shade of yellow

      if (!mqtt.publish(jsonData.c_str())) {
        fileSystem.saveMessageToFile(data.c_str());
        Serial.println(F("MQTT publish failed! Saving Data as JSON in Memory : OK"));
        mqttProblem();
      }

      mqtt.mqttLoop();
      answer = mqtt.read();
      this->update(answer.c_str());
      startAP();  // check if the user wants to start the AP for configuration
      
      this->sleep(this->getLogInterval());  // logInterval in seconds
    }
    this->previousLogTime = millis();
  }
  //If we got here we must be in Farm Mode
  startAP();  // check if the user wants to start the AP for configuration
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
  coolBoardLed.fade(51, 100, 50, 0.5); // dark shade of green

  Serial.println(F("CoolBoard is in Offline Mode"));

  coolBoardLed.fadeIn(245, 237, 27, 0.5); // shade of yellow
  coolBoardLed.blink(245, 237, 27, 0.5);  // shade of yellow

  // reading user data
  data = this->boardData();

  // formatting json
  data.setCharAt(data.lastIndexOf('}'), ',');

  // read sensors data
  Serial.println(F("Collecting sensors data "));
  Serial.println();
  data += this->readSensors();

  // formatting json correctly
  data.remove(data.lastIndexOf('{'), 1);
  coolBoardLed.fade(51, 100, 50, 0.5); // dark shade of green

  // get Time for temporal actors
  tmElements_t tm;
  tm = rtc.getTimeDate();

  // do action
  if (jetpackActive) {

#if DEBUG == 1
    Serial.println(F("jetpack is Active "));
    Serial.println(F("jetpack doing action "));
    Serial.println();
#endif

    coolBoardLed.fade(100, 100, 150, 0.5); // dark shade of blue
    data += jetPack.doAction(data.c_str(), int(tm.Hour),
                             int(tm.Minute)); //{..,..,..}{..,..,..}
    data.remove(data.lastIndexOf('{'), 1);    //{..,..,..}..,..,..}
    data.setCharAt(data.indexOf('}'), ',');   //{..,..,..,..,..,..}
  }

  delay(50);

  if (onBoardActor.actor.actif == true) {
    data += onBoardActor.doAction(data.c_str(), int(tm.Hour), int(tm.Minute));
    data.remove(data.lastIndexOf('{'), 1);
    data.setCharAt(data.indexOf('}'), ',');
  }

  coolBoardLed.fade(51, 100, 50, 0.5); // dark shade of green
  // log interval is passed in seconds, logInterval*1000 = logInterval in ms, if
  // no log was sent till now (so it also works when sleep is active since
  // previousLogTime is 0 on startup
  if ((millis() - (this->previousLogTime)) >= (logInterval * 1000) ||
      (this->previousLogTime == 0)) {
    // saving data in the file system as JSON
    if (this->saveAsJSON == 1) {
      fileSystem.saveMessageToFile(data.c_str());
      Serial.println(F("saving Data as JSON in Memory: OK"));
    }

    if (this->saveAsCSV == 1) {
      fileSystem.saveSensorDataCSV(data.c_str());
      Serial.println(F("saving Data as CSV in Memory: OK"));
    }
    this->previousLogTime = millis();
  }

  coolBoardLed.fadeOut(51, 100, 50, 0.5); // dark shade of green

  // case we have no connection at all
  if (wifiManager.state() != WL_CONNECTED) {
    Serial.println(F("there is no WiFi..."));

#if DEBUG == 1
    Serial.println(F("retrying to connect"));
#endif

    this->connect();
  }

  startAP();  // check if the user wants to start the AP for configuration
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
  yield();

  Serial.println();
  Serial.println("[" + WiFi.macAddress() + "]");

#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.config() "));
  Serial.println();
#endif

#if DEBUG == 0
  Serial.println();
  Serial.println(F("Loading configuration for this CoolBoard..."));
#endif

  // open file system
  fileSystem.begin();

  // start the led
  coolBoardLed.config();
  coolBoardLed.begin();
  delay(10);
  coolBoardLed.blink(243, 171, 46, 0.5); // shade of orange

  // open configuration file
  File configFile = SPIFFS.open("/coolBoardConfig.json", "r");

  if (!configFile) {
    Serial.println(F("failed to read /coolBoardConfig.json"));
    coolBoardLed.blink(255, 0, 0, 0.5); // shade of red
    return (false);
  }

  else {
    size_t size = configFile.size();

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());

    if (!json.success()) {
      Serial.println(F("failed to parse CoolBoard Config json object"));
      coolBoardLed.blink(255, 0, 0, 0.5); // shade of red
      return (false);
    }

    else {

#if DEBUG == 1
      Serial.println(F("configuration json : "));
      json.printTo(Serial);
      Serial.println();
      Serial.print(F("jsonBuffer size : "));
      Serial.print(jsonBuffer.size());
      Serial.println();
#endif

      // parsing logInterval key
      if (json["logInterval"].success()) {
        this->logInterval = json["logInterval"].as<unsigned long>();
      } else {
        this->logInterval = this->logInterval;
      }
      json["logInterval"] = this->logInterval;

      // parsing ireneActive key
      if (json["ireneActive"].success()) {
        this->ireneActive = json["ireneActive"];
      } else {
        this->ireneActive = this->ireneActive;
      }
      json["ireneActive"] = this->ireneActive;

      // parsing jetpackActive key
      if (json["jetpackActive"].success()) {
        this->jetpackActive = json["jetpackActive"];
      } else {
        this->jetpackActive = this->jetpackActive;
      }
      json["jetpackActive"] = this->jetpackActive;

      // parsing externalSensorsActive key
      if (json["externalSensorsActive"].success()) {
        this->externalSensorsActive = json["externalSensorsActive"];
      } else {
        this->externalSensorsActive = this->externalSensorsActive;
      }
      json["externalSensorsActive"] = this->externalSensorsActive;

      // parsing sleepActive key
      if (json["sleepActive"].success()) {
        this->sleepActive = json["sleepActive"];
      } else {
        this->sleepActive = this->sleepActive;
      }
      json["sleepActive"] = this->sleepActive;

      // parsing manual key
      if (json["manual"].success()) {
        this->manual = json["manual"].as<bool>();
      } else {
        this->manual = this->manual;
      }
      json["manual"] = this->manual;

      // parsing saveAsJSON key
      if (json["saveAsJSON"].success()) {
        this->saveAsJSON = json["saveAsJSON"].as<bool>();
      } else {
        this->saveAsJSON = this->saveAsJSON;
      }
      json["saveAsJSON"] = this->saveAsJSON;

      // parsing saveAsCSV key
      if (json["saveAsCSV"].success()) {
        this->saveAsCSV = json["saveAsCSV"].as<bool>();
      } else {
        this->saveAsCSV = this->saveAsCSV;
      }
      json["saveAsCSV"] = this->saveAsCSV;

      // saving the current/correct configuration
      configFile.close();
      configFile = SPIFFS.open("/coolBoardConfig.json", "w");

      if (!configFile) {
        Serial.println(F("failed to write to /coolBoardConfig.json"));
        Serial.println();
        coolBoardLed.blink(255, 0, 0, 0.5); // shade of red
        return (false);
      }

      json.printTo(configFile);
      configFile.close();
      coolBoardLed.blink(0, 255, 0, 0.5); // green blink

#if DEBUG == 0
      Serial.println(F("Configuration loaded : OK"));
      Serial.println();
#endif

      return (true);
    }
  }

  coolBoardLed.fadeOut(243, 171, 46, 0.5); // shade of orange
}

/**
 *  CoolBoard::printConf():
 *  This method is provided to print
 *  the configuration to the Serial
 *  Monitor.
 */
void CoolBoard::printConf() {

#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.printConf() "));
  Serial.println();
#endif

  Serial.println(F("Printing Cool Board Configuration"));
  Serial.print(F("log interval               :"));
  Serial.println(this->logInterval);
  Serial.print(F("irene active               :"));
  Serial.println(this->ireneActive);
  Serial.print(F("jetpack active            :"));
  Serial.println(this->jetpackActive);
  Serial.print(F("external sensors active   :"));
  Serial.println(this->externalSensorsActive);
  Serial.print(F("sleep active              :"));
  Serial.println(this->sleepActive);
  Serial.print(F("manual active             :"));
  Serial.println(this->manual);
  Serial.print(F("saveAsJSON active         :"));
  Serial.println(this->saveAsJSON);
  Serial.print(F("saveAsCSV active          :"));
  Serial.println(this->saveAsCSV);
  Serial.println();
}

/**
 *  CoolBoard::update(mqtt answer):
 *  This method is provided to handle the
 *  configuration update of the different parts
 */
void CoolBoard::update(const char *answer) {
  coolBoardLed.fadeIn(153, 76, 0, 0.5); // shade of brown

#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.update() "));
  Serial.println();
  Serial.println(F("message is : "));
  Serial.println(answer);
  Serial.println();
#endif

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(answer);
  JsonObject &stateDesired = root["state"];

#if DEBUG == 1
  Serial.println(F("root json : "));
  root.printTo(Serial);
  Serial.println();
  Serial.println(F("stateDesired json : "));
  stateDesired.printTo(Serial);
  Serial.println();
  Serial.print(F("jsonBuffer size : "));
  Serial.println(jsonBuffer.size());
#endif

  if (stateDesired["CoolBoard"]["manual"].success()) {
    this->manual = stateDesired["CoolBoard"]["manual"].as<bool>();

#if DEBUG == 1
    Serial.println("Manual Flag received");
    Serial.println(this->manual);
#endif
  }

  if (stateDesired.success()) {

#if DEBUG == 1
    Serial.println(F("update message parsing : success"));
    Serial.println();
#endif

    String answerDesired;

    stateDesired.printTo(answerDesired);

#if DEBUG == 1
    Serial.println(F("update is ok "));
    Serial.println(F("desired update is : "));
    Serial.println(answerDesired);
    Serial.println("json size is : ");
    Serial.println(jsonBuffer.size());
    Serial.println();
    Serial.print("ManualFlag : ");
    Serial.println(this->manual);
#endif

    // manual mode check
    if (this->manual == 1) {
      Serial.println();
      Serial.print("Enter Manual Actors...");
      // json parse for actors
      for (auto kv : stateDesired) {

#if DEBUG == 1
        Serial.print(F("writing to "));
        Serial.println(kv.key);
        Serial.print(F("state : "));
        Serial.println(kv.value.as<bool>());
#endif

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
      Serial.println(F("Starting Irene Calibration From MQTT Update"));
      Serial.println();
      delay(2000);
      Serial.println(F("ph7 calibration for 25 seconds"));
      delay(10000);
      irene3000.calibratepH7();
      delay(15000);
      irene3000.calibratepH7();
      delay(1000);
      Serial.println(F("ph 7 calibration OK"));
      Serial.println();
      Serial.println(F("ph 4 calibration for 25 seconds"));
      delay(10000);
      irene3000.calibratepH4();
      delay(15000);
      irene3000.calibratepH4();
      delay(1000);
      Serial.println(F("ph 4 calibration OK"));
      Serial.println();
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

#if DEBUG == 1
    Serial.println(F("preparing answer message "));
    Serial.println();
    Serial.println(F("updateAnswer : "));
    Serial.println(updateAnswer);
#endif

    mqtt.publish(updateAnswer.c_str());
    mqtt.mqttLoop();
    delay(10);

    if (manual == 0) {
      // restart La COOL Board to apply the new configuration
      ESP.restart();
    }
  } else {

#if DEBUG == 1
    Serial.println(
        F("Failed to parse update message( OR no message received )"));
    Serial.println();
#endif
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

#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.getLogInterval() "));
  Serial.println();
  Serial.println(F("log Interval is :"));
  Serial.println(logInterval);
  Serial.println();
#endif

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

#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.readSensors()"));
  Serial.println();
#endif

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

#if DEBUG == 1
  Serial.println();
  Serial.println(F("sensors data is "));
  Serial.println(sensorsData);
  Serial.println();
#endif

  coolBoardLed.fadeOut(128, 255, 0, 0.5); // light shade of green
  return (sensorsData);
}

/**
 *  CoolBoard::initReadI2C():
 *  This method is provided to enable the I2C
 *  Interface.
 */
void CoolBoard::initReadI2C() {

#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.initReadI2C()"));
  Serial.println();
#endif

  digitalWrite(enI2C, HIGH); // HIGH = I2C enabled
}

/**
 *  CoolBoard::boardData():
 *  This method is provided to
 *  return the board's data.
 *
 *  \return json string of the data's data
 */
String CoolBoard::boardData() {

#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.boardData() "));
  Serial.println();
#endif

  String tempMAC = WiFi.macAddress();
  tempMAC.replace(":", "");
  String boardJson = "{\"timestamp\":\"";
  boardJson += rtc.getESDate(); // "timestamp":"20yy-mm-ddThh:mm:ssZ"
  boardJson += "\",\"mac\":\"";
  boardJson += tempMAC;
  boardJson += "\"";
  //only send signal strenght if you got a existing wifi connection, logic, isn't it...
  if (isConnected() == 0) {
    boardJson += ",\"wifiSignal\":";
    boardJson += WiFi.RSSI();
  }
  boardJson += "}";

#if DEBUG == 1
  Serial.println(F("boardData is : "));
  Serial.println(boardJson);
  Serial.println();
#endif

  return (boardJson);
}

/**
 *  CoolBoard::sleep(int interval):
 *  This method is provided to allow the
 *  board to enter deepSleep mode for
 *  a period of time equal to interval in s
 */
void CoolBoard::sleep(unsigned long interval) {

  Serial.println(F("Entering CoolBoard.sleep()"));
  Serial.print(F("going to sleep for "));
  Serial.print(interval);
  Serial.println(F("s"));
  Serial.println();

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

#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.sendConfig()"));
#endif

  String result;

  // open file
  File configFile = SPIFFS.open(filePath, "r");

  if (!configFile) {
    Serial.print(F("failed to read "));
    Serial.println(filePath);
    return (false);
  } else {
    size_t size = configFile.size();

    // allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());

    if (!json.success()) {
      Serial.println(F("failed to parse json object"));
      return (false);
    }

    else {

#if DEBUG == 1
      Serial.println(F("configuration json : "));
      json.printTo(Serial);
      Serial.println();
      Serial.print(F("jsonBuffer size : "));
      Serial.print(jsonBuffer.size());
      Serial.println();
#endif

      String temporary;

      // JSON to string
      json.printTo(temporary);

      // format string
      result = "{\"state\":{\"reported\":{\"";
      result += moduleName;
      result += "\":";
      result += temporary;
      result += "} } }";

      // send over MQTT
      mqtt.publish(result.c_str());
      mqtt.mqttLoop();
      return (true);
    }
  }
}

/**
 *  CoolBoard::sendPublicIP():
 *  This method is provided to send
 *  the public IP of a device to the 
 *  CoolMenu over MQTT
 *
 *  \return true if successful, false if not
 */
bool CoolBoard::sendPublicIP()
{
#if DEBUG == 1
  Serial.println(F("Entering CoolBoard.sendConfig()"));
#endif
  //only send public if you got a existing wifi connection, logic, isn't it...
  if (isConnected() == 0) {
    
  String tempStr = wifiManager.getExternalIP();

#if DEBUG == 1
    Serial.printf ("External IP lenght : %ld \n", tempStr.length());
#endif

    if (tempStr.length() > 6) { // why 6? because a public IP should at least have 7 signs and look like this : 1.2.3.4
      String publicIP = "{\"state\":{\"reported\":{\"publicIP\":";
      publicIP += tempStr;
      publicIP += "}}}";

#if DEBUG == 1
      Serial.println();
      Serial.print("sending external IP : ");
      Serial.println(publicIP);
#endif

      mqtt.publish( publicIP.c_str() );
    }
  }
}

/**
 *  CoolBoard::startAP():
 *  This method is provided to check if the user 
 *  used the run/load switch to start the AP
 *  for further configuration/download
 *
 */
void CoolBoard::startAP() {
#if DEBUG == 1
  Serial.println(F("Entering Coolboard.startAP"));
  Serial.print(F("Bootstrap Switch : "));
  Serial.println(digitalRead(bootstrap));
#endif
  
  if (digitalRead(bootstrap) == LOW) {
    Serial.println(F("Bootstrap in load position, starting AP for further configuration..."));
    wifiManager.disconnect();
    delay(200);
    coolBoardLed.write(255, 128, 255); // whiteish violet..
    wifiManager.connectAP();
    yield();
    delay(500);
    ESP.restart();
  }
}

/**
 *  CoolBoard::mqttProblem():
 *  This method is provided to signal the user 
 *  a problem with the mqtt connection.
 *
 */
void CoolBoard::mqttProblem() {
  coolBoardLed.blink(255, 0, 0, 0.2);
  delay(200);
  coolBoardLed.blink(255, 0, 0, 0.2);
  delay(200);
}