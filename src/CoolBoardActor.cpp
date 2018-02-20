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

#include <ArduinoJson.h>

#include "CoolBoardActor.h"
#include "CoolLog.h"

/**
 *  CoolBoardActor::begin():
 *  This method is provided to
 *  initialise the CoolBoardActor pin
 */
void CoolBoardActor::begin() { pinMode(this->pin, OUTPUT); }

/**
 *  CoolBoardActor::write(action):
 *  This method is provided to write
 *  the given action to the CoolBoardActor.
 *
 */
void CoolBoardActor::write(bool action) {
  DEBUG_VAR("Setting onboard actuator pin to:", action);
  digitalWrite(this->pin, action);
}

/**
 *  CoolBoardActor::doAction(sensor data ):
 *  This method is provided to automate the CoolBoardActor.
 *
 *  The result action is the result of
 *  checking the different flags of the actor
 *  (actif , temporal ,inverted, primaryType
 *  and secondaryType ) and the corresponding
 *  call to the appropriate helping method
 *
 *  \return a string of the actor's state
 *
 */
String CoolBoardActor::doAction(const char *data, int hour, int minute) {
  DEBUG_VAR("input data is:", data);
  DEBUG_VAR("Hour value:", hour);
  DEBUG_VAR("Minute value:", minute);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(data);

  String output;
  DynamicJsonBuffer jsonBufferOutput;
  JsonObject &rootOutput = jsonBuffer.createObject();

  if (!root.success()) {
    ERROR_LOG("Failed to parse actuator action JSON");
  } else if (!rootOutput.success()) {
    ERROR_LOG("Failed to create actuator action JSON");
  } else {
    DEBUG_JSON("Actuator action JSON:", root);

    // invert the current action state for the actor
    // if the value is outside the limits
    if (this->actor.actif == 1) {
      // check if actor is active
      if (this->actor.temporal == 0) {
        // normal actor
        if (this->actor.inverted == 0) {
          // not inverted actor
          this->normalAction(root[this->actor.primaryType].as<float>());
        } else if (this->actor.inverted == 1) {
          // inverted actor
          this->invertedAction(root[this->actor.primaryType].as<float>());
        }
      } else if (this->actor.temporal == 1) {
        // temporal actor
        if (this->actor.secondaryType == "hour") {
          // hour actor
          if (root[this->actor.primaryType].success()) {
            // mixed hour actor
            this->mixedHourAction(hour,
                                  root[this->actor.primaryType].as<float>());
          } else {
            // normal hour actor
            this->hourAction(hour);
            // root[this->actor.secondaryType].as<int>());
          }
        } else if (this->actor.secondaryType == "minute") {
          // minute actor
          if (root[this->actor.primaryType].success()) {
            // mixed minute actor
            this->mixedMinuteAction(minute,
                                    root[this->actor.primaryType].as<float>());
          } else {
            // normal minute actor
            this->minuteAction(minute);
          }
        } else if (this->actor.secondaryType == "hourMinute") {
          // hourMinute actor
          if (root[this->actor.primaryType].success()) {
            // mixed hourMinute actor
            this->mixedHourMinuteAction(
                hour, minute, root[this->actor.primaryType].as<float>());
          } else {
            // normal hourMinute actor
            this->hourMinuteAction(hour, minute);
          }
        } else if (this->actor.secondaryType == "") {
          // normal temporal actor
          if (root[this->actor.primaryType].success()) {
            // mixed temporal actor
            this->mixedTemporalActionOn(
                root[this->actor.primaryType].as<float>());
          } else {
            // normal temporal actor
            this->temporalActionOn();
          }
        }
      }
    } else if (this->actor.actif == 0) {
      // disabled actor
      if (this->actor.temporal == 1) {
        // temporal actor
        if (root[this->actor.primaryType].success()) {
          // mixed temporal actor
          this->mixedTemporalActionOff(
              root[this->actor.primaryType].as<float>());
        } else {
          // normal temporal actor
          this->temporalActionOff();
        }
      }
    }
  }
  rootOutput["ActB"] = digitalRead(this->pin);
  rootOutput.printTo(output);
  return (output);
}

/**
 *  CoolBoardActor::config():
 *  This method is provided to configure the
 *  CoolBoardActor with a configuration file
 *
 *  \return true if successful,false otherwise
 */
