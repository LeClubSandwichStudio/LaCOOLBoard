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
#include <math.h>

#include "ArduinoJson.h"

#include "CoolBoardLed.h"
#include "CoolConfig.h"
#include "CoolLog.h"
#include "CoolTime.h"
#include "Irene3000.h"

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
    led.write(BRIGHT_RED);
    INFO_LOG("Starting pH 4 calibration for 30 seconds");
    delay(30000);
    this->calibratepH4();
    this->saveCalibrationDate();
    this->config(true);
    led.write(FUCHSIA);
    INFO_LOG("Calibration finished, hold button to exit calibration");
    this->waitForButtonPress();
    led.write(OFF);
  }
}

void Irene3000::read(JsonObject &root) {
  if (waterTemp.active) {
    root.createNestedObject("PT1000");
    this->readTemp(root["PT1000"]);
    if (phProbe.active) {
      root.createNestedObject("phProbe");
      this->readPh(root["phProbe"]);
      // readLastCalibrationDate(root);
    }
  }
  if (adc2.active) {
    root.createNestedObject("adc2");
    if (this->adc2.type == "DFrobotEC") {
      this->readEC(root["adc2"]);
    } else {
      root["adc2"][adc2.type] = this->readADSChannel2();
    }
  }
  DEBUG_JSON("Irene data:", root);
}

bool Irene3000::config(bool overwrite) {
  CoolConfig config("/sensors.json");
  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read /sensors.json");
    return (false);
  }
  JsonObject &sensors = config.get();
  JsonArray &root = sensors["sensors"];
  int i = 0;
  for (auto kv : root) {
    JsonArray &measures = kv["measures"];
    JsonArray &utils = kv["utils"];
    if (kv["key"] == "PT1000") {
      for (auto measure : measures) {
        if (measure == "waterTemp") {
          if (overwrite) {
            CoolConfig::setArray<bool>(kv, "measures", i,
                                       this->waterTemp.active, overwrite);
          } else {
            this->waterTemp.active = 1;
          }
        }
      }
    } else if (kv["key"] == "phProbe") {
      for (auto measure : measures) {
        if (measure == "phProbe") {
          if (overwrite) {
            CoolConfig::setArray<bool>(kv, "measures", i, this->phProbe.active,
                                       overwrite);
          } else {
            this->phProbe.active = 1;
          }
        }
        for (auto util : utils) {
          if (util == "ph4Cal") {
            CoolConfig::set<int>(kv["utils"], "ph4Cal", this->params.pH4Cal,
                                 overwrite);
          } else if (util == "ph7Cal") {
            CoolConfig::set<int>(kv["utils"], "ph7Cal", this->params.pH7Cal,
                                 overwrite);
          } else if (util == "phStep") {
            CoolConfig::set<float>(kv["utils"], "phStep",
                                   this->params.pHStep, overwrite);
          } else if (util == "calibrationDate") {
            CoolConfig::set<String>(kv["utils"], "calbrationDate",
                                    this->params.calibrationDate, overwrite);
          }
        }
      }
    } else if (kv["key"] == "adc2") {
      for (auto measure : measures) {
          CoolConfig::setArray<String>(kv, "measures", 0, this->adc2.type, overwrite);
          this->adc2.active = 1;
      }
      for (auto util : utils) {
        if (util == "gain") {
          CoolConfig::set<int>(kv["utils"], "gain", this->adc2.gain, overwrite);
        }
      }
    }
    i++;
  }
  DEBUG_LOG("IRN3000 configuration loaded");
  return (true);
}

void Irene3000::printConf() {
  DEBUG_LOG("IRN3000 configuration");
  DEBUG_VAR("  Temperature enabled:", waterTemp.active);
  DEBUG_NBR("  Temperature gain   :", waterTemp.gain, HEX);
  DEBUG_VAR("  pH probe enabled   :", phProbe.active);
  DEBUG_NBR("  pH probe gain      :", phProbe.gain, HEX);
  DEBUG_VAR("  Last PH calibration:", params.calibrationDate);
  DEBUG_VAR("  ADC2 enabled       :", adc2.active);
  DEBUG_NBR("  ADC2 gain          :", adc2.gain, HEX);
  DEBUG_VAR("  ADC2 type          :", adc2.type);
}

int Irene3000::readButton() {
  this->ads.setGain(GAIN_TWOTHIRDS);
  int result = this->ads.readADC_SingleEnded(BUTTON_CHANNEL);
  DEBUG_VAR("Button value:", result);
  return (result);
}

int Irene3000::readADSChannel2() {
  this->ads.setGain(this->gainConvert(this->adc2.gain));
  int result = this->ads.readADC_SingleEnded(FREE_ADC_CHANNEL);
  DEBUG_VAR("ADC2 value:", result);
  return (result);
}

