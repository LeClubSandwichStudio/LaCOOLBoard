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

// FIXME: merge with CoolBoardActor
void Jetpack::doAction(JsonObject &root, int hour, int minute) {
  for (int i = 0; i < 8; i++) {
    if (this->actors[i].actif == 1) {
      // check if actor is enabled
      if (this->actors[i].temporal == 0) {
        // normal actor
        if (this->actors[i].inverted == 0) {
          // not inverted actor
          this->normalAction(i, root[this->actors[i].primaryType].as<float>());

        } else if (this->actors[i].inverted == 1) {
          // inverted actor
          this->invertedAction(i,
                               root[this->actors[i].primaryType].as<float>());
        }
      } else if (this->actors[i].temporal == 1) {
        // temporal actor
        if (this->actors[i].secondaryType == "hour") {
          // hour actor
          if (root[this->actors[i].primaryType].success()) {
            // mixed hour actor
            this->mixedHourAction(
                i, hour, root[this->actors[i].primaryType].as<float>());
          } else {
            // normal hour actor
            this->hourAction(i, hour);
          }
        } else if (this->actors[i].secondaryType == "minute") {
          // minute actor
          if (root[this->actors[i].primaryType].success()) {
            // mixed minute actor
            this->mixedMinuteAction(
                i, root[this->actors[i].secondaryType].as<int>(),
                root[this->actors[i].primaryType].as<float>());
          } else {
            // normal minute actor
            this->minuteAction(i,
                               root[this->actors[i].secondaryType].as<int>());
          }
        } else if (this->actors[i].secondaryType == "hourMinute") {
          // hourMinute actor
          if (root[this->actors[i].primaryType].success()) {
            // mixed hourMinute actor
            this->mixedHourMinuteAction(
                i, hour, minute, root[this->actors[i].primaryType].as<float>());
          } else {
            // normal hourMinute actor
            this->hourMinuteAction(i, hour, minute);
          }
        } else if (this->actors[i].secondaryType == "") {
          // normal temporal actor
          if (root[this->actors[i].primaryType].success()) {
            // mixed temporal actor
            this->mixedTemporalActionOn(
                i, root[this->actors[i].primaryType].as<float>());
          } else {
            // normal temporal actor
            this->temporalActionOff(i);
          }
        }
      }
    } else if (this->actors[i].actif == 0) {
      // disabled actor
      if (this->actors[i].temporal == 1) {
        // temporal actor
        if (root[this->actors[i].primaryType].success()) {
          // mixed temporal actor
          this->mixedTemporalActionOff(
              i, root[this->actors[i].primaryType].as<float>());
        } else {
          // normal temporal actor
          this->temporalActionOn(i);
        }
      }
    }
    root[String("Act") + String(i)] = (bitRead(this->action, i) == 1);
  }
  // FIXME: unneeded ?
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
            this->actors[i].actif = json[String("Act") + String(i)]["actif"];
          }
          json[String("Act") + String(i)]["actif"] = this->actors[i].actif;

          // parsing temporal key
          if (json[String("Act") + String(i)]["temporal"].success()) {
            this->actors[i].temporal =
                json[String("Act") + String(i)]["temporal"];
          }
          json[String("Act") + String(i)]["temporal"] =
              this->actors[i].temporal;

          // parsing inverted key
          if (json[String("Act") + String(i)]["inverted"].success()) {
            this->actors[i].inverted =
                json[String("Act") + String(i)]["inverted"];
          }
          json[String("Act") + String(i)]["inverted"] =
              this->actors[i].inverted;

          // parsing inverted key
          if (json[String("Act") + String(i)]["inverted"].success()) {
            this->actors[i].inverted =
                json[String("Act") + String(i)]["inverted"];
          }
          json[String("Act") + String(i)]["inverted"] =
              this->actors[i].inverted;

          // parsing low key
          if (json[String("Act") + String(i)]["low"].success()) {
            this->actors[i].rangeLow =
                json[String("Act") + String(i)]["low"][0];
            this->actors[i].timeLow = json[String("Act") + String(i)]["low"][1];
            this->actors[i].hourLow = json[String("Act") + String(i)]["low"][2];
            this->actors[i].minuteLow =
                json[String("Act") + String(i)]["low"][3];
          }
          json[String("Act") + String(i)]["low"][0] = this->actors[i].rangeLow;
          json[String("Act") + String(i)]["low"][1] = this->actors[i].timeLow;
          json[String("Act") + String(i)]["low"][2] = this->actors[i].hourLow;
          json[String("Act") + String(i)]["low"][3] = this->actors[i].minuteLow;

          // parsing high key
          if (json[String("Act") + String(i)]["high"].success()) {
            this->actors[i].rangeHigh =
                json[String("Act") + String(i)]["high"][0];
            this->actors[i].timeHigh =
                json[String("Act") + String(i)]["high"][1];
            this->actors[i].hourHigh =
                json[String("Act") + String(i)]["high"][2];
            this->actors[i].minuteHigh =
                json[String("Act") + String(i)]["high"][3];
          }
          json[String("Act") + String(i)]["high"][0] =
              this->actors[i].rangeHigh;
          json[String("Act") + String(i)]["high"][1] = this->actors[i].timeHigh;
          json[String("Act") + String(i)]["high"][2] = this->actors[i].hourHigh;
          json[String("Act") + String(i)]["high"][3] =
              this->actors[i].minuteHigh;

          // parsing type key
          if (json[String("Act") + String(i)]["type"].success()) {
            this->actors[i].primaryType =
                json[String("Act") + String(i)]["type"][0].as<String>();
            this->actors[i].secondaryType =
                json[String("Act") + String(i)]["type"][1].as<String>();
          }
          json[String("Act") + String(i)]["type"][0] =
              this->actors[i].primaryType;
          json[String("Act") + String(i)]["type"][1] =
              this->actors[i].secondaryType;
        }
        json[String("Act") + String(i)]["actif"] = this->actors[i].actif;
        json[String("Act") + String(i)]["temporal"] = this->actors[i].temporal;
        json[String("Act") + String(i)]["inverted"] = this->actors[i].inverted;
        json[String("Act") + String(i)]["low"][0] = this->actors[i].rangeLow;
        json[String("Act") + String(i)]["low"][1] = this->actors[i].timeLow;
        json[String("Act") + String(i)]["low"][2] = this->actors[i].hourLow;
        json[String("Act") + String(i)]["low"][3] = this->actors[i].minuteLow;
        json[String("Act") + String(i)]["high"][0] = this->actors[i].rangeHigh;
        json[String("Act") + String(i)]["high"][1] = this->actors[i].timeHigh;
        json[String("Act") + String(i)]["high"][2] = this->actors[i].hourHigh;
        json[String("Act") + String(i)]["high"][3] = this->actors[i].minuteHigh;
        json[String("Act") + String(i)]["type"][0] =
            this->actors[i].primaryType;
        json[String("Act") + String(i)]["type"][1] =
            this->actors[i].secondaryType;
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
    INFO_VAR("  Actif          =", this->actors[i].actif);
    INFO_VAR("  Temporal       =", this->actors[i].temporal);
    INFO_VAR("  Inverted       =", this->actors[i].inverted);
    INFO_VAR("  Primary type   =", this->actors[i].primaryType);
    INFO_VAR("  Secondary type =", this->actors[i].secondaryType);
    INFO_VAR("  Range low      =", this->actors[i].rangeLow);
    INFO_VAR("  Time low       =", this->actors[i].timeLow);
    INFO_VAR("  Hour low       =", this->actors[i].hourLow);
    INFO_VAR("  Minute low     =", this->actors[i].minuteLow);
    INFO_VAR("  Range high     =", this->actors[i].rangeHigh);
    INFO_VAR("  Time high      =", this->actors[i].timeHigh);
    INFO_VAR("  Hour high      =", this->actors[i].hourHigh);
    INFO_VAR("  Minute high    =", this->actors[i].minuteHigh);
  }
}