bool CoolBoardActor::config() {
  File coolBoardActorConfig = SPIFFS.open("/coolBoardActorConfig.json", "r");

  if (!coolBoardActorConfig) {
    ERROR_LOG("Failed to read /coolBoardActorConfig.json");
    return (false);
  } else {
    size_t size = coolBoardActorConfig.size();

    // Allocate a buffer to store content of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    coolBoardActorConfig.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    if (!json.success()) {
      ERROR_LOG("Failed to parse JSON actuator config from file");
      return (false);
    } else {
      DEBUG_JSON("Actuator config JSON:", json);
      DEBUG_VAR("JSON buffer size:", jsonBuffer.size());
      if (json["actif"].success()) {
        this->actor.actif = json["actif"];
      }
      json["actif"] = this->actor.actif;
      // parsing temporal key
      if (json["temporal"].success()) {
        this->actor.temporal = json["temporal"];
      }
      json["temporal"] = this->actor.temporal;
      // parsing inverted key
      if (json["inverted"].success()) {
        this->actor.inverted = json["inverted"];
      }
      json["inverted"] = this->actor.inverted;

      // parsing low key
      if (json["low"].success()) {
        this->actor.rangeLow = json["low"][0];
        this->actor.timeLow = json["low"][1];
        this->actor.hourLow = json["low"][2];
        this->actor.minuteLow = json["low"][3];
      }
      json["low"][0] = this->actor.rangeLow;
      json["low"][1] = this->actor.timeLow;
      json["low"][2] = this->actor.hourLow;
      json["low"][3] = this->actor.minuteLow;

      // parsing high key
      if (json["high"].success()) {
        this->actor.rangeHigh = json["high"][0];
        this->actor.timeHigh = json["high"][1];
        this->actor.hourHigh = json["high"][2];
        this->actor.minuteHigh = json["high"][3];
      }
      json["high"][0] = this->actor.rangeHigh;
      json["high"][1] = this->actor.timeHigh;
      json["high"][2] = this->actor.hourHigh;
      json["high"][3] = this->actor.minuteHigh;

      // parsing type key
      if (json["type"].success()) {
        this->actor.primaryType = json["type"][0].as<String>();
        this->actor.secondaryType = json["type"][1].as<String>();
      }
      json["type"][0] = this->actor.primaryType;
      json["type"][1] = this->actor.secondaryType;
      coolBoardActorConfig.close();
      coolBoardActorConfig = SPIFFS.open("/coolBoardActorConfig.json", "w");

      if (!coolBoardActorConfig) {
        ERROR_LOG("Failed to write to /coolBoardActorConfig.json");
        return (false);
      }
      json.printTo(coolBoardActorConfig);
      coolBoardActorConfig.close();
      DEBUG_JSON("Saved actuator config to /coolBoardActorConfig.json", json);
      return (true);
    }
  }
}

/**
 *  CoolBoardActor::printConf():
 *  This method is provided to
 *  print the configuration to the
 *  Serial Monitor
 */
void CoolBoardActor::printConf() {
  INFO_LOG("Actuator configuration:");
  INFO_VAR("  Actif          = ", this->actor.actif);
  INFO_VAR("  Temporal       = ", this->actor.temporal);
  INFO_VAR("  Inverted       = ", this->actor.inverted);
  INFO_VAR("  Primary type   = ", this->actor.primaryType);
  INFO_VAR("  Secondary type = ", this->actor.secondaryType);
  INFO_VAR("  Range low      = ", this->actor.rangeLow);
  INFO_VAR("  Time low       = ", this->actor.timeLow);
  INFO_VAR("  Hour low       = ", this->actor.hourLow);
  INFO_VAR("  Minute low     = ", this->actor.minuteLow);
  INFO_VAR("  Range high     = ", this->actor.rangeHigh);
  INFO_VAR("  Time high      = ", this->actor.timeHigh);
  INFO_VAR("  Hour high      = ", this->actor.hourHigh);
  INFO_VAR("  Minute high    = ", this->actor.minuteHigh);
}

/**
 *  CoolBoardActor::normalAction( measured value):
 *  This method is provided to
 *  handle normal actors.
 *  it changes the action according to wether the
 *  measured value is: > rangeHigh ( deactivate actor)
 *  or < rangeLow (activate actor )
 */
