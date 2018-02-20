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
#include <stdint.h>

#include <ArduinoJson.h>

#include "CoolBoardSensors.h"
#include "CoolLog.h"

/**
 *  CoolBoardSensors::CoolBoardSensors():
 *  This Constructor is provided to
 *  init the different used pins
 */
CoolBoardSensors::CoolBoardSensors() {
  pinMode(AnMplex, OUTPUT);       // Declare Analog Multiplexer OUTPUT
  pinMode(EnMoisture, OUTPUT);    // Declare Moisture enable Pin
  digitalWrite(EnMoisture, HIGH); // Prevent Wearing on the soil moisture fork
}

/**
 *  CoolBoardSensors::allActive():
 *  This method is provided to allow
 *  activation of all the sensor board sensors
 *  without passing by the configuration file/method
 */
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

/**
 *  CoolBoardSensors::begin():
 *  This method is provided to start the
 *  sensors that are on the sensor board
 */
void CoolBoardSensors::begin() {
  while (!lightSensor.Begin()) {
    DEBUG_LOG("SI1145 light sensor is not ready, waiting for 1 second...");
    delay(1000);
  }
  this->setEnvSensorSettings();
  delay(100); // Make sure sensor had enough time to turn on. BME280 requires
              // 2ms to start up.
  uint8_t envSensorStatus = this->envSensor.begin();
  delay(1000); // Make sure sensor had enough time to turn on. BME280 requires
               // 2ms to start up.
  DEBUG_VAR("BME280 status after begin is:", envSensorStatus);
  DEBUG_LOG("Onboard sensors started");
}

/**
 *  CoolBoardSensors::end():
 *  This method is provided to end
 *  the sensors on the sensor board
 */
void CoolBoardSensors::end() {
  lightSensor.DeInit();
}

/**
 *  CoolBoardSensors::read():
 *  This method is provided to return the
 *  data read by the sensor board
 *
 *  \return a json string containing the
 *  sensors data
 **/
String CoolBoardSensors::read() {
  String data;
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  delay(100);

  if (lightDataActive.visible) {
    // check for overflow in SI1145 response register
    if (lightSensor.ReadResponseReg() == CoolSI114X_VIS_OVERFLOW) {
      root["visibleLight"] = "overflow";
      // send noop command to SI1145 to clear overflow value
      lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      root["visibleLight"] = lightSensor.ReadVisible();
    }
  }
  if (lightDataActive.ir) {
    // check for overflow in SI1145 response register
    if (lightSensor.ReadResponseReg() == CoolSI114X_IR_OVERFLOW) {
      root["infraRed"] = "overflow";
      // send noop command to SI1145 to clear overflow value
      lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      root["infraRed"] = lightSensor.ReadIR();
    }
  }

  if (lightDataActive.uv) {
    // check for overflow in SI1145 response register
    if (lightSensor.ReadResponseReg() == CoolSI114X_UV_OVERFLOW) {
      root["ultraViolet"] = "overflow";
      // send noop command to SI1145 to clear overflow value
      lightSensor.WriteParamData(CoolSI114X_COMMAND, CoolSI114X_NOP);
    } else {
      float tempUV = (float)lightSensor.ReadUV() / 100;
      root["ultraViolet"] = tempUV;
    }
  }

  if (airDataActive.temperature) {
    // wait for BME280 to finish data conversion (status reg bit3 == 0)
    while ((envSensor.readRegister(BME280_STAT_REG) & 0x10) != 0) {
      yield();
    }
    root["Temperature"] = envSensor.readTempC();
  }

  if (airDataActive.pressure) {
    // wait for BME280 to finish data conversion (status reg bit3 == 0)
    while ((envSensor.readRegister(BME280_STAT_REG) & 0x10) != 0) {
      yield();
    }
    root["Pressure"] = envSensor.readFloatPressure();
  }

  if (airDataActive.humidity) {
    // wait for BME280 to finish data conversion (status reg bit3 == 0)
    while ((envSensor.readRegister(BME280_STAT_REG) & 0x10) != 0) {
      yield();
    }
    root["Humidity"] = envSensor.readFloatHumidity();
  }
  if (vbatActive) {
    root["Vbat"] = this->readVBat();
  }
  if (soilMoistureActive) {
    root["soilMoisture"] = this->readMoisture();
  }
  root.printTo(data);
  DEBUG_JSON("Onboard sensors values JSON:", root);
  return (data);
}

/**
 *  CoolBoardSensors::config():
 *  This method is provided to configure the
 *  sensor board :  -activate   1
 *      -deactivate 0
 *
 *  \return true if configuration is successful,
 *  false otherwise
 */
bool CoolBoardSensors::config() {
  File coolBoardSensorsConfig =
      SPIFFS.open("/coolBoardSensorsConfig.json", "r");

  if (!coolBoardSensorsConfig) {
    ERROR_LOG("Failed to read /coolBoardSensorsConfig.json");
    return (false);
  } else {
    size_t size = coolBoardSensorsConfig.size();
    std::unique_ptr<char[]> buf(new char[size]);

    coolBoardSensorsConfig.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
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

/**
 *  CoolBoardSensors::printConf():
 *  This method is provided to print the
 *  configuration to the Serial Monitor
 */
void CoolBoardSensors::printConf() {
  INFO_LOG("Onboard sensors configuration:");
  INFO_VAR("  Air humidity         =", airDataActive.humidity);
  INFO_VAR("  Atmospheric pressure =", airDataActive.pressure);
  INFO_VAR("  Light visible        =", lightDataActive.visible);
  INFO_VAR("  Light IR             =", lightDataActive.ir);
  INFO_VAR("  Light UV             =", lightDataActive.uv);
  INFO_VAR("  Battery voltage      =", vbatActive);
  INFO_VAR("  Soil moisture        =", soilMoistureActive);
}

/**
 *  CoolBoardSensors::setEnvSensorSetting():
 *  This method is provided to set the enviornment
 *  sensor settings , if argument is ommitted , default value will be
 *assigned
 *
 */
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

/**
 *  CoolBoardSensors::readVBat():
 *  This method is provided to read the
 *  Battery Voltage.
 *
 *  \return a float representing the battery
 *  voltage
 */
float CoolBoardSensors::readVBat() {
  // Enable analog switch to get the batterie voltage
  digitalWrite(this->AnMplex, LOW);
  delay(200);

  int raw = analogRead(A0); // read battery voltage
  // convert it to approx. correct tension in volts
  float val = 6.04 / 1024 * raw;

  DEBUG_VAR("Battery voltage:", val);
  return (val);
}

/**
 *  CoolBoardSensors::readMoisture():
 *  This method is provided to red the
 *  Soil Moisture
 *
 *  \return a float represnting the
 *  soil moisture
 */
float CoolBoardSensors::readMoisture() {
  digitalWrite(EnMoisture, LOW); // enable moisture sensor and wait a bit
  digitalWrite(AnMplex, HIGH);   // enable analog switch to read moisture value
  delay(2000);

  int val = analogRead(A0); // read moisture sensor value

  if (val >= 891) {
    val = 890;
  }

  float result = (float)map(val, 0, 890, 0, 100);

  digitalWrite(EnMoisture, HIGH); // disable moisture sensor for minimum wear
  DEBUG_VAR("Raw moisture sensor value:", val);
  DEBUG_VAR("Computed soil moisture:", result);
  return (result);
}
