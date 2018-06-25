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

#include <OneWire.h>

#include "CoolConfig.h"
#include "ExternalSensors.h"

OneWire oneWire(0);

void ExternalSensors::begin() {

  for (uint8_t i = 0; i < this->sensorsNumber; i++) {
    if ((sensors[i].reference) == "NDIR_I2C") {
      std::unique_ptr<ExternalSensor<NDIR_I2C>> sensorCO2(
          new ExternalSensor<NDIR_I2C>(sensors[i].address));

      sensors[i].exSensor = sensorCO2.release();
      sensors[i].exSensor->begin();
      sensors[i].exSensor->read();
    } else if ((sensors[i].reference) == "DallasTemperature") {
      std::unique_ptr<ExternalSensor<DallasTemperature>> dallasTemp(
          new ExternalSensor<DallasTemperature>(&oneWire));

      sensors[i].exSensor = dallasTemp.release();
      sensors[i].exSensor->begin();
      sensors[i].exSensor->read();
    } else if ((sensors[i].reference) == "Adafruit_TCS34725") {
      int16_t r, g, b, c, colorTemp, lux;
      std::unique_ptr<ExternalSensor<Adafruit_TCS34725>> rgbSensor(
          new ExternalSensor<Adafruit_TCS34725>());

      sensors[i].exSensor = rgbSensor.release();
      sensors[i].exSensor->begin();
      sensors[i].exSensor->read(&r, &g, &b, &c, &colorTemp, &lux);
    } else if ((sensors[i].reference) == "Adafruit_CCS811") {
      int16_t C02, VOT;
      float Temp;
      std::unique_ptr<ExternalSensor<Adafruit_CCS811>> aqSensor(
          new ExternalSensor<Adafruit_CCS811>(sensors[i].address));

      sensors[i].exSensor = aqSensor.release();
      sensors[i].exSensor->begin();
      sensors[i].exSensor->read(&C02, &VOT, &Temp);
    } else if ((sensors[i].reference) == "Adafruit_ADS1015") {
      int16_t channel0, channel1, channel2, channel3, diff01, diff23;
      int16_t gain0, gain1, gain2, gain3;
      std::unique_ptr<ExternalSensor<Adafruit_ADS1015>> analogI2C(
          new ExternalSensor<Adafruit_ADS1015>(sensors[i].address));

      sensors[i].exSensor = analogI2C.release();
      sensors[i].exSensor->begin();
      sensors[i].exSensor->read(&channel0, &gain0, &channel1, &gain1, &channel2,
                                &gain2, &channel3, &gain3);

    } else if ((sensors[i].reference) == "Adafruit_ADS1115") {
      int16_t channel0, channel1, channel2, channel3, diff01, diff23;
      int16_t gain0, gain1, gain2, gain3;

      std::unique_ptr<ExternalSensor<Adafruit_ADS1115>> analogI2C(
          new ExternalSensor<Adafruit_ADS1115>(sensors[i].address));
      sensors[i].exSensor = analogI2C.release();
      sensors[i].exSensor->read(&channel0, &gain0, &channel1, &gain1, &channel2,
                                &gain2, &channel3, &gain3);
    } else if ((sensors[i].reference) == "CoolGauge") {
      uint32_t A, B, C;
      std::unique_ptr<ExternalSensor<Gauges>> gauge(
          new ExternalSensor<Gauges>());

      sensors[i].exSensor = gauge.release();
      sensors[i].exSensor->read(&A, &B, &C);
    } else if ((sensors[i].reference) == "SHT1x") {
      float A, B, C;
      std::unique_ptr<ExternalSensor<SHT1x>> CoolSHT1x(
          new ExternalSensor<SHT1x>());
      sensors[i].exSensor = CoolSHT1x.release();
      sensors[i].exSensor->begin();
    } else if ((sensors[i].reference) == "SDS011") {
      float A, B;
      std::unique_ptr<ExternalSensor<SDS011>> sds011(
          new ExternalSensor<SDS011>());
      sensors[i].exSensor = sds011.release();
      sensors[i].exSensor-> begin();
    }
  }
}

