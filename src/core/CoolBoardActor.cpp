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

#include "CoolBoardActor.h"
#include "CoolLog.h"

void CoolBoardActor::begin() { pinMode(ONBOARD_ACTUATOR_PIN, OUTPUT); }

void CoolBoardActor::write(bool action) {
  DEBUG_VAR("Setting onboard actuator pin to:", action);
  digitalWrite(ONBOARD_ACTUATOR_PIN, action);
}

bool CoolBoardActor::getStatus() {
  return digitalRead(ONBOARD_ACTUATOR_PIN);
}

bool CoolBoardActor::doAction(JsonObject &root, uint8_t hour, uint8_t minute) {
  DEBUG_VAR("Hour value:", hour);
  DEBUG_VAR("Minute value:", minute);

  // invert the current action state for the actor
  // if the value is outside the limits
  if (this->actif == 1) {
    // check if actor is active
    if (this->temporal == 0) {
      // normal actor
      if (this->inverted == 0) {
        // not inverted actor
        this->normalAction(root[this->primaryType].as<float>());
      } else if (this->inverted == 1) {
        // inverted actor
        this->invertedAction(root[this->primaryType].as<float>());
      }
    } else if (this->temporal == 1) {
      // temporal actor
      if (this->secondaryType == "hour") {
        // hour actor
        if (root[this->primaryType].success()) {
          // mixed hour actor
          this->mixedHourAction(hour,
                                root[this->primaryType].as<float>());
        } else {
          // normal hour actor
          this->hourAction(hour);
          // root[this->secondaryType].as<int>());
        }
      } else if (this->secondaryType == "minute") {
        // minute actor
        if (root[this->primaryType].success()) {
          // mixed minute actor
          this->mixedMinuteAction(minute,
                                  root[this->primaryType].as<float>());
        } else {
          // normal minute actor
          this->minuteAction(minute);
        }
      } else if (this->secondaryType == "hourMinute") {
        // hourMinute actor
        if (root[this->primaryType].success()) {
          // mixed hourMinute actor
          this->mixedHourMinuteAction(
              hour, minute, root[this->primaryType].as<float>());
        } else {
          // normal hourMinute actor
          this->hourMinuteAction(hour, minute);
        }
      } else if (this->secondaryType == "") {
        // normal temporal actor
        if (root[this->primaryType].success()) {
          // mixed temporal actor
          this->mixedTemporalActionOn(
              root[this->primaryType].as<float>());
        } else {
          // normal temporal actor
          this->temporalActionOn();
        }
      }
    }
  } else if (this->actif == 0) {
    // disabled actor
    if (this->temporal == 1) {
      // temporal actor
      if (root[this->primaryType].success()) {
        // mixed temporal actor
        this->mixedTemporalActionOff(root[this->primaryType].as<float>());
      } else {
        // normal temporal actor
        this->temporalActionOff();
      }
    }
  }
  return (this->state);
}

