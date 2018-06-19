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
#include <Wire.h>
#include <stdint.h>

#include "CoolBoardSensors.h"
#include "CoolConfig.h"
#include "CoolLog.h"

CoolBoardSensors::CoolBoardSensors() {
  pinMode(ANALOG_MULTIPLEXER_PIN, OUTPUT);
  pinMode(MOISTURE_SENSOR_PIN, OUTPUT);
  // prevents wear on the soil moisture fork
  digitalWrite(MOISTURE_SENSOR_PIN, HIGH);
}

void CoolBoardSensors::allActive() {
  this->lightDataActive.visible = 1;
  this->lightDataActive.ir = 1;
  this->lightDataActive.uv = 1;

  this->airDataActive.temperature = 1;
  this->airDataActive.humidity = 1;
  this->airDataActive.pressure = 1;

  this->vbatActive = 1;
  this->soilMoistureActive = 1;
}

void CoolBoardSensors::begin() {
  Wire.begin(2, 14);
  while (!this->lightSensor.Begin()) {
    DEBUG_LOG("SI1145 light sensor is not ready, waiting for 1 second...");
    delay(1000);
  }
  this->setEnvSensorSettings();
  delay(100);
  uint8_t envSensorStatus = this->envSensor.begin();
  // Make sure sensors had enough time to turn on.
  // BME280 requires 2ms to start up.
  delay(1000);
  DEBUG_VAR("BME280 status after begin is:", envSensorStatus);
  DEBUG_LOG("Builtin sensors started");
}

void CoolBoardSensors::end() { this->lightSensor.DeInit(); }

void CoolBoardSensors::read(JsonObject &root) {
  delay(100);

  if (this->lightDataActive.visible) {
    if (this->lightSensor.ReadResponseReg() == CoolSI114X_VIS_OVERFLOW) {
      root["visibleLight"] = RawJson("null");
      // send NOOP command to SI1145 to clear overflow value
      this->lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      root["visibleLight"] = this->lightSensor.ReadVisible();
    }
  }
  if (this->lightDataActive.ir) {
    if (this->lightSensor.ReadResponseReg() == CoolSI114X_IR_OVERFLOW) {
      root["infraRed"] = RawJson("null");
      // send NOOP command to SI1145 to clear overflow value
      this->lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      root["infraRed"] = this->lightSensor.ReadIR();
    }
  }

  if (this->lightDataActive.uv) {
    if (this->lightSensor.ReadResponseReg() == CoolSI114X_UV_OVERFLOW) {
      root["ultraViolet"] = RawJson("null");
      // send NOOP command to SI1145 to clear overflow value
      this->lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      root["ultraViolet"] = (float)this->lightSensor.ReadUV() / 100;
    }
  }

  if (this->airDataActive.temperature) {
    // wait for BME280 to finish data conversion (status reg bit3 == 0)
    while ((this->envSensor.readRegister(BME280_STAT_REG) & 0x10) != 0) {
      yield();
    }
    root["Temperature"] = this->envSensor.readTempC();
  }

  if (this->airDataActive.pressure) {
    // wait for BME280 to finish data conversion (status reg bit3 == 0)
    while ((this->envSensor.readRegister(BME280_STAT_REG) & 0x10) != 0) {
      yield();
    }
    root["Pressure"] = this->envSensor.readFloatPressure();
  }

  if (this->airDataActive.humidity) {
    // wait for BME280 to finish data conversion (status reg bit3 == 0)
    while ((this->envSensor.readRegister(BME280_STAT_REG) & 0x10) != 0) {
      yield();
    }
    root["Humidity"] = this->envSensor.readFloatHumidity();
  }
  if (this->vbatActive) {
    root["Vbat"] = this->readVBat();
  }
  if (this->soilMoistureActive) {
    root["soilMoisture"] = this->readSoilMoisture();
  }
  if (this->wallMoistureActive) {
    root["wallMoisture"] = this->readWallMoisture();
  }
  DEBUG_JSON("Builtin sensors data:", root);
}