void CoolBoardActor::normalAction(float measurment) {
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range HIGH:", this->actor.rangeHigh);
  DEBUG_VAR("Range LOW:", this->actor.rangeLow);

  if (measurment < this->actor.rangeLow) {
    // measured value lower than minimum range : activate actor
    this->write(1);
    DEBUG_LOG("Actuator ON (sample < rangeLow)");
  } else if (measurment > this->actor.rangeHigh) {
    // measured value higher than maximum range : deactivate actor
    DEBUG_LOG("Actuator OFF (sample > rangeHigh)");
    this->write(0);
  }
}

/**
 *  CoolBoardActor::invertedAction( measured value):
 *  This method is provided to
 *  handle inverted actors.
 *  it changes the action according to wether the
 *  measured value is:
 *  > rangeHigh (activate actor)
 *  < rangeLow ( deactivate actor )
 */
void CoolBoardActor::invertedAction(float measurment) {
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range HIGH:", this->actor.rangeHigh);
  DEBUG_VAR("Range LOW:", this->actor.rangeLow);

  if (measurment < this->actor.rangeLow) {
    // measured value lower than minimum range : deactivate actor
    this->write(0);
    DEBUG_LOG("Actuator OFF (sample < rangeLow)");
  } else if (measurment > this->actor.rangeHigh) {
    // measured value higher than maximum range : activate actor
    this->write(1);
    DEBUG_LOG("Actuator ON (sample < rangeHigh)");
  }
}

/**
 *  CoolBoardActor::temporalActionOff( ):
 *  This method is provided to
 *  handle temporal actors.
 *  it changes the action according to:
 *
 *  currentTime - startTime > timeHigh : deactivate actor
 *
 */
void CoolBoardActor::temporalActionOff() {
  DEBUG_LOG("Temporal actuator");
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Time active:", this->actor.actifTime);
  DEBUG_VAR("Time HIGH:", this->actor.timeHigh);

  if ((millis() - this->actor.actifTime) >= (this->actor.timeHigh)) {
    // stop the actor
    this->write(0);
    // make the actor inactif:
    this->actor.actif = 0;
    // start the low timer
    this->actor.inactifTime = millis();
    DEBUG_LOG("Actuator OFF (time active >= duration HIGH)");
  }
}

/**
 *  CoolBoardActor::mixedTemporalActionOff( measured value ):
 *  This method is provided to
 *  handle mixed temporal actors.
 *  it changes the action according to:
 *
 *  currentTime - startTime >= timeHigh :
 *    measured value >= rangeHigh : deactivate actor
 *    measured value < rangeHigh : activate actor
 */
void CoolBoardActor::mixedTemporalActionOff(float measurment) {
  DEBUG_LOG("Mixed temporal actuator");
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range HIGH:", this->actor.rangeHigh);
  DEBUG_VAR("Active time:", this->actor.actifTime);
  DEBUG_VAR("Time HIGH:", this->actor.timeHigh);

  if ((millis() - this->actor.actifTime) >= (this->actor.timeHigh)) {
    if (measurment >= this->actor.rangeHigh) {
      // stop the actor
      this->write(0);
      // make the actor inactif:
      this->actor.actif = 0;
      // start the low timer
      this->actor.inactifTime = millis();
      DEBUG_LOG("Actuator OFF (value >= range HIGH)");
    } else {
      this->write(1);
      DEBUG_LOG("Actuator ON (value < range HIGH)");
    }
  }
}

/**
 *  CoolBoardActor::temporalActionOn( ):
 *  This method is provided to
 *  handle temporal actors.
 *  it changes the action according to :
 *
 *  currentTime - stopTime > timeLow : activate actor
 *
 */
void CoolBoardActor::temporalActionOn() {
  DEBUG_LOG("Temporal actuator");
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Inactive time:", this->actor.inactifTime);
  DEBUG_VAR("Time LOW:", this->actor.timeLow);

  if ((millis() - this->actor.inactifTime) >= (this->actor.timeLow)) {
    // start the actor
    this->write(1);
    // make the actor actif:
    this->actor.actif = 1;
    // start the low timer
    this->actor.actifTime = millis();
    DEBUG_LOG("Actuator ON (time inactive >= duration LOW)");
  }
}