bool CoolBoardActor::config() {
  File configFile = SPIFFS.open("/coolBoardActorConfig.json", "r");
  if (!configFile) {
    ERROR_LOG("Failed to read /coolBoardActorConfig.json");
    return (false);
  } else {
    String data = configFile.readString();
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(data);
    if (!json.success()) {
      ERROR_LOG("Failed to parse JSON actuator config from file");
      return (false);
    } else {
      DEBUG_JSON("Actuator config JSON:", json);
      DEBUG_VAR("JSON buffer size:", jsonBuffer.size());
      if (json["actif"].success()) {
        this->actif = json["actif"];
      }
      json["actif"] = this->actif;
      // parsing temporal key
      if (json["temporal"].success()) {
        this->temporal = json["temporal"];
      }
      json["temporal"] = this->temporal;
      // parsing inverted key
      if (json["inverted"].success()) {
        this->inverted = json["inverted"];
      }
      json["inverted"] = this->inverted;

      // parsing low key
      if (json["low"].success()) {
        this->rangeLow = json["low"][0];
        this->timeLow = json["low"][1];
        this->hourLow = json["low"][2];
        this->minuteLow = json["low"][3];
      }
      json["low"][0] = this->rangeLow;
      json["low"][1] = this->timeLow;
      json["low"][2] = this->hourLow;
      json["low"][3] = this->minuteLow;

      // parsing high key
      if (json["high"].success()) {
        this->rangeHigh = json["high"][0];
        this->timeHigh = json["high"][1];
        this->hourHigh = json["high"][2];
        this->minuteHigh = json["high"][3];
      }
      json["high"][0] = this->rangeHigh;
      json["high"][1] = this->timeHigh;
      json["high"][2] = this->hourHigh;
      json["high"][3] = this->minuteHigh;

      // parsing type key
      if (json["type"].success()) {
        this->primaryType = json["type"][0].as<String>();
        this->secondaryType = json["type"][1].as<String>();
      }
      json["type"][0] = this->primaryType;
      json["type"][1] = this->secondaryType;
      configFile.close();
      configFile = SPIFFS.open("/coolBoardActorConfig.json", "w");

      if (!configFile) {
        ERROR_LOG("Failed to write to /coolBoardActorConfig.json");
        return (false);
      }
      json.printTo(configFile);
      configFile.close();
      DEBUG_JSON("Saved actuator config to /coolBoardActorConfig.json", json);
      return (true);
    }
  }
}

void CoolBoardActor::printConf() {
  INFO_VAR("  Actif          = ", this->actif);
  INFO_VAR("  Temporal       = ", this->temporal);
  INFO_VAR("  Inverted       = ", this->inverted);
  INFO_VAR("  Primary type   = ", this->primaryType);
  INFO_VAR("  Secondary type = ", this->secondaryType);
  INFO_VAR("  Range low      = ", this->rangeLow);
  INFO_VAR("  Time low       = ", this->timeLow);
  INFO_VAR("  Hour low       = ", this->hourLow);
  INFO_VAR("  Minute low     = ", this->minuteLow);
  INFO_VAR("  Range high     = ", this->rangeHigh);
  INFO_VAR("  Time high      = ", this->timeHigh);
  INFO_VAR("  Hour high      = ", this->hourHigh);
  INFO_VAR("  Minute high    = ", this->minuteHigh);
}

void CoolBoardActor::normalAction(float measurment) {
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range HIGH:", this->rangeHigh);
  DEBUG_VAR("Range LOW:", this->rangeLow);
Serial.print("normalAction(");
  if (measurment < this->rangeLow) {
    // measured value lower than minimum range : activate actor
    this->state = 1;
    DEBUG_LOG("Actuator ON (sample < rangeLow)");
    Serial.print("1");
  } else if (measurment > this->rangeHigh) {
    // measured value higher than maximum range : deactivate actor
    DEBUG_LOG("Actuator OFF (sample > rangeHigh)");
    this->state = 0;
    Serial.print("0");
  }
  Serial.println(")");
}

void CoolBoardActor::invertedAction(float measurment) {
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range HIGH:", this->rangeHigh);
  DEBUG_VAR("Range LOW:", this->rangeLow);
Serial.print("invertedAction(");
  if (measurment < this->rangeLow) {
    // measured value lower than minimum range : deactivate actor
    this->state = 0;
    Serial.print("0");
    DEBUG_LOG("Actuator OFF (sample < rangeLow)");
  } else if (measurment > this->rangeHigh) {
    // measured value higher than maximum range : activate actor
    this->state = 1;
    Serial.print("1");
    DEBUG_LOG("Actuator ON (sample < rangeHigh)");
  }
  Serial.println(")");
}

void CoolBoardActor::temporalActionOff() {
  DEBUG_LOG("Temporal actuator");
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Time active:", this->actifTime);
  DEBUG_VAR("Time HIGH:", this->timeHigh);
Serial.print("temporalActionOff(");
  if ((millis() - this->actifTime) >= (this->timeHigh)) {
    // stop the actor
    this->state = 0;
    Serial.print("0");
    // make the actor inactif:
    this->actif = 0;
    // start the low timer
    this->inactifTime = millis();
    DEBUG_LOG("Actuator OFF (time active >= duration HIGH)");
  }
  Serial.println(")");
}

