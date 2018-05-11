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
#include <Arduino.h>
#include <math.h>

#include "ArduinoJson.h"

#include "Irene3000.h"
#include "CoolLog.h"
#include "CoolBoardLed.h"

void Irene3000::begin() {
  this->ads.begin();
  delay(2000);
}

void Irene3000::waitForButtonPress() {
  while (!this->isButtonPressed()) {
    delay(1000);
  }
}

bool Irene3000::isButtonPressed() {
  int bValue = this->readButton();

  if (bValue < 2000) {
    return true;
  }
  return false;
}

void Irene3000::calibrate(CoolBoardLed &led) {
  led.write(WHITE);
  INFO_LOG("IRN3000 starting, hold button to calibrate the pH probe");
  delay(2000);

  if (this->isButtonPressed()) {
    led.write(BRIGHT_WHITE);
    delay(5000);
    led.write(GREEN);
    INFO_LOG("Ready to calibrate, hold button to start pH7 calibration");
    this->waitForButtonPress();
    led.write(BRIGHT_GREEN);
    INFO_LOG("Starting pH 7 calibration for 30 seconds");
    delay(30000);
    this->calibratepH7();
    led.write(RED);
    INFO_LOG("ph7 calibration finished, hold button to start pH4 calibration");
    this->waitForButtonPress();
    led.write(FUCHSIA);
    INFO_LOG("Starting pH 4 calibration for 30 seconds");
    delay(30000);
    this->calibratepH4();
    this->saveParams();
    led.write(WHITE);
    INFO_LOG("Calibration finished, hold button to exit calibration");
    this->waitForButtonPress();
  }
}

void Irene3000::read(JsonObject &root) {
  if (waterTemp.active) {
    root["waterTemp"] = this->readTemp();
    if (phProbe.active) {
      root["ph"] = this->readPh(root["waterTemp"].as<double>());
    }
  }
  if (adc2.active) {
    root[adc2.type] = this->readADSChannel2(adc2.gain);
  }
  DEBUG_JSON("Irene data:", root);
}

bool Irene3000::config() {
  File configFile = SPIFFS.open("/irene3000Config.json", "r");

  if (!configFile) {
    ERROR_LOG("Failed to read /irene3000Config.json");
    return (false);
  } else {
    uint16_t tempGain;

    String data = configFile.readString();
    DynamicJsonBuffer fileBuffer;
    JsonObject &json = fileBuffer.parseObject(data);

    if (!json.success()) {
      ERROR_LOG("Failed to parse IRN3000 config from file");
      return (false);
    } else {
      DEBUG_JSON("IRN3000 config JSON:", json);

      if (json["waterTemp"]["active"].success()) {
        this->waterTemp.active = json["waterTemp"]["active"];
      }
      json["waterTemp"]["active"] = this->waterTemp.active;

      if (json["phProbe"]["active"].success()) {
        this->phProbe.active = json["phProbe"]["active"];
      }
      json["phProbe"]["active"] = this->phProbe.active;

      if (json["adc2"]["active"].success()) {
        this->adc2.active = json["adc2"]["active"];
      }
      json["adc2"]["active"] = this->adc2.active;

      if (json["adc2"]["gain"].success()) {
        tempGain = json["adc2"]["gain"];
        this->adc2.gain = this->gainConvert(tempGain);
      }
      json["adc2"]["gain"] = this->adc2.gain;

      if (json["adc2"]["type"].success()) {
        this->adc2.type = json["adc2"]["type"].as<String>();
      }
      json["adc2"]["type"] = this->adc2.type;

      if (json["pH7Cal"].success()) {
        this->params.pH7Cal = json["pH7Cal"];
      }
      json["pH7Cal"] = this->params.pH7Cal;

      if (json["pH4Cal"].success()) {
        this->params.pH4Cal = json["pH4Cal"];
      }
      json["pH4Cal"] = this->params.pH4Cal;

      if (json["pHStep"].success()) {
        this->params.pHStep = json["pHStep"];
      }
      json["pHStep"] = this->params.pHStep;
      configFile.close();
      configFile = SPIFFS.open("/irene3000Config.json", "w");

      if (!configFile) {
        ERROR_LOG("Failed to write to /irene3000Config.json");
        return (false);
      }
      json.printTo(configFile);
      configFile.close();
      DEBUG_LOG("Saved IRN3000 config to /irene3000Config.json");
      return (true);
    }
  }
}

void Irene3000::printConf() {
  DEBUG_LOG("IRN3000 configuration");
  DEBUG_VAR("  Temperature enabled:", waterTemp.active);
  DEBUG_NBR("  Temperature gain   :", waterTemp.gain, HEX);
  DEBUG_VAR("  pH probe enabled   :", phProbe.active);
  DEBUG_NBR("  pH probe gain      :", phProbe.gain, HEX);
  DEBUG_VAR("  ADC2 enabled       :", adc2.active);
  DEBUG_NBR("  ADC2 gain          :", adc2.gain, HEX);
  DEBUG_VAR("  ADC2 type          :", adc2.type);
}