/**
 *  CoolBoardActor::mixedTemporalActionOn( measured value ):
 *  This method is provided to
 *  handle mixed temporal actors.
 *  it changes the action according to :
 *
 *  currentTime - stopTime > timeLow :
 *    measured value >= rangeLow : deactivate actor
 *    measured value < rangeLow : activate actor
 *
 */
void CoolBoardActor::mixedTemporalActionOn(float measurment) {
  DEBUG_LOG("Mixed temporal actuator");
  DEBUG_VAR("Current millis:", millis());
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->actor.rangeLow);
  DEBUG_VAR("Inactive time:", this->actor.inactifTime);
  DEBUG_VAR("Time LOW:", this->actor.timeLow);

  if ((millis() - this->actor.inactifTime) >= (this->actor.timeLow)) {
    if (measurment < this->actor.rangeLow) {
      // start the actor
      this->write(1);
      // make the actor actif:
      this->actor.actif = 1;
      // start the low timer
      this->actor.actifTime = millis();
      DEBUG_LOG("Actuator ON (value < range LOW)");
    } else {
      this->write(0);
      DEBUG_LOG("Actuator OFF (value >= range LOW)");
    }
  }
}

/**
 *  CoolBoardActor::hourAction( current hour ):
 *  This method is provided to
 *  handle hour actors.
 *  it changes the action according to:
 *
 *  hour >= hourLow : deactivate the actor
 *  hour >= hourHigh : activate the actor
 *
 */
void CoolBoardActor::hourAction(int hour) {
  DEBUG_LOG("Hourly triggered actuator");
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->actor.hourHigh);
  DEBUG_VAR("Hour LOW:", this->actor.hourLow);
  DEBUG_VAR("Inverted flag:", this->actor.inverted);

  if (this->actor.hourHigh < this->actor.hourLow) {
    if (hour >= this->actor.hourLow || hour < this->actor.hourHigh) {
      // stop the actor
      if (this->actor.inverted) {
        this->write(1);
      } else {
        this->write(0);
      }
      DEBUG_LOG("Daymode, onboard actuator OFF");
    } else {
      // start the actor
      if (this->actor.inverted) {
        this->write(0);
      } else {
        this->write(1);
      }
      DEBUG_LOG("Daymode, onboard actuator ON");
    }
  } else {
    if (hour >= this->actor.hourLow && hour < this->actor.hourHigh) {
      // stop the actor in "night mode", i.e. a light that is on at night
      if (this->actor.inverted) {
        this->write(1);
      } else {
        this->write(0);
      }
      DEBUG_LOG("Nightmode, onboard actuator OFF");
    } else {
      // starting the actor
      if (this->actor.inverted) {
        this->write(0);
      } else {
        this->write(1);
      }
      DEBUG_LOG("Nightmode, onboard actuator ON");
    }
  }
}

/**
 *  CoolBoardActor::mixedHourAction( current hour, measured value ):
 *  This method is provided to
 *  handle mixed hour actors.
 *  it changes the action according to :
 *
 *  hour >= hourLow :
 *    -measuredValue >= rangeHigh : deactivate actor
 *    -measured < rangeHigh : activate actor
 *
 *  hour >= hourHigh :
 *    -measuredValue < rangeLow : activate actor
 *    -measuredValue >=rangeLow : activate actor
 */
void CoolBoardActor::mixedHourAction(int hour, float measurment) {
  DEBUG_LOG("Mixed hourly triggered actuator");
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->actor.hourHigh);
  DEBUG_VAR("Hour LOW:", this->actor.hourLow);
  DEBUG_VAR("Inverted flag:", this->actor.inverted);
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->actor.rangeLow);
  DEBUG_VAR("Range HIGH:", this->actor.rangeHigh);

  if (measurment <= this->actor.rangeLow && this->actor.failsave == true) {
    this->actor.failsave = false;
    WARN_LOG("Resetting failsave for onboard actuator");
  } else if (measurment >= this->actor.rangeHigh &&
             this->actor.failsave == false) {
    this->actor.failsave = true;
    WARN_LOG("Engaging failsave for onboard actuator");
  }

  if (this->actor.hourHigh < this->actor.hourLow) {
    if ((hour >= this->actor.hourLow || hour < this->actor.hourHigh) ||
        this->actor.failsave == true) {
      // stop the actor
      if (this->actor.inverted) {
        this->write(1);
      } else {
        this->write(0);
      }
      DEBUG_LOG("Daymode, turned OFF onboard actuator");
    } else if (this->actor.failsave == false) {
      // starting the actor
      if (this->actor.inverted) {
        this->write(0);
      } else {
        this->write(1);
      }
      DEBUG_LOG("Daymode, turned ON onboard actuator");
    }
  } else {
    if ((hour >= this->actor.hourLow && hour < this->actor.hourHigh) ||
        this->actor.failsave == true) {
      // stop the actor in Nght mode ie a light that is on over night
      if (this->actor.inverted) {
        this->write(1);
      } else {
        this->write(0);
      }
      DEBUG_LOG("Nightmode, turned OFF onboard actuator");
    } else if (this->actor.failsave == false) {
      // starting the actor
      if (this->actor.inverted) {
        this->write(0);
      } else {
        this->write(1);
      }
      DEBUG_LOG("Nightmode, turned ON onboard actuator");
    }
  }
}