void ExternalSensors::read(JsonObject &root) {

  if (sensorsNumber > 0) {
    for (uint8_t i = 0; i < sensorsNumber; i++) {
      if (sensors[i].exSensor != NULL) {
        if (sensors[i].reference == "Adafruit_TCS34725") {
          int16_t r, g, b, c, colorTemp, lux;

          sensors[i].exSensor->read(&r, &g, &b, &c, &colorTemp, &lux);
          root[sensors[i].kind0] = r;
          root[sensors[i].kind1] = g;
          root[sensors[i].kind2] = b;
          root[sensors[i].kind3] = c;
        } else if (sensors[i].reference == "Adafruit_CCS811") {
          int16_t C, V;
          float T;

          sensors[i].exSensor->read(&C, &V, &T);
          root[sensors[i].kind0] = C;
          root[sensors[i].kind1] = V;
          root[sensors[i].kind2] = T;
        } else if ((sensors[i].reference == "Adafruit_ADS1015") ||
                   (sensors[i].reference == "Adafruit_ADS1115")) {
          int16_t channel0, channel1, channel2, channel3, diff01, diff23;
          int16_t gain0, gain1, gain2, gain3;

          sensors[i].exSensor->read(&channel0, &gain0, &channel1, &gain1,
                                    &channel2, &gain2, &channel3, &gain3);
          gain0 = gain0 / 512;
          gain1 = gain1 / 512;
          gain2 = gain2 / 512;
          gain3 = gain3 / 512;
          root["0_" + sensors[i].kind0] = channel0;
          root["G0_" + sensors[i].kind0] = gain0;
          root["1_" + sensors[i].kind1] = channel1;
          root["G1_" + sensors[i].kind1] = gain1;
          root["2_" + sensors[i].kind2] = channel2;
          root["G2_" + sensors[i].kind2] = gain2;
          root["3_" + sensors[i].kind3] = channel3;
          root["G3_" + sensors[i].kind3] = gain3;
        } else if (sensors[i].reference == "CoolGauge") {
          uint32_t A, B, C;
          sensors[i].exSensor->read(&A, &B, &C);
          root[sensors[i].kind0] = A;
          root[sensors[i].kind1] = B;
          root[sensors[i].kind2] = C;
        } else if (sensors[i].reference == "SHT1x") {
          float A, B, C;
          sensors[i].exSensor->read(&A, &B);
          root[sensors[i].kind0] = A;
          root[sensors[i].kind1] = B;
        } else if (sensors[i].reference == "SDS011") {
          float A, B;
          sensors[i].exSensor->read(&A, &B);
          delay(200);
          root[sensors[i].kind0] = A; //PM10
          root[sensors[i].kind1] = B; //PM2.5
        } else {
          root[sensors[i].type] = sensors[i].exSensor->read();
        }
      } else {
        ERROR_VAR("Undefined (NULL) external sensor at index #", i);
      }
    }
  }
  DEBUG_JSON("External sensors data:", root);
}

bool ExternalSensors::config() {
  CoolConfig config("/externalSensorsConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read external sensors configuration");
    return (false);
  }
  JsonObject &json = config.get();
  config.set<uint8_t>(json, "sensorsNumber", this->sensorsNumber);
  for (uint8_t i = 0; i < sensorsNumber; i++) {
    String name = "sensor" + String(i);

    if (!json[name].success()) {
      json.createNestedObject(name);
    }
    JsonObject &sensor = json[name];
    config.set<String>(sensor, "reference", sensors[i].reference);
    config.set<String>(sensor, "type", sensors[i].type);
    config.set<uint8_t>(sensor, "address", sensors[i].address);
    config.set<String>(sensor, "kind0", sensors[i].kind0);
    config.set<String>(sensor, "kind1", sensors[i].kind1);
    config.set<String>(sensor, "kind2", sensors[i].kind2);
    config.set<String>(sensor, "kind3", sensors[i].kind3);
  }
  DEBUG_LOG("External sensors configuration loaded");
  this->printConf(sensors);
  return (true);
}

void ExternalSensors::printConf(Sensor sensors[]) {
  INFO_LOG("External sensors configuration");
  INFO_VAR("Number of external sensors =", this->sensorsNumber);

  for (uint8_t i = 0; i < this->sensorsNumber; i++) {
    INFO_VAR("Sensor #", i);
    INFO_VAR("  Reference =", sensors[i].reference);
    DEBUG_VAR("  Type     =", sensors[i].type);
    DEBUG_VAR("  Address  =", sensors[i].address);
    DEBUG_VAR("  Kind (0) =", sensors[i].kind0);
    DEBUG_VAR("  Kind (1) =", sensors[i].kind1);
    DEBUG_VAR("  Kind (2) =", sensors[i].kind2);
    DEBUG_VAR("  Kind (3) =", sensors[i].kind3);
  }
}