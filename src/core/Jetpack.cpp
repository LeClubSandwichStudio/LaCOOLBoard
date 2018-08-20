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
    String actuatorName = String("Act") + String(i);
    JsonObject &act = json[actuatorName];
    if (act.success()) {
      // parsing actif key
      config.set<bool>(act, "actif", this->actuatorList[i].actif);
      // parsing temporal key
      config.set<bool>(act, "temporal", this->actuatorList[i].temporal);
      // parsing inverted key
      config.set<bool>(act, "inverted", this->actuatorList[i].inverted);
      // parsing low key
      config.setArray<int>(act, "low", 0, this->actuatorList[i].rangeLow);
      config.setArray<unsigned long>(act, "low", 1, this->actuatorList[i].timeLow);
      config.setArray<uint8_t>(act, "low", 2, this->actuatorList[i].hourLow);
      config.setArray<uint8_t>(act, "low", 3, this->actuatorList[i].minuteLow);
      // parsing high key
      config.setArray<int>(act, "high", 0, this->actuatorList[i].rangeHigh);
      config.setArray<unsigned long>(act, "high", 1, this->actuatorList[i].timeHigh);
      config.setArray<uint8_t>(act, "high", 2, this->actuatorList[i].hourHigh);
      config.setArray<uint8_t>(act, "high", 3, this->actuatorList[i].minuteHigh);
      // parsing type key
      config.setArray<String>(act, "type", 0, this->actuatorList[i].primaryType);
      config.setArray<String>(act, "type", 1, this->actuatorList[i].secondaryType);
    }
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