/**
 *  CoolBoardActor::minteAction( current minute ):
 *  This method is provided to
 *  handle minute actors.
 *  it changes the action according to:
 *
 *  minute >= minuteLow : deactivate the actor
 *  minute >= minuteHigh : activate the actor
 *
 */
void CoolBoardActor::minuteAction(int minute) {
  DEBUG_LOG("Minute-wise triggered onboard actuator");
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->actor.minuteHigh);
  DEBUG_VAR("Minute LOW:", this->actor.minuteLow);

  // FIXME: no inverted logic
  // FIXME: what if minuteHigh < minuteLow ?
  if (minute <= this->actor.minuteLow) {
    // stop the actor
    this->write(0);
    DEBUG_LOG("Turned OFF onboard actuator (minute <= minute LOW)");

  } else if (minute >= this->actor.minuteHigh) {
    // starting the actor
    this->write(1);
    DEBUG_LOG("Turned ON onboard actuator (minute >= minute HIGH)");
  }
}

/**
 *  CoolBoardActor::mixedMinuteAction( current minute, measured value ):
 *  This method is provided to
 *  handle mixed minute actors.
 *  it changes the action according to :
 *
 *  minute >= minuteLow :
 *    -measuredValue >= rangeHigh : deactivate actor
 *    -measured < rangeHigh : activate actor
 *
 *  minute >= minuteHigh :
 *    -measuredValue < rangeLow : activate actor
 *    -measuredValue >=rangeLow : activate actor
 */
void CoolBoardActor::mixedMinuteAction(int minute, float measurment) {
  DEBUG_LOG("Mixed minute-wise triggered onboard actuator");
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->actor.minuteHigh);
  DEBUG_VAR("Minute LOW:", this->actor.minuteLow);
  DEBUG_VAR("Sensor value:", measurment);
  DEBUG_VAR("Range LOW:", this->actor.rangeLow);
  DEBUG_VAR("Range HIGH:", this->actor.rangeHigh);

  // FIXME: no inverted logic
  // FIXME: what if minuteHigh < minuteLow ?
  if (minute <= this->actor.minuteLow) {
    if (measurment > this->actor.rangeHigh) {
      this->write(0);
      DEBUG_LOG("Turned OFF onboard actuator (value > range HIGH)");
    } else {
      this->write(1);
      DEBUG_LOG("Turned ON onboard actuator (value <= range HIGH)");
    }
  } else if (minute >= this->actor.minuteHigh) {
    if (measurment < this->actor.rangeLow) {
      this->write(1);
      DEBUG_LOG("Turned ON onboard actuator (value < range LOW)");
    } else {
      this->write(0);
      DEBUG_LOG("Turned OFF onboard actuator (value >= range LOW)");
    }
  }
}

/**
 *  CoolBoardActor::minteAction( current hour,current minute ):
 *  This method is provided to
 *  handle hour minute actors.
 *  it changes the action according to:
 *
 *  hour == hourLow :
 *    minute >= minuteLow : deactivate the actor
 *
 *  hour >  hourLow : deactivate the actor
 *
 *  hour == hourHigh :
 *    minute >= minteHigh : activate the actor
 *
 *  hour >  hourHigh : activate the actor
 */