void Irene3000::readPh(JsonObject &root) {
  this->ads.setGain(GAIN_FOUR);

  int adcR = ads.readADC_SingleEnded(PH_CHANNEL);
  double Voltage = REFERENCE_VOLTAGE_GAIN_4 * (adcR) / ADC_MAXIMUM_VALUE;
  float miliVolts = Voltage * 1000;
  float temporary =
      ((((REF_VOLTAGE * (float)params.pH7Cal) / 32767) * 1000) - miliVolts) /
      OPAMP_GAIN;
  float phT = 7 - (temporary / params.pHStep);

  DEBUG_VAR("ph value:", phT);
  if (isnan(phT)) {
    root["ph"] = RawJson("null");
  } else
    root["ph"] = phT;
}

void Irene3000::readTemp(JsonObject &root) {
  const double A = 3.9083E-3;
  const double B = -5.775E-7;
  double T;

  this->ads.setGain(GAIN_EIGHT);

  double adc0 = ads.readADC_SingleEnded(TEMP_CHANNEL);
  double R = ((adc0 * V_GAIN_8) / 0.095) / 1000;

  T = 0.0 - A;
  T += sqrt((A * A) - 4.0 * B * (1.0 - R));
  T /= (2.0 * B);

  DEBUG_VAR("IRN3000 temperature in °C: ", T);
  if (T > 0 && T < 200) {
    root["waterTemp"] = T;
  } else {
    root["waterTemp"] = RawJson("null");
  }
}

float Irene3000::readTemp() {
  const double A = 3.9083E-3;
  const double B = -5.775E-7;
  double T;

  this->ads.setGain(GAIN_EIGHT);

  double adc0 = ads.readADC_SingleEnded(TEMP_CHANNEL);
  double R = ((adc0 * V_GAIN_8) / 0.095) / 1000;

  T = 0.0 - A;
  T += sqrt((A * A) - 4.0 * B * (1.0 - R));
  T /= (2.0 * B);
  DEBUG_VAR("IRN3000 temperature in °C:", T);
  return T;
}

void Irene3000::readEC(JsonObject &root) {
  int overSample = 16;
  float ecCurrent = 0;
  unsigned long average = 0;
  unsigned int averageVoltage = 0;
  // Oversample 64 time to get a good reading
  for (int i = 0; i <= 64; i++) {
    delay(20);
    average += readADSChannel2();
  }
  average = average / 64;
  // resolution for ADS1115 is 0.1875 uV per tick
  averageVoltage = average * 0.1875;
  DEBUG_VAR("Average RAW : ", average);
  DEBUG_VAR("Average Voltage : ", averageVoltage);
  // temperature compensation formula: fFinalResult(25^C) =
  // fFinalResult(current)/(1.0+0.0185*(fTP-25.0));
  float TempCoefficient = 1.0 + 0.0185 * (readTemp() - 25.0);
  float CoefficientVolatge = (float)averageVoltage / TempCoefficient;
  // 1ms/cm<EC<=3ms/cm
  if (CoefficientVolatge <= 448)
    ecCurrent = 6.84 * CoefficientVolatge - 64.32;
  // 3ms/cm<EC<=10ms/cm
  else if (CoefficientVolatge <= 1457)
    ecCurrent = 6.98 * CoefficientVolatge - 127;
  // 10ms/cm<EC<20ms/cm
  else
    ecCurrent = 5.3 * CoefficientVolatge + 2278;
  // convert us/cm to ms/cm
  ecCurrent /= 1000;
  DEBUG_VAR("EC value", ecCurrent);
  root["EC"] = ecCurrent;
}

void Irene3000::calibratepH7() {
  delay(1000);
  this->ads.setGain(GAIN_FOUR);
  this->params.pH7Cal = ads.readADC_SingleEnded(PH_CHANNEL);
  this->calcpHSlope();
}

void Irene3000::calibratepH4() {
  delay(1000);
  this->ads.setGain(GAIN_FOUR);
  this->params.pH4Cal = ads.readADC_SingleEnded(PH_CHANNEL);
  this->calcpHSlope();
}

void Irene3000::saveCalibrationDate() {
  this->params.calibrationDate = CoolTime::getInstance().getIso8601DateTime();
}

void Irene3000::readLastCalibrationDate(JsonObject &root) {
  String calibrationDate = params.calibrationDate;
  DEBUG_VAR("laste calibration date: ", calibrationDate);
  if (calibrationDate == "0000-00-00T00:00:00Z") {
    ERROR_LOG("PH calibration date error");
    root["calibrationDate"] = RawJson("null");
  } else {
    root["calibrationDate"] = calibrationDate;
  }
}

void Irene3000::calcpHSlope() {
  params.pHStep =
      ((((REF_VOLTAGE * (float)(params.pH7Cal - params.pH4Cal)) / 32767) *
        1000) /
       OPAMP_GAIN) /
      3;
}

void Irene3000::resetParams(void) {
  params.pH7Cal = 16384; // assume ideal probe and amp conditions 1/2 of 4096
  params.pH4Cal = 8192;  // using ideal probe slope we end up this many 12bit
                         // units away on the 4 scale
  params.pHStep = 59.16; // ideal probe slope
  params.calibrationDate = "0000-00-00T00:00:00Z";
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
}