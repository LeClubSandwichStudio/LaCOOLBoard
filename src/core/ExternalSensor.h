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

#ifndef EXTERNALSENSOR_H
#define EXTERNALSENSOR_H

#include "CoolAdafruit_ADS1015.h"
#include "CoolAdafruit_CCS811.h"
#include "CoolAdafruit_TCS34725.h"
#include "CoolGauge.h"
#include "CoolNDIR_I2C.h"
#include "SHT1x.h"
#include "CoolSDS011.h"
#include <Arduino.h>
#include <DallasTemperature.h>

#include "CoolLog.h"

#define SHT1X_DATA_PIN 0
#define SHT1X_CLOCK_PIN 12

class BaseExternalSensor {

public:
  BaseExternalSensor() {}
  virtual uint8_t begin() { return (-2); }
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
  virtual float read(float *a, float *b) { return (-42.42); }
};

template <class T> class ExternalSensor : public BaseExternalSensor {
public:
  ExternalSensor() { sensor(); }
  virtual uint8_t begin() { return (sensor.begin()); }
  virtual float read() { return (sensor.read()); }

private:
  T sensor;
};

template <> class ExternalSensor<NDIR_I2C> : public BaseExternalSensor {
public:
  ExternalSensor(uint8_t i2c_addr) { sensor = NDIR_I2C(i2c_addr); }
  virtual uint8_t begin() {
    if (sensor.begin()) {
      delay(10000);
      return (true);
    } else {
      return (false);
    }
  }

  virtual float read() {
    if (sensor.measure()) {
      return ((float)sensor.ppm);
    } else {
      return (-42);
    }
  }

private:
  NDIR_I2C sensor = NULL;
};

template <>
class ExternalSensor<DallasTemperature> : public BaseExternalSensor {
public:
  ExternalSensor(OneWire *oneWire) { sensor = DallasTemperature(oneWire); }

  virtual uint8_t begin() {
    sensor.begin();
    delay(5);
    sensor.getAddress(this->dallasAddress, 0);
    return (true);
  }

  virtual float read() {
    sensor.requestTemperatures();
    float result = (float)sensor.getTempCByIndex(0);
    return (result);
  }

private:
  DallasTemperature sensor;
  DeviceAddress dallasAddress;
};

template <>
class ExternalSensor<Adafruit_TCS34725> : public BaseExternalSensor {
public:
  ExternalSensor() {
    sensor =
        Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);
  }

  virtual uint8_t begin() {
    if (sensor.begin()) {
      return (true);
    } else {
      return (false);
    }
  }

  virtual float read(int16_t *a, int16_t *b, int16_t *c, int16_t *d, int16_t *e,
                     int16_t *f) {
    uint16_t internR, internG, internB, internC, internColorTemp, internLux;

    sensor.getRawData(&internR, &internG, &internB, &internC);

    internColorTemp =
        sensor.calculateColorTemperature(internR, internG, internB);

    internLux = sensor.calculateLux(internR, internG, internB);

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

template <> class ExternalSensor<Adafruit_CCS811> : public BaseExternalSensor {
public:
  ExternalSensor(uint8_t i2c_addr) { sensor = Adafruit_CCS811(); }

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

    *a = (int16_t)internC;
    *b = (int16_t)internV;
    *c = internT;
    return (0.0);
  }

private:
  Adafruit_CCS811 sensor;
};

template <> class ExternalSensor<Adafruit_ADS1015> : public BaseExternalSensor {
public:
  ExternalSensor(uint8_t i2c_addr) { sensor = Adafruit_ADS1015(i2c_addr); }

  virtual uint8_t begin() {
    sensor.begin();
    return (true);
  }

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

template <> class ExternalSensor<Adafruit_ADS1115> : public BaseExternalSensor {
public:
  ExternalSensor(uint8_t i2c_addr) { sensor = Adafruit_ADS1115(i2c_addr); }

  virtual uint8_t begin() {
    sensor.begin();
    return (true);
  }

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
  ExternalSensor() { sensor = Gauges(); }

  virtual uint8_t begin() {}

  virtual float read(uint32_t *a, uint32_t *b, uint32_t *c) {
    uint32_t A, B, C;

    A = sensor.readGauge1();
    B = sensor.readGauge2();
    C = sensor.readGauge3();

    *a = A;
    *b = B;
    *c = C;
    return (0.0);
  }

private:
  Gauges sensor;
};

template <> class ExternalSensor<SHT1x> : public BaseExternalSensor {
public:
  ExternalSensor() : sensor(SHT1X_DATA_PIN, SHT1X_CLOCK_PIN) {}
  virtual uint8_t begin() {}
  virtual float read(float *a, float *b) {
    float A, B, C;
    A = sensor.readHumidity();
    B = sensor.readTemperatureC();
    *a = A;
    *b = B;
    return (0.0);
  }

private:
  SHT1x sensor;
};

template <> class ExternalSensor<SDS011> : public BaseExternalSensor {
public:
  ExternalSensor() :  sensor() {}
  virtual uint8_t begin() {
    sensor.start();
  }

  virtual float read(float *a, float *b) {
    float A, B;

    sensor.read();
    A = sensor.pm10();
    B = sensor.pm25();
    *a = A;
    *b = B;
    return (0.0);
  }

private:
  SDS011 sensor;
};
#endif