void Jetpack::normalAction(int actorNumber, float measurment) {
  DEBUG_VAR("Actuator #", actorNumber);
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range HIGH:", this->actors[actorNumber].rangeHigh);
  DEBUG_VAR("Range LOW:", this->actors[actorNumber].rangeLow);

  if (measurment < this->actors[actorNumber].rangeLow) {
    // measured value lower than minimum range : activate actor
    bitWrite(this->action, actorNumber, 1);
    DEBUG_VAR("Actuator ON (sample < rangeLow) for #", actorNumber);
  } else if (measurment > this->actors[actorNumber].rangeHigh) {
    // measured value higher than maximum range : deactivate actor
    bitWrite(this->action, actorNumber, 0);
    DEBUG_VAR("Actuator OFF (sample > rangeHigh) for #", actorNumber);
  }
}

void Jetpack::invertedAction(int actorNumber, float measurment) {
  DEBUG_VAR("Actuator #", actorNumber);
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range HIGH:", this->actors[actorNumber].rangeHigh);
  DEBUG_VAR("Range LOW:", this->actors[actorNumber].rangeLow);

  if (measurment < this->actors[actorNumber].rangeLow) {
    // measured value lower than minimum range : deactivate actor
    bitWrite(this->action, actorNumber, 0);
    DEBUG_VAR("Actuator OFF (sample < rangeLow) for #", actorNumber);
  } else if (measurment > this->actors[actorNumber].rangeHigh) {
    // measured value higher than maximum range : activate actor
    bitWrite(this->action, actorNumber, 1);
    DEBUG_VAR("Actuator ON (sample > rangeHigh) for #", actorNumber);
  }
}

