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

#ifndef CoolBoardSensors_H
#define CoolBoardSensors_H

#include "Arduino.h"
#include "internals/CoolSI114X.h" // light sensor
#include "SparkFunBME280.h"       // atmosperic sensor

/**
 *
 *  \class CoolBoardSensors
 *  \brief This class handles the on-board sensors.
 *
 */
class CoolBoardSensors {

public:
  CoolBoardSensors();

  void begin();

  // data is in JSON
  String read();

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

  // battery level
  float readVBat();

  // moisture
  float readMoisture();

  /**
   *  SI114X light sensor instance
   */
  CoolSI114X lightSensor;

  /**
   *  BME280 environment sensor instance
   */
  BME280 envSensor;

private:
  /**
   *  lightActive structure
   *
   *  set visible to 1 to have visibleLight readings
   *  set ir to 1 to have infraRed readings
   *  set uv to 1 to have ultraViolet readings
   */
  struct lightActive {
    bool visible = 1;
    bool ir = 1;
    bool uv = 1;
  } lightDataActive;

  /**
   *  airActive structure
   *
   *  set temperature to 1 to have temperature readings
   *  set humidity to 1 to have humidity readings
   *  set pressure to 1 to have pressure readings
   */
  struct airActive {
    bool temperature = 1;
    bool humidity = 1;
    bool pressure = 1;
  } airDataActive;

  /**
   *   Moisture enable pin
   */
  const int EnMoisture = 13;

  /**
   *  Analog multiplexer: LOW=Vbat, HIGH=Moisture
   */
  const int AnMplex = 12;

  /**
   *  set vbatActive to 1 to have battery voltage readings
   */
  bool vbatActive = 1;

  /**
   *  set soilMoistureActive to 1 to have soil Moisture readings
   */
  bool soilMoistureActive = 1;
};

#endif
