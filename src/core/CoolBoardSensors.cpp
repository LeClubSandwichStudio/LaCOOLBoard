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

void CoolBoardSensors::read(PrintAdapter streamer) {
  delay(100);

  if (this->lightDataActive.visible) {
    CoolMessagePack::msgpckMap(streamer, 3, "SI114X");
    msgpck_write_string(&streamer, "visibleLight");
    if (this->lightSensor.ReadResponseReg() == CoolSI114X_VIS_OVERFLOW) {
      msgpck_write_nil(&streamer);
      // send NOOP command to SI1145 to clear overflow value
      this->lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      msgpck_write_integer(&streamer, this->lightSensor.ReadVisible());
    }
  }
  if (this->lightDataActive.ir) {
    msgpck_write_string(&streamer, "infrared");
    if (this->lightSensor.ReadResponseReg() == CoolSI114X_IR_OVERFLOW) {
      msgpck_write_nil(&streamer);
      // send NOOP command to SI1145 to clear overflow value
      this->lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      msgpck_write_integer(&streamer, this->lightSensor.ReadIR());
    }
  }
  if (this->lightDataActive.uv) {
    msgpck_write_string(&streamer, "ultraviolet");
    if (this->lightSensor.ReadResponseReg() == CoolSI114X_UV_OVERFLOW) {
      msgpck_write_nil(&streamer);
      // send NOOP command to SI1145 to clear overflow value
      this->lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      msgpck_write_float(&streamer, (float)this->lightSensor.ReadUV() / 100);
    }
  }
  if (this->airDataActive.temperature) {
    // wait for BME280 to finish data conversion (status reg bit3 == 0)
    while ((this->envSensor.readRegister(BME280_STAT_REG) & 0x10) != 0) {
      yield();
    }
    CoolMessagePack::msgpckMap(streamer, 3, "BME280");
    CoolMessagePack::msgpckFloat(streamer, this->envSensor.readTempC(), "temperature");
  }
  if (this->airDataActive.pressure) {
    // wait for BME280 to finish data conversion (status reg bit3 == 0)
    while ((this->envSensor.readRegister(BME280_STAT_REG) & 0x10) != 0) {
      yield();
    }
    CoolMessagePack::msgpckFloat(streamer, this->envSensor.readFloatPressure(), "pressure");
  }
  if (this->airDataActive.humidity) {
    // wait for BME280 to finish data conversion (status reg bit3 == 0)
    while ((this->envSensor.readRegister(BME280_STAT_REG) & 0x10) != 0) {
      yield();
    }
    CoolMessagePack::msgpckFloat(streamer, this->envSensor.readFloatHumidity(), "humidity");
  }
  if (this->vbatActive) {
    CoolMessagePack::msgpckMap(streamer, 1, "Vbat");
    CoolMessagePack::msgpckFloat(streamer, this->readVBat(), "batteryV");
  }
  if (this->soilMoistureActive) {
    CoolMessagePack::msgpckMap(streamer, 1, "soilMoisture");
    CoolMessagePack::msgpckFloat(streamer, this->readSoilMoisture(), "soilMoisture");
  }
  if (this->wallMoistureActive) {
    CoolMessagePack::msgpckMap(streamer, 1, "wallMoisture");
    CoolMessagePack::msgpckFloat(streamer, this->readWallMoisture(), "wallMoisture");
  }
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
    String data = kv["reference"];
    if (data == "BME280") {
      CoolConfig::set<bool>(kv["measures"], "temperature",
                            this->airDataActive.temperature);
      CoolConfig::set<bool>(kv["measures"], "humidity",
                            this->airDataActive.humidity);
      CoolConfig::set<bool>(kv["measures"], "pressure",
                            this->airDataActive.pressure);
    } else if (data == "SI114X") {
      CoolConfig::set<bool>(kv["measures"], "visibleLight",
                            this->lightDataActive.visible);
      CoolConfig::set<bool>(kv["measures"], "infrared",
                            this->lightDataActive.ir);
      CoolConfig::set<bool>(kv["measures"], "ultraviolet",
                            this->lightDataActive.uv);
    } else if (data == "vbat") {
      CoolConfig::set<bool>(kv["measures"], "batteryV", this->vbatActive);
    } else if (data == "soilMoisture") {
      CoolConfig::set<bool>(kv["measures"], "soilMoisture",
                            this->soilMoistureActive);
    } else if (data == "wallMoisture") {
      CoolConfig::set<bool>(kv["measures"], "wallMoisture",
                            this->wallMoistureActive);
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