void Jetpack::temporalActionOff(int actorNumber) {
  DEBUG_VAR("Temporal actuator #", actorNumber);
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Time active:", this->actors[actorNumber].actifTime);
  DEBUG_VAR("Time HIGH:", this->actors[actorNumber].timeHigh);

  // FIXME: different condition in CoolBoardActor
  if ((millis() - this->actors[actorNumber].actifTime) >=
          (this->actors[actorNumber].timeHigh) ||
      this->actors[actorNumber].actifTime == 0) {
    // stop the actor
    bitWrite(this->action, actorNumber, 0);
    // make the actor inactif:
    this->actors[actorNumber].actif = 0;
    // start the low timer
    this->actors[actorNumber].inactifTime = millis();
    DEBUG_VAR("Actuator OFF (time active >= duration HIGH) for #", actorNumber);
  }
}

void Jetpack::mixedTemporalActionOff(int actorNumber, float measurment) {
  DEBUG_VAR("Mixed temporal actuator #", actorNumber);
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range HIGH:", this->actors[actorNumber].rangeHigh);
  DEBUG_VAR("Active time:", this->actors[actorNumber].actifTime);
  DEBUG_VAR("Time HIGH:", this->actors[actorNumber].timeHigh);

  if ((millis() - this->actors[actorNumber].actifTime) >=
      (this->actors[actorNumber].timeHigh)) {
    if (measurment >= this->actors[actorNumber].rangeHigh) {
      // stop the actor
      bitWrite(this->action, actorNumber, 0);
      // make the actor inactif:
      // FIXME: `actif` meaning is ambiguous
      this->actors[actorNumber].actif = 0;
      // start the low timer
      this->actors[actorNumber].inactifTime = millis();
      DEBUG_VAR("Actuator OFF (value >= range HIGH) for #", actorNumber);
    } else {
      DEBUG_VAR("Actuator ON (value < range HIGH) for #", actorNumber);
      // FIXME: not consistent with other actions
      bitWrite(this->action, actorNumber, 1);
    }
  }
}

void Jetpack::temporalActionOn(int actorNumber) {
  DEBUG_VAR("Temporal actuator #", actorNumber);
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Time active:", this->actors[actorNumber].actifTime);
  DEBUG_VAR("Time HIGH:", this->actors[actorNumber].timeHigh);

  if ((millis() - this->actors[actorNumber].inactifTime) >=
      (this->actors[actorNumber].timeLow)) {
    // start the actor
    bitWrite(this->action, actorNumber, 1);
    // make the actor actif:
    this->actors[actorNumber].actif = 1;
    // start the low timer
    this->actors[actorNumber].actifTime = millis();
    DEBUG_VAR("Actuator ON (time inactive >= duration LOW) for #", actorNumber);
  }
  // FIXME: missing else clause ?
}

