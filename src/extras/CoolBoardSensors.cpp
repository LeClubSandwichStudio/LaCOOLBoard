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
  DEBUG_LOG("Onboard sensors started");
}

void CoolBoardSensors::end() { this->lightSensor.DeInit(); }

void CoolBoardSensors::read(JsonObject &root) {
  delay(100);

  if (this->lightDataActive.visible) {
    if (this->lightSensor.ReadResponseReg() == CoolSI114X_VIS_OVERFLOW) {
      root["visibleLight"] = "overflow";
      // send NOOP command to SI1145 to clear overflow value
      this->lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      root["visibleLight"] = this->lightSensor.ReadVisible();
    }
  }
  if (this->lightDataActive.ir) {
    if (this->lightSensor.ReadResponseReg() == CoolSI114X_IR_OVERFLOW) {
      root["infraRed"] = "overflow";
      // send NOOP command to SI1145 to clear overflow value
      this->lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      root["infraRed"] = this->lightSensor.ReadIR();
    }
  }

  if (this->lightDataActive.uv) {
    if (this->lightSensor.ReadResponseReg() == CoolSI114X_UV_OVERFLOW) {
      root["ultraViolet"] = "overflow";
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
  DEBUG_JSON("Onboard sensors data:", root);
}

bool CoolBoardSensors::config() {
  File coolBoardSensorsConfig =
      SPIFFS.open("/coolBoardSensorsConfig.json", "r");

  if (!coolBoardSensorsConfig) {
    ERROR_LOG("Failed to read /coolBoardSensorsConfig.json");
    return (false);
  } else {
    String data = coolBoardSensorsConfig.readString();
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(data);
    if (!json.success()) {
      ERROR_LOG("Failed to parse onboard sensors configuration JSON");
      return (false);
    } else {
      DEBUG_JSON("Onboard sensors config JSON:", json);
      DEBUG_VAR("JSON buffer size:", jsonBuffer.size());
      if (json["BME280"]["temperature"].success()) {
        this->airDataActive.temperature = json["BME280"]["temperature"];
      }
      json["BME280"]["temperature"] = this->airDataActive.temperature;
      if (json["BME280"]["humidity"].success()) {
        this->airDataActive.humidity = json["BME280"]["humidity"];
      }
      json["BME280"]["humidity"] = this->airDataActive.humidity;
      if (json["BME280"]["pressure"].success()) {
        this->airDataActive.pressure = json["BME280"]["pressure"];
      }
      json["BME280"]["pressure"] = this->airDataActive.pressure;
      if (json["SI114X"]["visible"].success()) {
        this->lightDataActive.visible = json["SI114X"]["visible"];
      }
      json["SI114X"]["visible"] = this->lightDataActive.visible;
      if (json["SI114X"]["ir"].success()) {
        this->lightDataActive.ir = json["SI114X"]["ir"];
      }
      json["SI114X"]["ir"] = this->lightDataActive.ir;
      if (json["SI114X"]["uv"].success()) {
        this->lightDataActive.uv = json["SI114X"]["uv"];
      }
      json["SI114X"]["uv"] = this->lightDataActive.uv;
      if (json["vbat"].success()) {
        this->vbatActive = json["vbat"];
      }
      json["vbat"] = this->vbatActive;
      if (json["soilMoisture"].success()) {
        this->soilMoistureActive = json["soilMoisture"];
      }
      json["soilMoisture"] = this->soilMoistureActive;
      if (json["wallMoisture"].success()) {
        this->wallMoistureActive = json["wallMoisture"];
      }
      json["wallMoisture"] = this->wallMoistureActive;
      coolBoardSensorsConfig.close();
      coolBoardSensorsConfig = SPIFFS.open("/coolBoardSensorsConfig.json", "w");
      if (!coolBoardSensorsConfig) {
        ERROR_LOG("Failed to write to /coolBoardSensorsConfig.json");
        return (false);
      }
      json.printTo(coolBoardSensorsConfig);
      coolBoardSensorsConfig.close();
      DEBUG_LOG("Saved onboard sensors config to /coolBoardSensorsConfig.json");
      return (true);
    }
  }
}

void CoolBoardSensors::printConf() {
  INFO_LOG("Onboard sensors configuration:");
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
  // convert it to approx. correct tension in volts
  float val = 6.04 / 1024 * raw;

  DEBUG_VAR("Battery voltage:", val);
  return (val);
}

float CoolBoardSensors::readSoilMoisture() {
  digitalWrite(MOISTURE_SENSOR_PIN, LOW);
  digitalWrite(ANALOG_MULTIPLEXER_PIN, HIGH);
  delay(2000);

  // read moisture sensor value
  int val = analogRead(A0);

  if (val >= 891) {
    val = 890;
  }

  float result = (float)map(val, 0, 890, 0, 100);
  // disable moisture sensor for minimum wear
  digitalWrite(MOISTURE_SENSOR_PIN, HIGH);
  DEBUG_VAR("Raw soil moisture sensor value:", val);
  DEBUG_VAR("Computed soil moisture:", result);
  return (result);
}

float CoolBoardSensors::readWallMoisture() {
  float val = 0;
  digitalWrite(ANALOG_MULTIPLEXER_PIN, HIGH);
  delay(200);
  // oversample value 64 times for stable readings
  for (int i = 1; i <= 64; i++) {
    digitalWrite(MOISTURE_SENSOR_PIN, LOW);
    delay(2);
    // read moisture sensor value
    val = val + analogRead(A0);
    delay(2);
    // disable moisture sensor for minimum wear
    digitalWrite(MOISTURE_SENSOR_PIN, HIGH);
  }
  // divide by 64 to get the average
  val = val / 64;
  DEBUG_VAR("Raw wall moisture sensor value:", val);
  return (float(val));
}