int Irene3000::readButton() {
  this->setGain(GAIN_TWOTHIRDS);
  int result = this->ads.readADC_SingleEnded(BUTTON_CHANNEL);
  DEBUG_VAR("Button value:", result);
  return (result);
}

void Irene3000::setGain(adsGain_t gain) {
  this->ads.setGain(gain);
}

int Irene3000::readADSChannel2(adsGain_t gain) {
  this->setGain(gain);
  int result = this->ads.readADC_SingleEnded(FREE_ADC_CHANNEL);
  DEBUG_VAR("ADC2 value:", result);
  return (result);
}

float Irene3000::readPh(double t) {
  // FIXME: Magic numbers
  this->setGain(GAIN_FOUR);

  int adcR = ads.readADC_SingleEnded(PH_CHANNEL);
  double Voltage = REFERENCE_VOLTAGE_GAIN_4 * (adcR) / ADC_MAXIMUM_VALUE;
  float miliVolts = Voltage * 1000;
  float temporary =
      ((((REF_VOLTAGE * (float)params.pH7Cal) / 32767) * 1000) - miliVolts) /
      OPAMP_GAIN;
  float phT = 7 - (temporary / params.pHStep);

  DEBUG_VAR("pH value:", phT);
  if (isnan(phT)) {
    // FIXME: No, returning NaN is better!
    return (-42);
  }
  return (phT);
}

double Irene3000::readTemp() {
  // FIXME: Magic numbers
  const double A = 3.9083E-3;
  const double B = -5.775E-7;
  double T;

  this->setGain(GAIN_EIGHT);

  double adc0 = ads.readADC_SingleEnded(TEMP_CHANNEL);
  double R = ((adc0 * V_GAIN_8) / 0.095) / 1000;

  T = 0.0 - A;
  T += sqrt((A * A) - 4.0 * B * (1.0 - R));
  T /= (2.0 * B);

  if (T > 0 && T < 200) {
    DEBUG_VAR("IRN3000 temperature in °C: ", T);
    if (isnan(T)) {
      // FIXME: No, returning NaN is better!
      return (-300);
    }
    return T;
  } else {
    T = 0.0 - A;
    T -= sqrt((A * A) - 4.0 * B * (1.0 - R));
    T /= (2.0 * B);
    DEBUG_VAR("IRN3000 temperature in °C:", T);
    if (isnan(T)) {
      // FIXME: No, returning NaN is better!
      return (-400);
    }
    return T;
  }
}

void Irene3000::calibratepH7() {
  delay(1000);

  this->setGain(GAIN_FOUR);
  this->params.pH7Cal = ads.readADC_SingleEnded(PH_CHANNEL);
  this->calcpHSlope();
}

void Irene3000::calibratepH4() {
  delay(1000);

  this->setGain(GAIN_FOUR);
  this->params.pH4Cal = ads.readADC_SingleEnded(PH_CHANNEL);
  this->calcpHSlope();
}

void Irene3000::calcpHSlope() {
  params.pHStep =
      ((((REF_VOLTAGE * (float)(params.pH7Cal - params.pH4Cal)) / 32767) * 1000) /
       OPAMP_GAIN) / 3;
}

void Irene3000::resetParams(void) {
  params.pH7Cal = 16384; // assume ideal probe and amp conditions 1/2 of 4096
  params.pH4Cal = 8192;  // using ideal probe slope we end up this many 12bit
                         // units away on the 4 scale
  params.pHStep = 59.16; // ideal probe slope
}

adsGain_t Irene3000::gainConvert(uint16_t tempGain) {
  switch (tempGain) {
  case (1):
    return (GAIN_ONE);
  case (2):
    return (GAIN_TWO);
  case (4):
    return (GAIN_FOUR);
  case (8):
    return (GAIN_EIGHT);
  case (16):
    return (GAIN_SIXTEEN);
  default:
    return (GAIN_TWOTHIRDS);
  }
  return (GAIN_ONE);
}

bool Irene3000::saveParams() {
  File irene3000Config = SPIFFS.open("/irene3000Config.json", "r");

  if (!irene3000Config) {
    ERROR_LOG("Failed to read /irene3000Config.json");
    return (false);
  } else {
    size_t size = irene3000Config.size();
    std::unique_ptr<char[]> buf(new char[size]);

    irene3000Config.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    if (!json.success()) {
      DEBUG_LOG("Failed to parse JSON IRN3000 config from file");
      return (false);
    } else {
      DEBUG_JSON("IRN3000 config JSON:", json);
      json["pH7Cal"] = this->params.pH7Cal;
      json["pH4Cal"] = this->params.pH4Cal;
      json["pHStep"] = this->params.pHStep;
      irene3000Config.close();
      irene3000Config = SPIFFS.open("/irene3000Config.json", "w");
      if (!irene3000Config) {
        ERROR_LOG("Failed to write to /irene3000Config.json");
        return (false);
      }
      json.printTo(irene3000Config);
      irene3000Config.close();
      DEBUG_LOG("Saved config to /irene3000Config.json");
      return (true);
    }
  }
}