void Jetpack::mixedTemporalActionOn(int actorNumber, float measurment) {
  DEBUG_VAR("Mixed temporal actuator #", actorNumber);
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->actors[actorNumber].rangeLow);
  DEBUG_VAR("Inactive time:", this->actors[actorNumber].inactifTime);
  DEBUG_VAR("Time LOW:", this->actors[actorNumber].timeLow);

  if ((millis() - this->actors[actorNumber].inactifTime) >=
      (this->actors[actorNumber].timeLow)) {
    if (measurment < this->actors[actorNumber].rangeLow) {
      // start the actor
      bitWrite(this->action, actorNumber, 1);
      // make the actor actif:
      // FIXME: actif is ambiguous
      this->actors[actorNumber].actif = 1;
      // start the low timer
      this->actors[actorNumber].actifTime = millis();
      DEBUG_VAR("Actuator ON (value < range LOW) for #", actorNumber);
    } else {
      bitWrite(this->action, actorNumber, 0);
      DEBUG_VAR("Actuator OFF (value >= range LOW)", actorNumber);
    }
  }
}

void Jetpack::hourAction(int actorNumber, int hour) {
  DEBUG_VAR("Hourly triggered actuator #", actorNumber);
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->actors[actorNumber].hourHigh);
  DEBUG_VAR("Hour LOW:", this->actors[actorNumber].hourLow);
  DEBUG_VAR("Inverted flag:", this->actors[actorNumber].inverted);

  if (this->actors[actorNumber].hourHigh < this->actors[actorNumber].hourLow) {
    if (hour >= this->actors[actorNumber].hourLow ||
        hour < this->actors[actorNumber].hourHigh) {
      // stop the actor
      if (this->actors[actorNumber].inverted) {
        bitWrite(this->action, actorNumber, 1);
      } else {
        bitWrite(this->action, actorNumber, 0);
      }
      DEBUG_VAR("Daymode, turned OFF actuator #", actorNumber);
    } else {
      if (this->actors[actorNumber].inverted) {
        // starting the actor
        bitWrite(this->action, actorNumber, 0);
      } else {
        bitWrite(this->action, actorNumber, 1);
      }
      DEBUG_VAR("Daymode, turned ON actuator #", actorNumber);
    }
  } else {
    if (hour >= this->actors[actorNumber].hourLow &&
        hour < this->actors[actorNumber].hourHigh) {
      // stop the actor in Nght mode ie a light that is on over night
      if (this->actors[actorNumber].inverted) {
        bitWrite(this->action, actorNumber, 1);
      } else {
        bitWrite(this->action, actorNumber, 0);
      }
      DEBUG_VAR("Nightmode, turned OFF actuator #", actorNumber);
    } else {
      // starting the actor
      if (this->actors[actorNumber].inverted) {
        bitWrite(this->action, actorNumber, 0);
      } else {
        bitWrite(this->action, actorNumber, 1);
      }
      DEBUG_VAR("Nightmode, turned ON actuator #", actorNumber);
    }
  }
}

