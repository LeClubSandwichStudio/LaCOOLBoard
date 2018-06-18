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

#include "CoolConfig.h"
#include "CoolLog.h"
#include "Jetpack.h"

void Jetpack::begin() {
  pinMode(JETPACK_I2C_ENABLE_PIN, OUTPUT);
  pinMode(JETPACK_DATA_PIN, OUTPUT);
  pinMode(JETPACK_CLOCK_PIN, OUTPUT);
}

void Jetpack::write(byte action) {
  DEBUG_VAR("Setting Jetpack actuators to:", action);
  this->action = action;
  digitalWrite(JETPACK_I2C_ENABLE_PIN, LOW);
  shiftOut(JETPACK_DATA_PIN, JETPACK_CLOCK_PIN, MSBFIRST, this->action);
  digitalWrite(JETPACK_I2C_ENABLE_PIN, HIGH);
}

void Jetpack::writeBit(byte pin, bool state) {
  DEBUG_VAR("Setting Jetpack actuator #:", pin);
  DEBUG_VAR("To state:", state);
  bitWrite(this->action, pin, state);
  digitalWrite(JETPACK_I2C_ENABLE_PIN, LOW);
  shiftOut(JETPACK_DATA_PIN, JETPACK_CLOCK_PIN, MSBFIRST, this->action);
  digitalWrite(JETPACK_I2C_ENABLE_PIN, HIGH);
}

void Jetpack::doAction(JsonObject &root, int hour, int minute) {
  bool state = false;

  for (int pin = 0; pin < 8; pin++) {
    state = this->actuatorList[pin].doAction(root, hour, minute);
    root[String("Act") + String(pin)] = state;
    bitWrite(this->action, pin, state);
  }
  this->write(this->action);
}

bool Jetpack::config() {
  CoolConfig config("/jetPackConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read Jetpack configuration");
    return (false);
  }
  JsonObject &json = config.get();
  for (int i = 0; i < 8; i++) {
    if (json[String("Act") + String(i)].success()) {
      // parsing actif key
      if (json[String("Act") + String(i)]["actif"].success()) {
        this->actuatorList[i].actif = json[String("Act") + String(i)]["actif"];
      }
      json[String("Act") + String(i)]["actif"] = this->actuatorList[i].actif;

      // parsing temporal key
      if (json[String("Act") + String(i)]["temporal"].success()) {
        this->actuatorList[i].temporal =
            json[String("Act") + String(i)]["temporal"];
      }
      json[String("Act") + String(i)]["temporal"] =
          this->actuatorList[i].temporal;

      // parsing inverted key
      if (json[String("Act") + String(i)]["inverted"].success()) {
        this->actuatorList[i].inverted =
            json[String("Act") + String(i)]["inverted"];
      }
      json[String("Act") + String(i)]["inverted"] =
          this->actuatorList[i].inverted;

      // parsing inverted key
      if (json[String("Act") + String(i)]["inverted"].success()) {
        this->actuatorList[i].inverted =
            json[String("Act") + String(i)]["inverted"];
      }
      json[String("Act") + String(i)]["inverted"] =
          this->actuatorList[i].inverted;

      // parsing low key
      if (json[String("Act") + String(i)]["low"].success()) {
        this->actuatorList[i].rangeLow =
            json[String("Act") + String(i)]["low"][0];
        this->actuatorList[i].timeLow =
            json[String("Act") + String(i)]["low"][1];
        this->actuatorList[i].hourLow =
            json[String("Act") + String(i)]["low"][2];
        this->actuatorList[i].minuteLow =
            json[String("Act") + String(i)]["low"][3];
      }
      json[String("Act") + String(i)]["low"][0] =
          this->actuatorList[i].rangeLow;
      json[String("Act") + String(i)]["low"][1] = this->actuatorList[i].timeLow;
      json[String("Act") + String(i)]["low"][2] = this->actuatorList[i].hourLow;
      json[String("Act") + String(i)]["low"][3] =
          this->actuatorList[i].minuteLow;

      // parsing high key
      if (json[String("Act") + String(i)]["high"].success()) {
        this->actuatorList[i].rangeHigh =
            json[String("Act") + String(i)]["high"][0];
        this->actuatorList[i].timeHigh =
            json[String("Act") + String(i)]["high"][1];
        this->actuatorList[i].hourHigh =
            json[String("Act") + String(i)]["high"][2];
        this->actuatorList[i].minuteHigh =
            json[String("Act") + String(i)]["high"][3];
      }
      json[String("Act") + String(i)]["high"][0] =
          this->actuatorList[i].rangeHigh;
      json[String("Act") + String(i)]["high"][1] =
          this->actuatorList[i].timeHigh;
      json[String("Act") + String(i)]["high"][2] =
          this->actuatorList[i].hourHigh;
      json[String("Act") + String(i)]["high"][3] =
          this->actuatorList[i].minuteHigh;

      // parsing type key
      if (json[String("Act") + String(i)]["type"].success()) {
        this->actuatorList[i].primaryType =
            json[String("Act") + String(i)]["type"][0].as<String>();
        this->actuatorList[i].secondaryType =
            json[String("Act") + String(i)]["type"][1].as<String>();
      }
      json[String("Act") + String(i)]["type"][0] =
          this->actuatorList[i].primaryType;
      json[String("Act") + String(i)]["type"][1] =
          this->actuatorList[i].secondaryType;
    }
    json[String("Act") + String(i)]["actif"] = this->actuatorList[i].actif;
    json[String("Act") + String(i)]["temporal"] =
        this->actuatorList[i].temporal;
    json[String("Act") + String(i)]["inverted"] =
        this->actuatorList[i].inverted;
    json[String("Act") + String(i)]["low"][0] = this->actuatorList[i].rangeLow;
    json[String("Act") + String(i)]["low"][1] = this->actuatorList[i].timeLow;
    json[String("Act") + String(i)]["low"][2] = this->actuatorList[i].hourLow;
    json[String("Act") + String(i)]["low"][3] = this->actuatorList[i].minuteLow;
    json[String("Act") + String(i)]["high"][0] =
        this->actuatorList[i].rangeHigh;
    json[String("Act") + String(i)]["high"][1] = this->actuatorList[i].timeHigh;
    json[String("Act") + String(i)]["high"][2] = this->actuatorList[i].hourHigh;
    json[String("Act") + String(i)]["high"][3] =
        this->actuatorList[i].minuteHigh;
    json[String("Act") + String(i)]["type"][0] =
        this->actuatorList[i].primaryType;
    json[String("Act") + String(i)]["type"][1] =
        this->actuatorList[i].secondaryType;
  }
  INFO_LOG("Jetpack configuration loaded");
  return (true);
}

void Jetpack::printConf() {
  INFO_LOG("Jetpack configuration");

  for (int i = 0; i < 8; i++) {
    INFO_VAR("Actuator #", i);
    this->actuatorList[i].printConf();
  }
}