void CoolBoardActor::hourMinuteAction(int hour, int minute) {
  DEBUG_LOG("Hour:minute triggered onboard actuator");
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->actor.hourHigh);
  DEBUG_VAR("Hour LOW:", this->actor.hourLow);
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->actor.minuteHigh);
  DEBUG_VAR("Minute LOW:", this->actor.minuteLow);

  // FIXME: no inverted logic
  // FIXME: what if hourHigh/minuteHigh < hourLow/minuteLow ?
  if (hour == this->actor.hourLow) {
    if (minute >= this->actor.minuteLow) {
      this->write(0);
      DEBUG_LOG("Turned OFF onboard actuator (hour:minute > hour:minute LOW)");
    }
  } else if (hour > this->actor.hourLow) {
    this->write(0);
    DEBUG_LOG("Turned OFF onboard actuator (hour > hour LOW)");
  } else if (hour == this->actor.hourHigh) {
    if (minute >= this->actor.minuteHigh) {
      this->write(1);
      DEBUG_LOG("Turned ON onboard actuator (hour:minute > hour:minute HIGH)");
    }
  } else if (hour > this->actor.hourHigh) {
    this->write(1);
    DEBUG_LOG("Turned ON onboard actuator (hour > hour HIGH)");
  }
}

/**
 *  CoolBoardActor::minteAction( current hour,current minute , measured
 *Value ): This method is provided to handle hour minute actors. it changes
 *the action according to:
 *
 *  hour == hourLow :
 *    minute >= minuteLow :
 *      measuredValue >= rangeHigh : deactivate actor
 *      measuredValue < rangeHigh : activate actor
 *
 *  hour >  hourLow :
 *    measuredValue >= rangeHigh : deactivate actor
 *    measuredValue < rangeHigh : activate actor
 *
 *  hour == hourHigh :
 *    minute >= minteHigh :
 *      measuredValue >= rangeLow : deactivate actor
 *      measuredValue < rangeLow : activate actor
 *
 *  hour >  hourHigh :
 *    measuredValue >= rangeLow : deactivate actor
 *    measuredValue < rangeLow : activate actor
 *
 */
void CoolBoardActor::mixedHourMinuteAction(int hour, int minute,
                                           float measurment) {
  DEBUG_LOG("Mixed Hour:minute triggered onboard actuator");
  DEBUG_VAR("Current hour:", hour);
  DEBUG_VAR("Hour HIGH:", this->actor.hourHigh);
  DEBUG_VAR("Hour LOW:", this->actor.hourLow);
  DEBUG_VAR("Current minute:", minute);
  DEBUG_VAR("Minute HIGH:", this->actor.minuteHigh);
  DEBUG_VAR("Minute LOW:", this->actor.minuteLow);
  DEBUG_VAR("Measured value:", measurment);
  DEBUG_VAR("Range LOW:", this->actor.rangeLow);
  DEBUG_VAR("Range HIGH:", this->actor.rangeHigh);

  // stop the actor
  if (hour == this->actor.hourLow) {
    if (minute >= this->actor.minuteLow) {
      if (measurment >= this->actor.rangeHigh) {
        this->write(0);
        DEBUG_LOG("Turned OFF onboard actuator (value >= range HIGH)");
      } else {
        this->write(1);
        DEBUG_LOG("Turned ON onboard actuator (value < range HIGH)");
      }
    }
  } else if (hour > this->actor.hourLow) {
    if (measurment >= this->actor.rangeHigh) {
      this->write(0);
      DEBUG_LOG("Turned OFF onboard actuator (value >= range HIGH)");
    } else {
      this->write(1);
      DEBUG_LOG("Turned ON onboard actuator (value < range HIGH)");
    }
  }
  // start the actor
  else if (hour == this->actor.hourHigh) {
    if (minute >= this->actor.minuteHigh) {
      if (measurment < this->actor.rangeLow) {
        this->write(1);
        DEBUG_LOG("Turned ON onboard actuator (value < range LOW)");
      } else {
        this->write(0);
        DEBUG_LOG("Turned OFF onboard actuator (value >= range LOW)");
      }
    }
  } else if (hour > this->actor.hourHigh) {
    if (measurment < this->actor.rangeLow) {
      this->write(1);
      DEBUG_LOG("Turned ON onboard actuator (value < range LOW)");
    } else {
      this->write(0);
      DEBUG_LOG("Turned OFF onboard actuator (value >= range LOW)");
    }
  }
}
