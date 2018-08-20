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

#ifndef COOLBOARDSENSORS_H
#define COOLBOARDSENSORS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SparkFunBME280.h>

#include "CoolSI114X.h"

#define MOISTURE_SENSOR_PIN 13
#define ANALOG_MULTIPLEXER_PIN 12
#define ADC_MAX_VAL 1023.
#define MAX_BATTERY_VOLTAGE 5.6
#define LINEARISATION_MOISTURE_A 0.00027
#define LINEARISATION_MOISTURE_B -0.13
#define LINEARISATION_MOISTURE_C -4.0
#define MOISTURE_SAMPLES 64

class CoolBoardSensors {

public:
  CoolBoardSensors();
  void begin();
  void read(JsonObject &root);
  void allActive();
  void end();
  bool config();
  void printConf();
  void setEnvSensorSettings(uint8_t commInterface = I2C_MODE,
                            uint8_t I2CAddress = 0x76, uint8_t runMode = 3,
                            uint8_t tStandby = 0, uint8_t filter = 0,
                            uint8_t tempOverSample = 1,
                            uint8_t pressOverSample = 1,
                            uint8_t humidOverSample = 1);
  float readVBat();
  float soilMoistureLinearisation(float rawMoistureValue);
  float readSoilMoisture();
  float readWallMoisture();
  CoolSI114X lightSensor;
  BME280 envSensor;

private:
  struct {
    bool visible = true;
    bool ir = true;
    bool uv = true;
  } lightDataActive;

  struct {
    bool temperature = true;
    bool humidity = true;
    bool pressure = true;
  } airDataActive;

  bool vbatActive = true;
  bool soilMoistureActive = true;
  bool wallMoistureActive = false;
};

#endif