void CoolBoardActor::mixedTemporalActionOff(float measurment) {
  DEBUG_LOG("Mixed temporal actuator");
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range HIGH:", this->rangeHigh);
  DEBUG_VAR("Active time:", this->actifTime);
  DEBUG_VAR("Time HIGH:", this->timeHigh);
Serial.print("mixedTemporalActionOff(");
  if ((millis() - this->actifTime) >= (this->timeHigh)) {
    if (measurment >= this->rangeHigh) {
      // stop the actor
      this->state = 0;
      Serial.print("0");
      // make the actor inactif:
      this->actif = 0;
      // start the low timer
      this->inactifTime = millis();
      DEBUG_LOG("Actuator OFF (value >= range HIGH)");
    } else {
      this->state = 1;
      Serial.print("1");
      DEBUG_LOG("Actuator ON (value < range HIGH)");
    }
  }
  Serial.println(")");
}

void CoolBoardActor::temporalActionOn() {
  DEBUG_LOG("Temporal actuator");
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Inactive time:", this->inactifTime);
  DEBUG_VAR("Time LOW:", this->timeLow);
Serial.print("temporalActionOn(");
  if ((millis() - this->inactifTime) >= (this->timeLow)) {
    // start the actor
    this->state = 1;
    // make the actor actif:
    this->actif = 1;
    // start the low timer
    this->actifTime = millis();
    Serial.print("1");
    DEBUG_LOG("Actuator ON (time inactive >= duration LOW)");
  }
  Serial.println(")");
}

void CoolBoardActor::mixedTemporalActionOn(float measurment) {
  DEBUG_LOG("Mixed temporal actuator");
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->rangeLow);
  DEBUG_VAR("Inactive time:", this->inactifTime);
  DEBUG_VAR("Time LOW:", this->timeLow);
Serial.print("mixedTemporalActionOn(");
  if ((millis() - this->inactifTime) >= (this->timeLow)) {
    if (measurment < this->rangeLow) {
      // start the actor
      this->state = 1;
      // make the actor actif:
      this->actif = 1;
      Serial.print("1");
      // start the low timer
      this->actifTime = millis();
      DEBUG_LOG("Actuator ON (value < range LOW)");
    } else {
      this->state = 0;
      Serial.print("0");
      DEBUG_LOG("Actuator OFF (value >= range LOW)");
    }
  }
  Serial.println(")");
}

void CoolBoardActor::hourAction(uint8_t hour) {
  DEBUG_LOG("Hourly triggered actuator");
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->hourHigh);
  DEBUG_VAR("Hour LOW:", this->hourLow);
  DEBUG_VAR("Inverted flag:", this->inverted);
Serial.print("hourAction(");
  if (this->hourHigh < this->hourLow) {
    if (hour >= this->hourLow || hour < this->hourHigh) {
      // stop the actor
      if (this->inverted) {
        this->state = 1;
        Serial.print("1");
      } else {
        this->state = 0;
        Serial.print("0");
      }
      DEBUG_LOG("Daymode, actuator OFF");
    } else {
      // start the actor
      if (this->inverted) {
        this->state = 0;
        Serial.print("0");
      } else {
        this->state = 1;
        Serial.print("1");
      }
      DEBUG_LOG("Daymode, actuator ON");
    }
  } else {
    if (hour >= this->hourLow && hour < this->hourHigh) {
      // stop the actor in "night mode", i.e. a light that is on at night
      if (this->inverted) {
        this->state = 1;
        Serial.print("1");
      } else {
        this->state = 0;
        Serial.print("0");
      }
      DEBUG_LOG("Nightmode, actuator OFF");
    } else {
      // starting the actor
      if (this->inverted) {
        this->state = 0;
        Serial.print("0");
      } else {
        this->state = 1;
        Serial.print("1");
      }
      DEBUG_LOG("Nightmode, actuator ON");
    }
  }
  Serial.println(")");
}