void Jetpack::mixedHourAction(int actorNumber, int hour, float measurment) {
  DEBUG_VAR("Mixed hourly triggered actuator #", actorNumber);
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->actors[actorNumber].hourHigh);
  DEBUG_VAR("Hour LOW:", this->actors[actorNumber].hourLow);
  DEBUG_VAR("Inverted flag:", this->actors[actorNumber].inverted);
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->actors[actorNumber].rangeLow);
  DEBUG_VAR("Range HIGH:", this->actors[actorNumber].rangeHigh);

  if (measurment <= this->actors[actorNumber].rangeLow &&
      this->actors[actorNumber].failsave == true) {
    this->actors[actorNumber].failsave = false;
    WARN_VAR("Resetting failsave for actuator #", actorNumber);
  } else if (measurment >= this->actors[actorNumber].rangeHigh &&
             this->actors[actorNumber].failsave == false) {
    this->actors[actorNumber].failsave = true;
    WARN_VAR("Engaging failsave for actuator #", actorNumber);
  }

  if (this->actors[actorNumber].hourHigh < this->actors[actorNumber].hourLow) {
    if ((hour >= this->actors[actorNumber].hourLow ||
         hour < this->actors[actorNumber].hourHigh) ||
        this->actors[actorNumber].failsave == true) {
      // stop the actor
      if (this->actors[actorNumber].inverted) {
        bitWrite(this->action, actorNumber, 1);
      } else {
        bitWrite(this->action, actorNumber, 0);
      }
      DEBUG_VAR("Daymode, turned OFF actuator #", actorNumber);
    } else if (this->actors[actorNumber].failsave == false) {
      // starting the actor
      if (this->actors[actorNumber].inverted) {
        bitWrite(this->action, actorNumber, 0);
      } else {
        bitWrite(this->action, actorNumber, 1);
      }
      DEBUG_VAR("Daymode, turned ON actuator #", actorNumber);
    }
  } else {
    if ((hour >= this->actors[actorNumber].hourLow &&
         hour < this->actors[actorNumber].hourHigh) ||
        this->actors[actorNumber].failsave == true) {
      // stop the actor in Nght mode ie a light that is on over night
      if (this->actors[actorNumber].inverted) {
        bitWrite(this->action, actorNumber, 1);
      } else {
        bitWrite(this->action, actorNumber, 0);
      }
      DEBUG_VAR("Nightmode, turned OFF actuator #", actorNumber);
    } else if (this->actors[actorNumber].failsave == false) {
      // starting the actor
      if (this->actors[actorNumber].inverted) {
        bitWrite(this->action, actorNumber, 0);
      } else {
        bitWrite(this->action, actorNumber, 1);
      }
      DEBUG_VAR("Nightmode, turned ON actuator #", actorNumber);
    }
  }
}

void Jetpack::minuteAction(int actorNumber, int minute) {
  DEBUG_VAR("Minute-wise triggered actuator #", actorNumber);
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->actors[actorNumber].minuteHigh);
  DEBUG_VAR("Minute LOW:", this->actors[actorNumber].minuteLow);
  DEBUG_VAR("Inverted flag:", this->actors[actorNumber].inverted);

  if (minute <= this->actors[actorNumber].minuteLow) {
    // stop the actor
    bitWrite(this->action, actorNumber, 0);
    DEBUG_VAR("Turned OFF onboard actuator (minute <= minute LOW) for #",
              actorNumber);
  } else if (minute >= this->actors[actorNumber].minuteHigh) {
    // starting the actor
    bitWrite(this->action, actorNumber, 1);
    DEBUG_VAR("Turned ON onboard actuator (minute >= minute HIGH) for #",
              actorNumber);
  }
}

void Jetpack::mixedMinuteAction(int actorNumber, int minute, float measurment) {
  DEBUG_VAR("Mixed minute-wise triggered actuator #", actorNumber);
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->actors[actorNumber].minuteHigh);
  DEBUG_VAR("Minute LOW:", this->actors[actorNumber].minuteLow);
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->actors[actorNumber].rangeLow);
  DEBUG_VAR("Range HIGH:", this->actors[actorNumber].rangeHigh);

  if (minute <= this->actors[actorNumber].minuteLow) {
    if (measurment > this->actors[actorNumber].rangeHigh) {
      bitWrite(this->action, actorNumber, 0);
      DEBUG_VAR("Turned OFF actuator (value > range HIGH) for #", actorNumber);
    } else {
      bitWrite(this->action, actorNumber, 1);
      DEBUG_VAR("Turned ON actuator (value <= range HIGH) for #", actorNumber);
    }
  } else if (minute >= this->actors[actorNumber].minuteHigh) {
    if (measurment < this->actors[actorNumber].rangeLow) {
      bitWrite(this->action, actorNumber, 1);
      DEBUG_VAR("Turned ON actuator (value < range LOW) for #", actorNumber);
    } else {
      bitWrite(this->action, actorNumber, 0);
      DEBUG_VAR("Turned OFF actuator (value >= range LOW) for #", actorNumber);
    }
  }
}

