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

#ifndef COOLBOARD_H
#define COOLBOARD_H

#include <Arduino.h>

#include "CoolBoardActuator.h"
#include "CoolBoardLed.h"
#include "CoolBoardSensors.h"
#include "CoolFileSystem.h"
#include "CoolTime.h"
#include "CoolWifi.h"
#include "ExternalSensors.h"
#include "Irene3000.h"
#include "Jetpack.h"
#include "CoolPubSubClient.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#define ENABLE_I2C_PIN 5
#define BOOTSTRAP_PIN 0
#define MIN_BAT_VOLTAGE 3.5
#define NOT_IN_CHARGING 1.8
#define LOW_POWER_SLEEP 300
#define MQTT_RETRIES 5
#define MAX_MQTT_RETRIES 15
#define MAX_SLEEP_TIME 3600

class CoolBoard {

public:
  void begin();
  bool config();
  void update(const char *answer);
  void loop();
  void connect();
  bool isConnected();
  unsigned long getLogInterval();
  void printConf();
  void sleep();
  void handleActuators(JsonObject &reported);
  void readSensors(JsonObject &reported);
  void readBoardData(JsonObject &reported);
  void sendSavedMessages();
  void sendAllConfig();
  void sendConfig(const char *moduleName, const char *filePath);
  void readPublicIP(JsonObject &reported);
  void clockProblem();
  void networkProblem();
  void spiffsProblem();
  void lowBattery();
  void powerCheck();
  void messageSent();
  unsigned long secondsToNextLog();
  bool shouldLog();
  void printMqttState(int state);
  void mqttConnect();
  bool mqttPublish(String data);
  bool mqttListen();
  void mqttCallback(char *topic, byte *payload, unsigned int length);
  void mqttsConfig();
  static int b64decode(String b64Text, uint8_t *output);
  void mqttsConvert(String cert);
  void updateFirmware(String firmwareVersion, String firmwareUrl, String firmwareUrlFingerprint);
  void tryFirmwareUpdate();
  void mqttLog(String data);

private:
  uint8_t mqttRetries = 0;
  CoolBoardSensors coolBoardSensors;
  CoolBoardLed coolBoardLed;
  CoolWifi *coolWifi = new CoolWifi;
  Jetpack jetPack;
  Irene3000 irene3000;
  ExternalSensors *externalSensors = new ExternalSensors;
  CoolBoardActuator coolBoardActuator;
  CoolPubSubClient *coolPubSubClient = new CoolPubSubClient;
  WiFiClientSecure *wifiClientSecure = new WiFiClientSecure;
  bool ireneActive = false;
  bool jetpackActive = false;
  bool externalSensorsActive = false;
  bool sleepActive = true;
  bool manual = false;
  unsigned long logInterval = 3600;
  unsigned long previousLogTime = 0;
  String mqttId;
  String mqttServer;
  String mqttInTopic;
  String mqttOutTopic;
};

#endif