void CoolBoardActor::mixedHourAction(uint8_t hour, float measurment) {
  DEBUG_LOG("Mixed hourly triggered actuator");
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->hourHigh);
  DEBUG_VAR("Hour LOW:", this->hourLow);
  DEBUG_VAR("Inverted flag:", this->inverted);
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->rangeLow);
  DEBUG_VAR("Range HIGH:", this->rangeHigh);
Serial.print("mixedHourAction(");
  if (measurment <= this->rangeLow && this->failsave == true) {
    this->failsave = false;
    WARN_LOG("Resetting failsave for actuator");
  } else if (measurment >= this->rangeHigh &&
             this->failsave == false) {
    this->failsave = true;
    WARN_LOG("Engaging failsave for actuator");
  }

  if (this->hourHigh < this->hourLow) {
    if ((hour >= this->hourLow || hour < this->hourHigh) ||
        this->failsave == true) {
      // stop the actor
      if (this->inverted) {
        this->state = 1;
        Serial.print("1");
      } else {
        this->state = 0;
        Serial.print("0");
      }
      DEBUG_LOG("Daymode, turned OFF actuator");
    } else if (this->failsave == false) {
      // starting the actor
      if (this->inverted) {
        this->state = 0;
        Serial.print("0");
      } else {
        this->state = 1;
        Serial.print("1");
      }
      DEBUG_LOG("Daymode, turned ON actuator");
    }
  } else {
    if ((hour >= this->hourLow && hour < this->hourHigh) ||
        this->failsave == true) {
      // stop the actor in Nght mode ie a light that is on over night
      if (this->inverted) {
        this->state = 1;
        Serial.print("1");
      } else {
        this->state = 0;
        Serial.print("0");
      }
      DEBUG_LOG("Nightmode, turned OFF actuator");
    } else if (this->failsave == false) {
      // starting the actor
      if (this->inverted) {
        this->state = 0;
        Serial.print("0");
      } else {
        this->state = 1;
        Serial.print("1");
      }
      DEBUG_LOG("Nightmode, turned ON actuator");
    }
  }
  Serial.println(")");
}

void CoolBoardActor::minuteAction(uint8_t minute) {
  DEBUG_LOG("Minute-wise triggered actuator");
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->minuteHigh);
  DEBUG_VAR("Minute LOW:", this->minuteLow);
Serial.print("minuteAction(");
  // FIXME: no inverted logic
  // FIXME: what if minuteHigh < minuteLow ?
  if (minute <= this->minuteLow) {
    // stop the actor
    this->state = 0;
    Serial.print("0");
    DEBUG_LOG("Turned OFF actuator (minute <= minute LOW)");

  } else if (minute >= this->minuteHigh) {
    // starting the actor
    this->state = 1;
    Serial.print("1");
    DEBUG_LOG("Turned ON actuator (minute >= minute HIGH)");
  }
  Serial.println(")");
}

void CoolBoardActor::mixedMinuteAction(uint8_t minute, float measurment) {
  DEBUG_LOG("Mixed minute-wise triggered actuator");
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->minuteHigh);
  DEBUG_VAR("Minute LOW:", this->minuteLow);
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->rangeLow);
  DEBUG_VAR("Range HIGH:", this->rangeHigh);
Serial.print("mixedMinuteAction(");
  // FIXME: no inverted logic
  // FIXME: what if minuteHigh < minuteLow ?
  if (minute <= this->minuteLow) {
    if (measurment > this->rangeHigh) {
      this->state = 0;
      Serial.print("0");
      DEBUG_LOG("Turned OFF actuator (value > range HIGH)");
    } else {
      this->state = 1;
      Serial.print("1");
      DEBUG_LOG("Turned ON actuator (value <= range HIGH)");
    }
  } else if (minute >= this->minuteHigh) {
    if (measurment < this->rangeLow) {
      this->state = 1;
      Serial.print("1");
      DEBUG_LOG("Turned ON actuator (value < range LOW)");
    } else {
      this->state = 0;
      Serial.print("0");
      DEBUG_LOG("Turned OFF actuator (value >= range LOW)");
    }
  }
  Serial.println(")");
}

