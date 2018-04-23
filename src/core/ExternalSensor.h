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

#ifndef BaseExternalSensor_H
#define BaseExternalSensor_H

#include <Arduino.h>
#include <DallasTemperature.h>

#include "CoolAdafruit_ADS1015.h"
#include "CoolAdafruit_CCS811.h"
#include "CoolAdafruit_TCS34725.h"
#include "CoolGauge.h"
#include "CoolNDIR_I2C.h"

#include "CoolLog.h"

/**
 *  \class BaseExternalSensor:
 *  \brief This class is a generic external Sensor
 *  it is a way to access real external sensor
 *  methods through run Time polymorphism
 */
class BaseExternalSensor {

public:
  /**
   *  BaseExternalSensor():
   *  Base class generic Constructor
   */
  BaseExternalSensor() {}

  /**
   *  begin():
   *  Base class virtual
   *  generic begin method
   *
   *  \return generic value as it's not supposed
   *  to be used
   */
  virtual uint8_t begin() { return (-2); }

  /**
   *  read():
   *  Base class virtual
   *  generic read method
   *
   *  \return generic value
   *  as it is not supposed
   *  to be used
   */
  virtual float read() { return (-2); }

  virtual float read(int16_t *a) { return (-42, 42); }

  virtual float read(int16_t *a, int16_t *b, float *c) { return (-42.42); }

  virtual float read(uint32_t *a, uint32_t *b, uint32_t *c) { return (-42.42); }

  virtual float read(int16_t *a, int16_t *b, int16_t *c, int16_t *d) {
    return (-42.42);
  }

  virtual float read(int16_t *a, int16_t *b, int16_t *c, int16_t *d, int16_t *e,
                     int16_t *f) {
    return (-42.42);
  }

  virtual float read(int16_t *a, int16_t *b, int16_t *c, int16_t *d, int16_t *e,
                     int16_t *f, int16_t *g, int16_t *h) {
    return (-42.42);
  }
};

/**
 *  \class ExternalSensor
 *  \brief template<class SensorClass> class External Sensor:
 *  Derived class from BaseExternalSensor.
 *
 *  This is the generic Template for an external sensor
 *  This class works automatically with sensors that
 *  provide the following methods:
 *    - constructor(void);
 *    - uint8_t/bool begin(void);
 *    - float read(void);
 *
 *  If your sensor doesn't provide these methods
 *  or is not present in the specialized templates
 *  feel free to implement your own specialization,
 *  following the provided generic template,
 *  or contact us and we will be glad to expand our
 *  list of supported external sensors
 */
template <class T> class ExternalSensor : public BaseExternalSensor {
public:
  /**
   *  Generic Constructor
   */
  ExternalSensor() { sensor(); }

  /**
   *  Generic begin method
   */
  virtual uint8_t begin() { return (sensor.begin()); }

  /**
   *  Generic read method
   */
  virtual float read() { return (sensor.read()); }

private:
  T sensor; // the sensor itself
};

/**
 *  \class ExternalSensor<NDIR_I2C>
 *  \brief NDIR_I2C Specialization Class
 *  This is the template specialization
 *  for the NDIR_I2C CO2 sensor
 */
template <> class ExternalSensor<NDIR_I2C> : public BaseExternalSensor {
public:
  /**
   *  ExternalSensor(I2C address):
   *  NDIR_I2C specific constructor
   */
  ExternalSensor(uint8_t i2c_addr) { sensor = NDIR_I2C(i2c_addr); }

  /**
   *  begin():
   *  NDIR_I2C specific begin method
   *
   *  \return true if successful,
   *  false otherwise
   */
  virtual uint8_t begin() {
    if (sensor.begin()) {
      // INFO_LOG("NDIR_I2C init, wating 10 seconds");
      delay(10000);
      return (true);
    } else {
      // ERROR_LOG("Failed to init NDIR_I2C");
      return (false);
    }
  }

  /**
   *  read():
   *  NDIR_I2C specific read method
   *
   *  \return the ppm value if successful,
   *  else return -42
   */
  virtual float read() {
    if (sensor.measure()) {
      // DEBUG_VAR("Reading NDIR_I2C ppm:", (float)sensor.ppm);
      return ((float)sensor.ppm);
    } else {
      // ERROR_LOG("Failed to read from NDIR_I2C");
      return (-42);
    }
  }

private:
  NDIR_I2C sensor = NULL;
};

/**
 *  \class ExternalSensor<DallasTemperature>
 *  \brief DallasTemperature Specialization Class
 *  This is the template specialization
 *  for the Dallas Temperature sensor
 */
template <>
class ExternalSensor<DallasTemperature> : public BaseExternalSensor {
public:
  /**
   *  ExternalSensor():
   *  DallasTemperature specific constructor
   */
  ExternalSensor(OneWire *oneWire) {
    sensor = DallasTemperature(oneWire);
  }

