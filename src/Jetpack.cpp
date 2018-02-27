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

#include "CoolConfig.h"
#include "CoolLog.h"
#include "Jetpack.h"

// FIXME: Jepack and CoolBoardActor duplicate logic.
/**
 *  Jetpack::begin():
 *  This method is provided to
 *  initialise the pin that control
 *  the Jetpack shield
 */
void Jetpack::begin() {
  pinMode(EnI2C, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
}

/**
 *  Jetpack::write(action):
 *  This method is provided to write
 *  the given action to the entire Jetpack
 *  action is a Byte (8 bits ), each bit goes
 *  to an output.
 *  MSBFirst
 */
void Jetpack::write(byte action) {
  DEBUG_VAR("Setting actuator pin to:", action);
  this->action = action;
  digitalWrite(EnI2C, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, this->action);
  digitalWrite(EnI2C, HIGH);
}

/**
 *  Jetpack::writeBit(pin,state):
 *  This method is provided to write
 *  the given state to the given pin
 */
void Jetpack::writeBit(byte pin, bool state) {
  DEBUG_VAR("Setting Jetpack actuator pin #:", pin);
  DEBUG_VAR("To state:", state);
  bitWrite(this->action, pin, state);
  digitalWrite(EnI2C, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, this->action);
  digitalWrite(EnI2C, HIGH);
}

/**
 *  Jetpack::doAction(sensor data ):
 *  This method is provided to automate the Jetpack.
 *
 *  The result action is the result of
 *  checking the different flags of an actor
 *  (actif , temporal ,inverted, primaryType
 *  and secondaryType ) and the corresponding
 *  call to the appropriate helping method
 *
 *  \return a string of the current Jetpack state
 *
 */
// FIXME: merge with CoolBoardActor
String Jetpack::doAction(const char *data, int hour, int minute) {
  DEBUG_VAR("input data is:", data);
  DEBUG_VAR("Hour value:", hour);
  DEBUG_VAR("Minute value:", minute);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(data);

  String jetpackState;
  DynamicJsonBuffer jsonBufferOutput;
  JsonObject &rootOutput = jsonBufferOutput.createObject();

  if (!root.success()) {
    ERROR_LOG("Failed to parse jetPack action JSON");
  } else if (!rootOutput.success()) {
    ERROR_LOG("Failed to create actuator action JSON");
  } else {
    // invert the current action state for each actor
    // if the value is outside the limits
    for (int i = 0; i < 8; i++) {
      if (this->actors[i].actif == 1) {
        // check if actor is enabled
        if (this->actors[i].temporal == 0) {
          // normal actor
          if (this->actors[i].inverted == 0) {
            // not inverted actor
            this->normalAction(i,
                               root[this->actors[i].primaryType].as<float>());

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
                  i, hour, minute,
                  root[this->actors[i].primaryType].as<float>());
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
      rootOutput[String("Act") + String(i)] = (bitRead(this->action, i) == 1);
    }
    // FIXME: unneeded ?
    this->write(this->action);
  }
  rootOutput.printTo(jetpackState);
  return (jetpackState);
}

/**
 *  Jetpack::config():
 *  This method is provided to configure the
 *  Jetpack with a configuration file
 *
 *  \return true if successful,false otherwise
 */
bool Jetpack::config() {
  CoolConfig config("/jetPackConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read Jetpack config");
    return (false);
  }
  JsonObject &json = config.get();
  for (int i = 0; i < 8; i++) {
    if (json[String("Act") + String(i)].success()) {
      // parsing actif key
      if (json[String("Act") + String(i)]["actif"].success()) {
        this->actors[i].actif = json[String("Act") + String(i)]["actif"];
      }
      json[String("Act") + String(i)]["actif"] = this->actors[i].actif;

      // parsing temporal key
      if (json[String("Act") + String(i)]["temporal"].success()) {
        this->actors[i].temporal = json[String("Act") + String(i)]["temporal"];
      }
      json[String("Act") + String(i)]["temporal"] = this->actors[i].temporal;

      // parsing inverted key
      if (json[String("Act") + String(i)]["inverted"].success()) {
        this->actors[i].inverted = json[String("Act") + String(i)]["inverted"];
      }
      json[String("Act") + String(i)]["inverted"] = this->actors[i].inverted;

      // parsing inverted key
      if (json[String("Act") + String(i)]["inverted"].success()) {
        this->actors[i].inverted = json[String("Act") + String(i)]["inverted"];
      }
      json[String("Act") + String(i)]["inverted"] = this->actors[i].inverted;

      // parsing low key
      if (json[String("Act") + String(i)]["low"].success()) {
        this->actors[i].rangeLow = json[String("Act") + String(i)]["low"][0];
        this->actors[i].timeLow = json[String("Act") + String(i)]["low"][1];
        this->actors[i].hourLow = json[String("Act") + String(i)]["low"][2];
        this->actors[i].minuteLow = json[String("Act") + String(i)]["low"][3];
      }
      json[String("Act") + String(i)]["low"][0] = this->actors[i].rangeLow;
      json[String("Act") + String(i)]["low"][1] = this->actors[i].timeLow;
      json[String("Act") + String(i)]["low"][2] = this->actors[i].hourLow;
      json[String("Act") + String(i)]["low"][3] = this->actors[i].minuteLow;

      // parsing high key
      if (json[String("Act") + String(i)]["high"].success()) {
        this->actors[i].rangeHigh = json[String("Act") + String(i)]["high"][0];
        this->actors[i].timeHigh = json[String("Act") + String(i)]["high"][1];
        this->actors[i].hourHigh = json[String("Act") + String(i)]["high"][2];
        this->actors[i].minuteHigh = json[String("Act") + String(i)]["high"][3];
      }
      json[String("Act") + String(i)]["high"][0] = this->actors[i].rangeHigh;
      json[String("Act") + String(i)]["high"][1] = this->actors[i].timeHigh;
      json[String("Act") + String(i)]["high"][2] = this->actors[i].hourHigh;
      json[String("Act") + String(i)]["high"][3] = this->actors[i].minuteHigh;

      // parsing type key
      if (json[String("Act") + String(i)]["type"].success()) {
        this->actors[i].primaryType =
            json[String("Act") + String(i)]["type"][0].as<String>();
        this->actors[i].secondaryType =
            json[String("Act") + String(i)]["type"][1].as<String>();
      }
      json[String("Act") + String(i)]["type"][0] = this->actors[i].primaryType;
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
    json[String("Act") + String(i)]["type"][0] = this->actors[i].primaryType;
    json[String("Act") + String(i)]["type"][1] = this->actors[i].secondaryType;
  }

  if (!config.writeJsonToFile()) {
    ERROR_LOG("Failed to save Jetpack config");
    return (false);
  }
  return (true);
}

/**
 *  Jetpack::printConf():
 *  This method is provided to
 *  print the configuration to the
 *  Serial Monitor
 */
void Jetpack::printConf() {
  INFO_LOG("Jetpack configuration ");

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

/**
 *  Jetpack::normalAction(actorNumber , measured value):
 *  This method is provided to
 *  handle normal actors.
 *  it changes the action according to wether the
 *  measured value is: > rangeHigh ( deactivate actor)
 *  or < rangeLow (activate actor )
 */
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

/**
 *  Jetpack::invertedAction(actorNumber , measured value):
 *  This method is provided to
 *  handle inverted actors.
 *  it changes the action according to wether the
 *  measured value is:
 *  > rangeHigh (activate actor)
 *  < rangeLow ( deactivate actor )
 */
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

/**
 *  Jetpack::temporalActionOff(actorNumber ):
 *  This method is provided to
 *  handle temporal actors.
 *  it changes the action according to:
 *
 *  currentTime - startTime > timeHigh : deactivate actor
 *
 */
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

/**
 *  Jetpack::mixedTemporalActionOff(actorNumber, measured value ):
 *  This method is provided to
 *  handle mixed temporal actors.
 *  it changes the action according to:
 *
 *  currentTime - startTime >= timeHigh :
 *    measured value >= rangeHigh : deactivate actor
 *    measured value < rangeHigh : activate actor
 */
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

/**
 *  Jetpack::temporalActionOn(actorNumber ):
 *  This method is provided to
 *  handle temporal actors.
 *  it changes the action according to :
 *
 *  currentTime - stopTime > timeLow : activate actor
 *
 */
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

/**
 *  Jetpack::mixedTemporalActionOn(actorNumber, measured value ):
 *  This method is provided to
 *  handle mixed temporal actors.
 *  it changes the action according to :
 *
 *  currentTime - stopTime > timeLow :
 *    measured value >= rangeLow : deactivate actor
 *    measured value < rangeLow : activate actor
 *
 */
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

/**
 *  Jetpack::hourAction(actorNumber, current hour ):
 *  This method is provided to
 *  handle hour actors.
 *  it changes the action according to:
 *
 *  hour >= hourLow : deactivate the actor
 *  hour >= hourHigh : activate the actor
 *
 */
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

/**
 *  Jetpack::mixedHourAction(actorNumber, current hour, measured value ):
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

/**
 *  Jetpack::minteAction(actorNumber, current minute ):
 *  This method is provided to
 *  handle minute actors.
 *  it changes the action according to:
 *
 *  minute >= minuteLow : deactivate the actor
 *  minute >= minuteHigh : activate the actor
 *
 */
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

/**
 *  Jetpack::mixedMinuteAction(actorNumber, current minute, measured value
 *): This method is provided to handle mixed minute actors. it changes the
 *action according to :
 *
 *  minute >= minuteLow :
 *    -measuredValue >= rangeHigh : deactivate actor
 *    -measured < rangeHigh : activate actor
 *
 *  minute >= minuteHigh :
 *    -measuredValue < rangeLow : activate actor
 *    -measuredValue >=rangeLow : activate actor
 */
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

/**
 *  Jetpack::minteAction(actorNumber, current hour,current minute ):
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

/**
 *  Jetpack::minteAction(actorNumber, current hour,current minute , measured
 *Value ): This method is provided to handle hour minute actors. it changes the
 *action according to:
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
  }
  // start the actor
  else if (hour == this->actors[actorNumber].hourHigh) {
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
