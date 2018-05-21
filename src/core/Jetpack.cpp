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
    state = actorList[pin].doAction(root, hour, minute);
    root[String("Act") + String(pin)] = state;
    Serial.print("-----------------------[");
    Serial.print(state);
    Serial.println("]-----------------------");
    bitWrite(this->action, pin, state);
  }
  this->write(this->action);
}

bool Jetpack::config() {
  File jetPackConfig = SPIFFS.open("/jetPackConfig.json", "r");

  if (!jetPackConfig) {
    ERROR_LOG("Failed to read /jetPackConfig.json");
    return (false);
  } else {
    size_t size = jetPackConfig.size();

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    jetPackConfig.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    if (!json.success()) {
      ERROR_LOG("Failed to parse JSON Jetpack config from file");
      return (false);
    } else {
      DEBUG_JSON("Jetpack config JSON:", json);
      for (int i = 0; i < 8; i++) {
        if (json[String("Act") + String(i)].success()) {
          // parsing actif key
          if (json[String("Act") + String(i)]["actif"].success()) {
            this->actorList[i].actif = json[String("Act") + String(i)]["actif"];
          }
          json[String("Act") + String(i)]["actif"] = this->actorList[i].actif;

          // parsing temporal key
          if (json[String("Act") + String(i)]["temporal"].success()) {
            this->actorList[i].temporal =
                json[String("Act") + String(i)]["temporal"];
          }
          json[String("Act") + String(i)]["temporal"] =
              this->actorList[i].temporal;

          // parsing inverted key
          if (json[String("Act") + String(i)]["inverted"].success()) {
            this->actorList[i].inverted =
                json[String("Act") + String(i)]["inverted"];
          }
          json[String("Act") + String(i)]["inverted"] =
              this->actorList[i].inverted;

          // parsing inverted key
          if (json[String("Act") + String(i)]["inverted"].success()) {
            this->actorList[i].inverted =
                json[String("Act") + String(i)]["inverted"];
          }
          json[String("Act") + String(i)]["inverted"] =
              this->actorList[i].inverted;

          // parsing low key
          if (json[String("Act") + String(i)]["low"].success()) {
            this->actorList[i].rangeLow =
                json[String("Act") + String(i)]["low"][0];
            this->actorList[i].timeLow = json[String("Act") + String(i)]["low"][1];
            this->actorList[i].hourLow = json[String("Act") + String(i)]["low"][2];
            this->actorList[i].minuteLow =
                json[String("Act") + String(i)]["low"][3];
          }
          json[String("Act") + String(i)]["low"][0] = this->actorList[i].rangeLow;
          json[String("Act") + String(i)]["low"][1] = this->actorList[i].timeLow;
          json[String("Act") + String(i)]["low"][2] = this->actorList[i].hourLow;
          json[String("Act") + String(i)]["low"][3] = this->actorList[i].minuteLow;

          // parsing high key
          if (json[String("Act") + String(i)]["high"].success()) {
            this->actorList[i].rangeHigh =
                json[String("Act") + String(i)]["high"][0];
            this->actorList[i].timeHigh =
                json[String("Act") + String(i)]["high"][1];
            this->actorList[i].hourHigh =
                json[String("Act") + String(i)]["high"][2];
            this->actorList[i].minuteHigh =
                json[String("Act") + String(i)]["high"][3];
          }
          json[String("Act") + String(i)]["high"][0] =
              this->actorList[i].rangeHigh;
          json[String("Act") + String(i)]["high"][1] = this->actorList[i].timeHigh;
          json[String("Act") + String(i)]["high"][2] = this->actorList[i].hourHigh;
          json[String("Act") + String(i)]["high"][3] =
              this->actorList[i].minuteHigh;

          // parsing type key
          if (json[String("Act") + String(i)]["type"].success()) {
            this->actorList[i].primaryType =
                json[String("Act") + String(i)]["type"][0].as<String>();
            this->actorList[i].secondaryType =
                json[String("Act") + String(i)]["type"][1].as<String>();
          }
          json[String("Act") + String(i)]["type"][0] =
              this->actorList[i].primaryType;
          json[String("Act") + String(i)]["type"][1] =
              this->actorList[i].secondaryType;
        }
        json[String("Act") + String(i)]["actif"] = this->actorList[i].actif;
        json[String("Act") + String(i)]["temporal"] = this->actorList[i].temporal;
        json[String("Act") + String(i)]["inverted"] = this->actorList[i].inverted;
        json[String("Act") + String(i)]["low"][0] = this->actorList[i].rangeLow;
        json[String("Act") + String(i)]["low"][1] = this->actorList[i].timeLow;
        json[String("Act") + String(i)]["low"][2] = this->actorList[i].hourLow;
        json[String("Act") + String(i)]["low"][3] = this->actorList[i].minuteLow;
        json[String("Act") + String(i)]["high"][0] = this->actorList[i].rangeHigh;
        json[String("Act") + String(i)]["high"][1] = this->actorList[i].timeHigh;
        json[String("Act") + String(i)]["high"][2] = this->actorList[i].hourHigh;
        json[String("Act") + String(i)]["high"][3] = this->actorList[i].minuteHigh;
        json[String("Act") + String(i)]["type"][0] =
            this->actorList[i].primaryType;
        json[String("Act") + String(i)]["type"][1] =
            this->actorList[i].secondaryType;
      }
      jetPackConfig.close();
      jetPackConfig = SPIFFS.open("/jetPackConfig.json", "w");

      if (!jetPackConfig) {
        ERROR_LOG("Failed to write to /jetPackConfig.json");
        return (false);
      }
      json.printTo(jetPackConfig);
      jetPackConfig.close();
      INFO_LOG("Saved Jetpack config to /jetPackConfig.json");
      return (true);
    }
  }
}

void Jetpack::printConf() {
  INFO_LOG("Jetpack configuration");

  for (int i = 0; i < 8; i++) {
    INFO_VAR("Actuator #", i);
    actorList[i].printConf();
  }
}