  /**
   *  begin():
   *  DallasTemperature specific begin method
   *
   *  \return true if successful
   */
  virtual uint8_t begin() {
    sensor.begin();
    delay(5);
    sensor.getAddress(this->dallasAddress, 0);
    return (true);
  }
  /**
   *  read():
   *  DallasTemperature specific read method
   *
   *  \return the temperature in °C
   */
  virtual float read() {

    sensor.requestTemperatures(); // Send the command to get temperatures
    float result = (float)sensor.getTempCByIndex(0);
    // DEBUG_VAR("Dallas temperature sensor value:", result);
    return (result);
  }

private:
  DallasTemperature sensor;
  DeviceAddress dallasAddress;
};

/**
 *  \class ExternalSensor<Adafruit_TCS34725>
 *  \brief Adafruit_TCS34725 Specialization Class
 *  This is the template specialization
 *  for the Adafruit RGB Sensor
 */

template <>
class ExternalSensor<Adafruit_TCS34725> : public BaseExternalSensor {
public:
  /**
   *  ExternalSensor():
   *  Adafruit_TCS34725 specific constructor
   */
  ExternalSensor() {
    /* Initialise with default values (int time = 2.4ms, gain = 1x) */
    sensor =
        Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);
  }

  /**
   *  begin():
   *  Adafruit_TCS34725 specific begin method
   *
   *  \return true if successful
   */
  virtual uint8_t begin() {
    if (sensor.begin()) {
      // DEBUG_LOG("Found TCS34725 RGB sensor");
      return (true);
    } else {
      // ERROR_LOG("Failed to find TCS3472 RGB sensor, check connection");
      return (false);
    }
  }

  /**
   *  read(int16_t *a,int16_t *b,int16_t *c,int16_t *d,int16_t *e,int16_t *f):
   *  Adafruit_TCS34725 specific read method
   *
   *  modifies the input to R,G,B,C,lux,ColorTemp values
   */
  virtual float read(int16_t *a, int16_t *b, int16_t *c, int16_t *d, int16_t *e,
                     int16_t *f) {
    uint16_t internR, internG, internB, internC, internColorTemp, internLux;

    sensor.getRawData(&internR, &internG, &internB, &internC);

    internColorTemp =
        sensor.calculateColorTemperature(internR, internG, internB);

    internLux = sensor.calculateLux(internR, internG, internB);

    // DEBUG_LOG("TCS34725 RGB sensor value:");
    // DEBUG_VAR("Color temperature in Kelvin:", internColorTemp);
    // DEBUG_VAR("Illuminance in lux:", internLux);
    // DEBUG_VAR("Red value:", internR);
    // DEBUG_VAR("Green value:", internG);
    // DEBUG_VAR("Blue value:", internB);
    // DEBUG_VAR("C value:", internC);

    *a = (int16_t)internR;
    *b = (int16_t)internG;
    *c = (int16_t)internB;
    *d = (int16_t)internC;
    *e = (int16_t)internColorTemp;
    *f = (int16_t)internLux;

    return (0.0);
  }

private:
  Adafruit_TCS34725 sensor;
};

/**
 *  \class ExternalSensor<Adafruit_CCS881>
 *  \brief Adafruit_CCS881 Specialization Class
 *  This is the template specialization
 *  for the Adafruit VOC/eCO2 Sensor
 */

template <> class ExternalSensor<Adafruit_CCS811> : public BaseExternalSensor {
public:
  /**
   *  ExternalSensor():
   *  Adafruit_CCS881 specific constructor
   */
  ExternalSensor(uint8_t i2c_addr) {
    sensor = Adafruit_CCS811();

    if (!sensor.begin(i2c_addr)) {
      // ERROR_LOG("Failed to start CCS881 VOC/eCO2 sensor, check connection");
    }
  }

  virtual uint8_t begin() {
    while (!sensor.available())
      ;
    float T = sensor.calculateTemperature();
    sensor.setTempOffset(T - 25.0);
  }

  virtual float read(int16_t *a, int16_t *b, float *c) {
    uint16_t internC, internV;
    float internT;

    if (sensor.available()) {
      internT = sensor.calculateTemperature();
      if (!sensor.readData()) {
        internC = sensor.geteCO2();
        internV = sensor.getTVOC();
      }
    }
    // DEBUG_VAR("TCS3472 CO2 value in ppm:", internC);
    // DEBUG_VAR("TCS3472 VOC in ppb:", internV);
    // DEBUG_VAR("TCS3472 Temperature in °CL", internT);

    *a = (int16_t)internC;
    *b = (int16_t)internV;
    *c = internT;
    return (0.0);
  }

private:
  Adafruit_CCS811 sensor;
};

/**
 *  \class ExternalSensor<Adafruit_ADS1015>
 *  \brief Adafruit_ADS1015 Specialization Class
 *  This is the template specialization
 *  for the Adafruit Analog I2C Interface
 */