void Jetpack::hourMinuteAction(int actorNumber, int hour, int minute) {
  DEBUG_VAR("Hour:minute triggered actuator #", actorNumber);
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->actors[actorNumber].hourHigh);
  DEBUG_VAR("Hour LOW:", this->actors[actorNumber].hourLow);
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->actors[actorNumber].minuteHigh);
  DEBUG_VAR("Minute LOW:", this->actors[actorNumber].minuteLow);

  // FIXME: no inverted logic
  // FIXME: what if hourHigh/minuteHigh < hourLow/minuteLow ?
  if (hour == this->actors[actorNumber].hourLow) {
    if (minute >= this->actors[actorNumber].minuteLow) {
      bitWrite(this->action, actorNumber, 0);
      DEBUG_VAR("Turned OFF actuator (hour:minute > hour:minute LOW) for #",
                actorNumber);
    }
  } else if (hour > this->actors[actorNumber].hourLow) {
    bitWrite(this->action, actorNumber, 0);
    DEBUG_VAR("Turned OFF actuator (hour > hour LOW) for #", actorNumber);
  } else if (hour == this->actors[actorNumber].hourHigh) {
    if (minute >= this->actors[actorNumber].minuteHigh) {
      bitWrite(this->action, actorNumber, 1);
      DEBUG_VAR("Turned ON actuator (hour:minute > hour:minute HIGH) for #",
                actorNumber);
    }
  } else if (hour > this->actors[actorNumber].hourHigh) {
    bitWrite(this->action, actorNumber, 1);
    DEBUG_VAR("Turned ON actuator (hour > hour HIGH) for #", actorNumber);
  }
}

void Jetpack::mixedHourMinuteAction(int actorNumber, int hour, int minute,
                                    float measurment) {
  DEBUG_VAR("Mixed hour:minute triggered actuator #", actorNumber);
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->actors[actorNumber].hourHigh);
  DEBUG_VAR("Hour LOW:", this->actors[actorNumber].hourLow);
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->actors[actorNumber].minuteHigh);
  DEBUG_VAR("Minute LOW:", this->actors[actorNumber].minuteLow);
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->actors[actorNumber].rangeLow);
  DEBUG_VAR("Range HIGH:", this->actors[actorNumber].rangeHigh);

  // stop the actor
  if (hour == this->actors[actorNumber].hourLow) {
    if (minute >= this->actors[actorNumber].minuteLow) {
      if (measurment >= this->actors[actorNumber].rangeHigh) {
        bitWrite(this->action, actorNumber, 0);
        DEBUG_VAR("Turned OFF actuator (value >= range HIGH) for #",
                  actorNumber);
      } else {
        bitWrite(this->action, actorNumber, 1);
        DEBUG_VAR("Turned ON actuator (value < range HIGH) for #", actorNumber);
      }
    }
  } else if (hour > this->actors[actorNumber].hourLow) {
    if (measurment >= this->actors[actorNumber].rangeHigh) {
      bitWrite(this->action, actorNumber, 0);
      DEBUG_VAR("Turned OFF actuator (value >= range HIGH) for #", actorNumber);
    } else {
      bitWrite(this->action, actorNumber, 1);
      DEBUG_VAR("Turned ON actuator (value < range HIGH) for #", actorNumber);
    }
  } else if (hour == this->actors[actorNumber].hourHigh) {
    if (minute >= this->actors[actorNumber].minuteHigh) {
      if (measurment < this->actors[actorNumber].rangeLow) {
        bitWrite(this->action, actorNumber, 1);
        DEBUG_VAR("Turned ON actuator (value < range LOW) for #", actorNumber);
      } else {
        bitWrite(this->action, actorNumber, 0);
        DEBUG_VAR("Turned OFF actuator (value >= range LOW) for #",
                  actorNumber);
      }
    }
  } else if (hour > this->actors[actorNumber].hourHigh) {
    if (measurment < this->actors[actorNumber].rangeLow) {
      bitWrite(this->action, actorNumber, 1);
      DEBUG_VAR("Turned ON actuator (value < range LOW) for #", actorNumber);
    } else {
      bitWrite(this->action, actorNumber, 0);
      DEBUG_VAR("Turned OFF actuator (value >= range LOW) for #", actorNumber);
    }
  }
}
