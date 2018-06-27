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

#include "CoolBoardActuator.h"
#include "CoolConfig.h"
#include "CoolLog.h"

void CoolBoardActuator::begin() { pinMode(ONBOARD_ACTUATOR_PIN, OUTPUT); }

void CoolBoardActuator::write(bool action) {
  DEBUG_VAR("Setting onboard actuator pin to:", action);
  digitalWrite(ONBOARD_ACTUATOR_PIN, action);
}

bool CoolBoardActuator::getStatus() {
  return digitalRead(ONBOARD_ACTUATOR_PIN);
}

bool CoolBoardActuator::doAction(JsonObject &root, uint8_t hour,
                                 uint8_t minute) {
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
          this->mixedHourAction(hour, root[this->primaryType].as<float>());
        } else {
          // normal hour actor
          this->hourAction(hour);
          // root[this->secondaryType].as<int>());
        }
      } else if (this->secondaryType == "minute") {
        // minute actor
        if (root[this->primaryType].success()) {
          // mixed minute actor
          this->mixedMinuteAction(minute, root[this->primaryType].as<float>());
        } else {
          // normal minute actor
          this->minuteAction(minute);
        }
      } else if (this->secondaryType == "hourMinute") {
        // hourMinute actor
        if (root[this->primaryType].success()) {
          // mixed hourMinute actor
          this->mixedHourMinuteAction(hour, minute,
                                      root[this->primaryType].as<float>());
        } else {
          // normal hourMinute actor
          this->hourMinuteAction(hour, minute);
        }
      } else if (this->secondaryType == "") {
        // normal temporal actor
        if (root[this->primaryType].success()) {
          // mixed temporal actor
          this->mixedTemporalActionOn(root[this->primaryType].as<float>());
        } else {
          // normal temporal actor
          this->temporalActionOff();
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
        this->temporalActionOn();
      }
    }
  }
  return (this->state);
}

bool CoolBoardActuator::config() {
  CoolConfig config("/coolBoardActorConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read builtin actuator configuration");
    return (false);
  }
  JsonObject &json = config.get();
  config.set<bool>(json, "actif", this->actif);
  // parsing temporal key
  config.set<bool>(json, "temporal", this->temporal);
  // parsing inverted key
  config.set<bool>(json, "inverted", this->inverted);
  // parsing low key
  config.setArray<int>(json, "low", 0, this->rangeLow);
  config.setArray<unsigned long>(json, "low", 1, this->timeLow);
  config.setArray<uint8_t>(json, "low", 2, this->hourLow);
  config.setArray<uint8_t>(json, "low", 3, this->minuteLow);
  // parsing high key
  config.setArray<int>(json, "high", 0, this->rangeHigh);
  config.setArray<unsigned long>(json, "high", 1, this->timeHigh);
  config.setArray<uint8_t>(json, "high", 2, this->hourHigh);
  config.setArray<uint8_t>(json, "high", 3, this->minuteHigh);
  // parsing type key
  config.setArray<String>(json, "type", 0, this->primaryType);
  config.setArray<String>(json, "type", 1, this->secondaryType);
  INFO_LOG("Builtin actuator configuration loaded");
  return (true);
}

