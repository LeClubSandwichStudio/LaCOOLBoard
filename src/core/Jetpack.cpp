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
  JsonArray &actuators =
      root.createNestedObject("actuators").createNestedArray("enabled");

  for (int pin = 0; pin < this->sizeList; pin++) {
    state = this->actuatorList[pin].doAction(
        root[this->actuatorList[pin].primaryType], hour, minute);
    actuators.add(state);
    if (pin == 0) {
      this->actuatorList[pin].write(state);
    } else {
      bitWrite(this->action, pin - 1, state);
    }
  }
  this->write(this->action);
}

bool Jetpack::config() {
  if (!SPIFFS.exists("/actuators.json")) {
    File f = SPIFFS.open("/actuators.json", "w");
    if (!f) {
      ERROR_VAR("Failed to open file for writing:", "/actuators.json");
      return (false);
    }
    f.close();
  }
  CoolConfig config("/actuators.json");
  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read /actuators.json");
    return (false);
  }
  JsonObject &actuators = config.get();
  JsonArray &root = actuators["actuators"];
  this->sizeList = root.size();
  this->actuatorList = new CoolBoardActuator[this->sizeList];
  for (int i = 0; i < this->sizeList; i++) {
    INFO_VAR("configuration loaded for actuator #", i);
    this->actuatorList[i].config(root[i]);
  }
  INFO_LOG("Jetpack configuration loaded");
  return (true);
}

void Jetpack::printConf() {
  INFO_LOG("Jetpack configuration");

  for (int i = 0; i < this->sizeList; i++) {
    INFO_VAR("Actuator #", i);
    this->actuatorList[i].printConf();
  }
}
