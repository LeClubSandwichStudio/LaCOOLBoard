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
  // Make sure sensors had enough time to turn on.
  // BME280 requires 2ms to start up.
  if (this->envSensor.begin()) {
    DEBUG_LOG("Builtin sensors started");
  } else {
    DEBUG_LOG("BME280 fail to begin");
  }
}

void CoolBoardSensors::end() { this->lightSensor.DeInit(); }

void CoolBoardSensors::read(JsonObject &root) {
  delay(100);

  if (this->lightDataActive.visible) {
    root.createNestedObject("SI114X_1");
    if (this->lightSensor.ReadResponseReg() == CoolSI114X_VIS_OVERFLOW) {
      root["SI114X_1"]["visibleLight"] = RawJson("null");
      // send NOOP command to SI1145 to clear overflow value
      this->lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      root["SI114X_1"]["visibleLight"] = this->lightSensor.ReadVisible();
    }
  }
  if (this->lightDataActive.ir) {
    if (!root["SI114X_1"].success()) {
      root.createNestedObject("SI114X_1");
    }
    if (this->lightSensor.ReadResponseReg() == CoolSI114X_IR_OVERFLOW) {
      root["SI114X_1"]["infrared"] = RawJson("null");
      // send NOOP command to SI1145 to clear overflow value
      this->lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      root["SI114X_1"]["infrared"] = this->lightSensor.ReadIR();
    }
  }

  if (this->lightDataActive.uv) {
    if (!root["SI114X_1"].success()) {
      root.createNestedObject("SI114X_1");
    }
    if (this->lightSensor.ReadResponseReg() == CoolSI114X_UV_OVERFLOW) {
      root["SI114X_1"]["ultraviolet"] = RawJson("null");
      // send NOOP command to SI1145 to clear overflow value
      this->lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      root["SI114X_1"]["ultraviolet"] = (float)this->lightSensor.ReadUV() / 100;
    }
  }

  if (this->airDataActive.temperature) {
    // wait for BME280 to finish data conversion (status reg bit3 == 0)
    while ((this->envSensor.readRegister(BME280_STAT_REG) & 0x10) != 0) {
      yield();
    }
    root.createNestedObject("BME280_1");
    root["BME280_1"]["temperature"] = this->envSensor.readTempC();
  }

  if (this->airDataActive.pressure) {
    // wait for BME280 to finish data conversion (status reg bit3 == 0)
    while ((this->envSensor.readRegister(BME280_STAT_REG) & 0x10) != 0) {
      yield();
    }
    if (!root["BME280_1"].success()) {
      root.createNestedObject("BME280_1");
    }
    root["BME280_1"]["pressure"] = this->envSensor.readFloatPressure();
  }

  if (this->airDataActive.humidity) {
    // wait for BME280 to finish data conversion (status reg bit3 == 0)
    while ((this->envSensor.readRegister(BME280_STAT_REG) & 0x10) != 0) {
      yield();
    }
    if (!root["BME280_1"].success()) {
      root.createNestedObject("BME280_1");
    }
    root["BME280_1"]["humidity"] = this->envSensor.readFloatHumidity();
  }
  if (this->soilMoistureActive) {
    root.createNestedObject("soilMoisture_1");
    root["soilMoisture_1"]["soilMoisture"] = this->readSoilMoisture();
  }
  if (this->wallMoistureActive) {
    root.createNestedObject("wallMoisture_1");
    root["wallMoisture_1"]["wallMoisture"] = this->readWallMoisture();
  }
  root.createNestedObject("battery");
  root["battery"]["voltage"] = this->readVBat();
  DEBUG_JSON("Builtin sensors data:", root);
}

bool CoolBoardSensors::config() {
  CoolConfig config("/sensors.json");
  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read /sensors.json");
    return (false);
  }
  JsonObject &sensors = config.get();
  JsonArray &root = sensors["sensors"];
  for (auto kv : root) {
    JsonArray &measures = kv["measures"];
    if (kv["reference"] == "BME280") {
      for (auto measure : measures) {
        if (measure == "temperature") {
          this->airDataActive.temperature = 1;
        }
        if (measure == "humidity") {
          this->airDataActive.humidity = 1;
        }
        if (measure == "pressure") {
          this->airDataActive.pressure = 1;
        }
      }
    } else if (kv["reference"] == "SI114X") {
      for (auto measure : measures) {
        if (measure == "visibleLight") {
          this->lightDataActive.visible = 1;
        }
        if (measure == "infrared") {
          this->lightDataActive.ir = 1;
        }
        if (measure == "ultraviolet") {
          this->lightDataActive.uv = 1;
        }
      }
    } else if (kv["reference"] == "soilMoisture") {
      for (auto measure : measures) {
        if (measure == "soilMoisture") {
          this->soilMoistureActive = 1;
        }
      }
    } else if (kv["reference"] == "wallMoisture") {
      for (auto measure : measures) {
        if (measure == "wallMoisture") {
          this->wallMoistureActive = 1;
        }
      }
    }
  }
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
  float moistureValue = rawMoistureValue / LINEARISATION_MOISTURE_100;
  moistureValue =
      moistureValue * moistureValue * moistureValue * LINEARISATION_MOISTURE_A;
  if ((int16_t)moistureValue >= 100)
    return 100.;
  if ((int)moistureValue < 0)
    return 0.;
  return moistureValue;
}

float CoolBoardSensors::readSoilMoisture() {
  digitalWrite(ANALOG_MULTIPLEXER_PIN, HIGH);
  delay(200);
  int rawVal = 0;
  for (int i = 0; i < SOIL_MOISTURE_SAMPLES; i++) {
    digitalWrite(MOISTURE_SENSOR_PIN, LOW);
    delay(2);
    rawVal = rawVal + analogRead(A0);
    delay(200);
    digitalWrite(MOISTURE_SENSOR_PIN,
                 HIGH); // disable moisture sensor for minimum wear
  }
  float result = (float)rawVal / SOIL_MOISTURE_SAMPLES;
  result = soilMoistureLinearisation(result);
  digitalWrite(MOISTURE_SENSOR_PIN, HIGH);
  DEBUG_VAR("Raw soil moisture sensor value:", rawVal / SOIL_MOISTURE_SAMPLES);
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