bool CoolBoardSensors::config() {
  CoolConfig config("/coolBoardSensorsConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to builtin sensors configuration");
    return (false);
  }
  JsonObject &json = config.get();
  config.set<bool>(json["BME280"], "temperature",
                   this->airDataActive.temperature);
  config.set<bool>(json["BME280"], "humidity", this->airDataActive.humidity);
  config.set<bool>(json["BME280"], "pressure", this->airDataActive.pressure);
  config.set<bool>(json["SI114X"], "visible", this->lightDataActive.visible);
  config.set<bool>(json["SI114X"], "ir", this->lightDataActive.ir);
  config.set<bool>(json["SI114X"], "uv", this->lightDataActive.uv);
  config.set<bool>(json, "vbat", this->vbatActive);
  config.set<bool>(json, "soilMoisture", this->soilMoistureActive);
  config.set<bool>(json, "wallMoisture", this->wallMoistureActive);
  DEBUG_LOG("Builtin sensors configuration loaded");
  return (true);
}

void CoolBoardSensors::printConf() {
  INFO_LOG("Builtin sensors configuration");
  INFO_VAR("  Air humidity         =", airDataActive.humidity);
  INFO_VAR("  Atmospheric pressure =", airDataActive.pressure);
  INFO_VAR("  Light visible        =", lightDataActive.visible);
  INFO_VAR("  Light IR             =", lightDataActive.ir);
  INFO_VAR("  Light UV             =", lightDataActive.uv);
  INFO_VAR("  Battery voltage      =", vbatActive);
  INFO_VAR("  Soil moisture        =", soilMoistureActive);
  INFO_VAR("  Wall moisture        =", wallMoistureActive);
}

void CoolBoardSensors::setEnvSensorSettings(uint8_t commInterface,
                                            uint8_t I2CAddress, uint8_t runMode,
                                            uint8_t tStandby, uint8_t filter,
                                            uint8_t tempOverSample,
                                            uint8_t pressOverSample,
                                            uint8_t humidOverSample) {
  this->envSensor.settings.commInterface = commInterface;
  this->envSensor.settings.I2CAddress = I2CAddress;
  this->envSensor.settings.runMode = runMode;
  this->envSensor.settings.tStandby = tStandby;
  this->envSensor.settings.filter = filter;
  this->envSensor.settings.tempOverSample = tempOverSample;
  this->envSensor.settings.pressOverSample = pressOverSample;
  this->envSensor.settings.humidOverSample = humidOverSample;
}

float CoolBoardSensors::readVBat() {
  digitalWrite(ANALOG_MULTIPLEXER_PIN, LOW);
  delay(200);

  // read battery voltage
  int raw = analogRead(A0);
  float voltage = (raw * MAX_BATTERY_VOLTAGE) / ADC_MAX_VAL;
  DEBUG_VAR("Raw value:", raw);
  DEBUG_VAR("Battery voltage:", voltage);
  return (voltage);
}
float CoolBoardSensors::soilMoistureLinearisation(float rawMoistureValue) {
  float moistureValue =
      LINEARISATION_MOISTURE_A * rawMoistureValue * rawMoistureValue +
      LINEARISATION_MOISTURE_B * rawMoistureValue + LINEARISATION_MOISTURE_C;
  return moistureValue;
}

float CoolBoardSensors::readSoilMoisture() {
  digitalWrite(MOISTURE_SENSOR_PIN, LOW);
  digitalWrite(ANALOG_MULTIPLEXER_PIN, HIGH);
  delay(2000);
  int rawVal = 0;
  for (int i = 0; i < MOISTURE_SAMPLES; i++) {
    delay(10);
    rawVal = rawVal + analogRead(A0);
    delay(10);
  }
  float result = (float)rawVal / MOISTURE_SAMPLES;
  result = soilMoistureLinearisation(result);
  if ((int)result >= 100)
    result = 100.;
  if ((int)result < 0)
    result = 0.;
  // disable moisture sensor for minimum wear
  digitalWrite(MOISTURE_SENSOR_PIN, HIGH);
  DEBUG_VAR("Raw soil moisture sensor value:", rawVal / MOISTURE_SAMPLES);
  DEBUG_VAR("Computed soil moisture:", result);
  return (result);
}

float CoolBoardSensors::readWallMoisture() {
  float val = 0;
  digitalWrite(ANALOG_MULTIPLEXER_PIN, HIGH);
  delay(200);
  for (int i = 1; i <= MOISTURE_SAMPLES; i++) {
    digitalWrite(MOISTURE_SENSOR_PIN, LOW);
    delay(2);
    val = val + analogRead(A0);
    delay(2);
    digitalWrite(MOISTURE_SENSOR_PIN, HIGH);
  }
  val = val / MOISTURE_SAMPLES;
  DEBUG_VAR("Raw wall moisture sensor value:", val);
  return (float(val));
}