void CoolBoardActuator::printConf() {
  INFO_LOG("Builtin actuator configuration");
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

void CoolBoardActuator::normalAction(float measurment) {
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range HIGH:", this->rangeHigh);
  DEBUG_VAR("Range LOW:", this->rangeLow);
  if (measurment < this->rangeLow) {
    this->state = 1;
    DEBUG_LOG("Actuator ON (sample < rangeLow)");
  } else if (measurment > this->rangeHigh) {
    DEBUG_LOG("Actuator OFF (sample > rangeHigh)");
    this->state = 0;
  }
}

void CoolBoardActuator::invertedAction(float measurment) {
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range HIGH:", this->rangeHigh);
  DEBUG_VAR("Range LOW:", this->rangeLow);
  if (measurment < this->rangeLow) {
    this->state = 0;
    DEBUG_LOG("Actuator OFF (sample < rangeLow)");
  } else if (measurment > this->rangeHigh) {
    this->state = 1;
    DEBUG_LOG("Actuator ON (sample < rangeHigh)");
  }
}

void CoolBoardActuator::temporalActionOff() {
  DEBUG_LOG("Temporal actuator");
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Time active:", this->actifTime);
  DEBUG_VAR("Time HIGH:", this->timeHigh);
  if ((millis() - this->actifTime) >= (this->timeHigh) ||
      this->actifTime == 0) {
    this->state = 0;
    this->actif = 0;
    this->inactifTime = millis();
    DEBUG_LOG("Actuator OFF (time active >= duration HIGH)");
  }
}

void CoolBoardActuator::mixedTemporalActionOff(float measurment) {
  DEBUG_LOG("Mixed temporal actuator");
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range HIGH:", this->rangeHigh);
  DEBUG_VAR("Active time:", this->actifTime);
  DEBUG_VAR("Time HIGH:", this->timeHigh);
  if ((millis() - this->actifTime) >= (this->timeHigh)) {
    if (measurment >= this->rangeHigh) {
      this->state = 0;
      this->actif = 0;
      this->inactifTime = millis();
      DEBUG_LOG("Actuator OFF (value >= range HIGH)");
    } else {
      this->state = 1;
      DEBUG_LOG("Actuator ON (value < range HIGH)");
    }
  }
}

void CoolBoardActuator::temporalActionOn() {
  DEBUG_LOG("Temporal actuator");
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Inactive time:", this->inactifTime);
  DEBUG_VAR("Time LOW:", this->timeLow);
  if ((millis() - this->inactifTime) >= (this->timeLow)) {
    this->state = 1;
    this->actif = 1;
    this->actifTime = millis();
    DEBUG_LOG("Actuator ON (time inactive >= duration LOW)");
  }
}

void CoolBoardActuator::mixedTemporalActionOn(float measurment) {
  DEBUG_LOG("Mixed temporal actuator");
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->rangeLow);
  DEBUG_VAR("Inactive time:", this->inactifTime);
  DEBUG_VAR("Time LOW:", this->timeLow);
  if ((millis() - this->inactifTime) >= (this->timeLow)) {
    if (measurment < this->rangeLow) {
      this->state = 1;
      this->actif = 1;
      this->actifTime = millis();
      DEBUG_LOG("Actuator ON (value < range LOW)");
    } else {
      this->state = 0;
      DEBUG_LOG("Actuator OFF (value >= range LOW)");
    }
  }
}

void CoolBoardActuator::hourAction(uint8_t hour) {
  DEBUG_LOG("Hourly triggered actuator");
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->hourHigh);
  DEBUG_VAR("Hour LOW:", this->hourLow);
  DEBUG_VAR("Inverted flag:", this->inverted);
  if (this->hourHigh < this->hourLow) {
    if (hour >= this->hourLow || hour < this->hourHigh) {
      if (this->inverted) {
        this->state = 1;
      } else {
        this->state = 0;
      }
      DEBUG_LOG("Daymode, actuator OFF");
    } else {
      if (this->inverted) {
        this->state = 0;
      } else {
        this->state = 1;
      }
      DEBUG_LOG("Daymode, actuator ON");
    }
  } else {
    if (hour >= this->hourLow && hour < this->hourHigh) {
      if (this->inverted) {
        this->state = 1;
      } else {
        this->state = 0;
      }
      DEBUG_LOG("Nightmode, actuator OFF");
    } else {
      if (this->inverted) {
        this->state = 0;
      } else {
        this->state = 1;
      }
      DEBUG_LOG("Nightmode, actuator ON");
    }
  }
}

void CoolBoardActuator::mixedHourAction(uint8_t hour, float measurment) {
  DEBUG_LOG("Mixed hourly triggered actuator");
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->hourHigh);
  DEBUG_VAR("Hour LOW:", this->hourLow);
  DEBUG_VAR("Inverted flag:", this->inverted);
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->rangeLow);
  DEBUG_VAR("Range HIGH:", this->rangeHigh);
  if (measurment <= this->rangeLow && this->failsave == true) {
    this->failsave = false;
    WARN_LOG("Resetting failsave for actuator");
  } else if (measurment >= this->rangeHigh && this->failsave == false) {
    this->failsave = true;
    WARN_LOG("Engaging failsave for actuator");
  }

  if (this->hourHigh < this->hourLow) {
    if ((hour >= this->hourLow || hour < this->hourHigh) ||
        this->failsave == true) {
      if (this->inverted) {
        this->state = 1;
      } else {
        this->state = 0;
      }
      DEBUG_LOG("Daymode, turned OFF actuator");
    } else if (this->failsave == false) {
      if (this->inverted) {
        this->state = 0;
      } else {
        this->state = 1;
      }
      DEBUG_LOG("Daymode, turned ON actuator");
    }
  } else {
    if ((hour >= this->hourLow && hour < this->hourHigh) ||
        this->failsave == true) {
      if (this->inverted) {
        this->state = 1;
      } else {
        this->state = 0;
      }
      DEBUG_LOG("Nightmode, turned OFF actuator");
    } else if (this->failsave == false) {
      if (this->inverted) {
        this->state = 0;
      } else {
        this->state = 1;
      }
      DEBUG_LOG("Nightmode, turned ON actuator");
    }
  }
}