void CoolBoardActor::hourMinuteAction(uint8_t hour, uint8_t minute) {
  DEBUG_LOG("Hour:minute triggered actuator");
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->hourHigh);
  DEBUG_VAR("Hour LOW:", this->hourLow);
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->minuteHigh);
  DEBUG_VAR("Minute LOW:", this->minuteLow);
Serial.print("hourMinuteAction(");
  // FIXME: no inverted logic
  // FIXME: what if hourHigh/minuteHigh < hourLow/minuteLow ?
  if (hour == this->hourLow) {
    if (minute >= this->minuteLow) {
      this->state = 0;
      Serial.print("0");
      DEBUG_LOG("Turned OFF actuator (hour:minute > hour:minute LOW)");
    }
  } else if (hour > this->hourLow) {
    this->state = 0;
    Serial.print("0");
    DEBUG_LOG("Turned OFF actuator (hour > hour LOW)");
  } else if (hour == this->hourHigh) {
    if (minute >= this->minuteHigh) {
      this->state = 1;
      Serial.print("1");
      DEBUG_LOG("Turned ON actuator (hour:minute > hour:minute HIGH)");
    }
  } else if (hour > this->hourHigh) {
    this->state = 1;
    Serial.print("1");
    DEBUG_LOG("Turned ON actuator (hour > hour HIGH)");
  }
  Serial.println(")");
}

void CoolBoardActor::mixedHourMinuteAction(uint8_t hour, uint8_t minute,
                                           float measurment) {
  DEBUG_LOG("Mixed Hour:minute triggered actuator");
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->hourHigh);
  DEBUG_VAR("Hour LOW:", this->hourLow);
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->minuteHigh);
  DEBUG_VAR("Minute LOW:", this->minuteLow);
  DEBUG_VAR("Measured value:", measurment);
  DEBUG_VAR("Range LOW:", this->rangeLow);
  DEBUG_VAR("Range HIGH:", this->rangeHigh);
Serial.print("mixedHourMinuteAction(");
  // stop the actor
  if (hour == this->hourLow) {
    if (minute >= this->minuteLow) {
      if (measurment >= this->rangeHigh) {
        this->state = 0;
        Serial.print("0");
        DEBUG_LOG("Turned OFF actuator (value >= range HIGH)");
      } else {
        this->state = 1;
        Serial.print("1");
        DEBUG_LOG("Turned ON actuator (value < range HIGH)");
      }
    }
  } else if (hour > this->hourLow) {
    if (measurment >= this->rangeHigh) {
      this->state = 0;
      Serial.print("0");
      DEBUG_LOG("Turned OFF actuator (value >= range HIGH)");
    } else {
      this->state = 1;
      Serial.print("1");
      DEBUG_LOG("Turned ON actuator (value < range HIGH)");
    }
  }
  // start the actor
  else if (hour == this->hourHigh) {
    if (minute >= this->minuteHigh) {
      if (measurment < this->rangeLow) {
        this->state = 1;
        Serial.print("1");
        DEBUG_LOG("Turned ON actuator (value < range LOW)");
      } else {
        this->state = 0;
        Serial.print("0");
        DEBUG_LOG("Turned OFF actuator (value >= range LOW)");
      }
    }
  } else if (hour > this->hourHigh) {
    if (measurment < this->rangeLow) {
      this->state = 1;
      Serial.print("1");
      DEBUG_LOG("Turned ON actuator (value < range LOW)");
    } else {
      this->state = 0;
      Serial.print("0");
      DEBUG_LOG("Turned OFF actuator (value >= range LOW)");
    }
  }
  Serial.println(")");
}