template <> class ExternalSensor<Adafruit_ADS1015> : public BaseExternalSensor {
public:
  /**
   *  ExternalSensor():
   *  Adafruit_ADS1015 specific constructor
   */
  ExternalSensor(uint8_t i2c_addr) {
    sensor = Adafruit_ADS1015(i2c_addr);
  }

  /**
   *  begin():
   *  Adafruit_ADS1015 specific begin method
   *
   *  \return true if successful
   */
  virtual uint8_t begin() {
    sensor.begin();
    return (true);
  }

  /**
   *  read(uint16_t *a,uint16_t *b,uint16_t *c,uint16_t *d,uint16_t
   **e,uint16_t *f): Adafruit_ADS1015 specific read method
   *
   *  modifies the input variables to channel0..3 and differential01 ,23
   *values
   */
  virtual float read(int16_t *a, int16_t *b, int16_t *c, int16_t *d, int16_t *e,
                     int16_t *f, int16_t *g, int16_t *h) {
    uint16_t channel0, channel1, channel2, channel3;
    uint16_t gain0, gain1, gain2, gain3;

    channel0 = sensor.readADC_SingleEnded(0);
    gain0 = sensor.getGain();
    channel1 = sensor.readADC_SingleEnded(1);
    gain1 = sensor.getGain();
    channel2 = sensor.readADC_SingleEnded(2);
    gain2 = sensor.getGain();
    channel3 = sensor.readADC_SingleEnded(3);
    gain3 = sensor.getGain();
    // DEBUG_VAR("ADS1015 ADC Channel 0 value:", channel0);
    // DEBUG_VAR("ADS1015 ADC Channel 2 value:", channel1);
    // DEBUG_VAR("ADS1015 ADC Channel 2 value:", channel2);
    // DEBUG_VAR("ADS1015 ADC Channel 3 value:", channel3);
    *a = (int16_t)channel0;
    *b = (int16_t)gain0;
    *c = (int16_t)channel1;
    *d = (int16_t)gain1;
    *e = (int16_t)channel2;
    *f = (int16_t)gain2;
    *g = (int16_t)channel3;
    *h = (int16_t)gain3;
    return (0.0);
  }

private:
  Adafruit_ADS1015 sensor;
};

/**
 *  \class ExternalSensor<Adafruit_ADS1115>
 *  \brief Adafruit_ADS1115 Specialization Class
 *  This is the template specialization
 *  for the Adafruit Analog I2C Interface
 */

template <> class ExternalSensor<Adafruit_ADS1115> : public BaseExternalSensor {
public:
  /**
   *  ExternalSensor():
   *  Adafruit_ADS1115 specific constructor
   */
  ExternalSensor(uint8_t i2c_addr) {
    sensor = Adafruit_ADS1115(i2c_addr);
  }

  /**
   *  begin():
   *  Adafruit_ADS1115 specific begin method
   *
   *  \return true if successful
   */
  virtual uint8_t begin() {
    sensor.begin();
    return (true);
  }

  /**
   *  read(uint16_t *a,uint16_t *b,uint16_t *c,uint16_t *d,uint16_t
   **e,uint16_t *f): Adafruit_ADS1115 specific read method
   *
   *  modifies the input variables to channel0..3 and differential01 ,23
   *values
   */
  virtual float read(int16_t *a, int16_t *b, int16_t *c, int16_t *d, int16_t *e,
                     int16_t *f, int16_t *g, int16_t *h) {
    uint16_t channel0, channel1, channel2, channel3;
    uint16_t gain0, gain1, gain2, gain3;

    channel0 = sensor.readADC_SingleEnded(0);
    gain0 = sensor.getGain();
    channel1 = sensor.readADC_SingleEnded(1);
    gain1 = sensor.getGain();
    channel2 = sensor.readADC_SingleEnded(2);
    gain2 = sensor.getGain();
    channel3 = sensor.readADC_SingleEnded(3);
    gain3 = sensor.getGain();
    // DEBUG_VAR("ADS1015 ADC Channel 0 value:", channel0);
    // DEBUG_VAR("ADS1015 ADC Channel 2 value:", channel1);
    // DEBUG_VAR("ADS1015 ADC Channel 2 value:", channel2);
    // DEBUG_VAR("ADS1015 ADC Channel 3 value:", channel3);
    *a = (int16_t)channel0;
    *b = (int16_t)gain0;
    *c = (int16_t)channel1;
    *d = (int16_t)gain1;
    *e = (int16_t)channel2;
    *f = (int16_t)gain2;
    *g = (int16_t)channel3;
    *h = (int16_t)gain3;
    return (0.0);
  }

private:
  Adafruit_ADS1115 sensor;
};

template <> class ExternalSensor<Gauges> : public BaseExternalSensor {
public:
  /**
   *  ExternalSensor():
   *  CoolGauge specific constructor
   */
  ExternalSensor() {
    sensor = Gauges();
  }

  virtual uint8_t begin() {}

  virtual float read(uint32_t *a, uint32_t *b, uint32_t *c) {
    uint32_t A, B, C;

    A = sensor.readGauge1();
    B = sensor.readGauge2();
    C = sensor.readGauge3();

    // DEBUG_VAR("COOL Gauge 1:", A);
    // DEBUG_VAR("COOL Gauge 2:", B);
    // DEBUG_VAR("COOL Gauge 3:", C);

    *a = A;
    *b = B;
    *c = C;
    return (0.0);
  }

private:
  Gauges sensor;
};

#endif