void CoolBoardActuator::minuteAction(uint8_t minute) {
  DEBUG_LOG("Minute-wise triggered actuator");
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->minuteHigh);
  DEBUG_VAR("Minute LOW:", this->minuteLow);
  // FIXME: no inverted logic
  // FIXME: what if minuteHigh < minuteLow ?
  if (minute <= this->minuteLow) {
    this->state = 0;
    DEBUG_LOG("Turned OFF actuator (minute <= minute LOW)");
  } else if (minute >= this->minuteHigh) {
    this->state = 1;
    DEBUG_LOG("Turned ON actuator (minute >= minute HIGH)");
  }
}

void CoolBoardActuator::mixedMinuteAction(uint8_t minute, float measurment) {
  DEBUG_LOG("Mixed minute-wise triggered actuator");
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->minuteHigh);
  DEBUG_VAR("Minute LOW:", this->minuteLow);
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->rangeLow);
  DEBUG_VAR("Range HIGH:", this->rangeHigh);
  // FIXME: no inverted logic
  // FIXME: what if minuteHigh < minuteLow ?
  if (minute <= this->minuteLow) {
    if (measurment > this->rangeHigh) {
      this->state = 0;
      DEBUG_LOG("Turned OFF actuator (value > range HIGH)");
    } else {
      this->state = 1;
      DEBUG_LOG("Turned ON actuator (value <= range HIGH)");
    }
  } else if (minute >= this->minuteHigh) {
    if (measurment < this->rangeLow) {
      this->state = 1;
      DEBUG_LOG("Turned ON actuator (value < range LOW)");
    } else {
      this->state = 0;
      DEBUG_LOG("Turned OFF actuator (value >= range LOW)");
    }
  }
}

void CoolBoardActuator::hourMinuteAction(uint8_t hour, uint8_t minute) {
  DEBUG_LOG("Hour:minute triggered actuator");
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->hourHigh);
  DEBUG_VAR("Hour LOW:", this->hourLow);
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->minuteHigh);
  DEBUG_VAR("Minute LOW:", this->minuteLow);
  // FIXME: no inverted logic
  // FIXME: what if hourHigh/minuteHigh < hourLow/minuteLow ?
  if (hour == this->hourLow) {
    if (minute >= this->minuteLow) {
      this->state = 0;
      DEBUG_LOG("Turned OFF actuator (hour:minute > hour:minute LOW)");
    }
  } else if (hour > this->hourLow) {
    this->state = 0;
    DEBUG_LOG("Turned OFF actuator (hour > hour LOW)");
  } else if (hour == this->hourHigh) {
    if (minute >= this->minuteHigh) {
      this->state = 1;
      DEBUG_LOG("Turned ON actuator (hour:minute > hour:minute HIGH)");
    }
  } else if (hour > this->hourHigh) {
    this->state = 1;
    DEBUG_LOG("Turned ON actuator (hour > hour HIGH)");
  }
}

void CoolBoardActuator::mixedHourMinuteAction(uint8_t hour, uint8_t minute,
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
  if (hour == this->hourLow) {
    if (minute >= this->minuteLow) {
      if (measurment >= this->rangeHigh) {
        this->state = 0;
        DEBUG_LOG("Turned OFF actuator (value >= range HIGH)");
      } else {
        this->state = 1;
        DEBUG_LOG("Turned ON actuator (value < range HIGH)");
      }
    }
  } else if (hour > this->hourLow) {
    if (measurment >= this->rangeHigh) {
      this->state = 0;
      DEBUG_LOG("Turned OFF actuator (value >= range HIGH)");
    } else {
      this->state = 1;
      DEBUG_LOG("Turned ON actuator (value < range HIGH)");
    }
  } else if (hour == this->hourHigh) {
    if (minute >= this->minuteHigh) {
      if (measurment < this->rangeLow) {
        this->state = 1;
        DEBUG_LOG("Turned ON actuator (value < range LOW)");
      } else {
        this->state = 0;
        DEBUG_LOG("Turned OFF actuator (value >= range LOW)");
      }
    }
  } else if (hour > this->hourHigh) {
    if (measurment < this->rangeLow) {
      this->state = 1;
      DEBUG_LOG("Turned ON actuator (value < range LOW)");
    } else {
      this->state = 0;
      DEBUG_LOG("Turned OFF actuator (value >= range LOW)");
    }
  }
}