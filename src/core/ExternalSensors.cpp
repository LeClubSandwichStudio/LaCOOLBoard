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
      int16_t channel0, channel1, channel2, channel3;
      int16_t gain0, gain1, gain2, gain3;
      std::unique_ptr<ExternalSensor<Adafruit_ADS1015>> analogI2C(
          new ExternalSensor<Adafruit_ADS1015>(sensors[i].address));

      sensors[i].exSensor = analogI2C.release();
      sensors[i].exSensor->begin();
      sensors[i].exSensor->read(&channel0, &gain0, &channel1, &gain1, &channel2,
                                &gain2, &channel3, &gain3);

    } else if ((sensors[i].reference) == "Adafruit_ADS1115") {
      int16_t channel0, channel1, channel2, channel3;
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
      std::unique_ptr<ExternalSensor<SHT1x>> CoolSHT1x(
          new ExternalSensor<SHT1x>());
      sensors[i].exSensor = CoolSHT1x.release();
      sensors[i].exSensor->begin();
    } else if ((sensors[i].reference) == "SDS011") {
      std::unique_ptr<ExternalSensor<SDS011>> sds011(
          new ExternalSensor<SDS011>());
      sensors[i].exSensor = sds011.release();
      sensors[i].exSensor-> begin();
    } else if ((sensors[i].reference) == "MCP342X_4-20mA") {
      int16_t channel0, channel1, channel2, channel3;

      std::unique_ptr<ExternalSensor<MCP342X>> analogI2C(
          new ExternalSensor<MCP342X>(sensors[i].address));
      sensors[i].exSensor = analogI2C.release();
      sensors[i].exSensor->read(&channel0, &channel1, &channel2, &channel3);
    } else if ((sensors[i].reference) == "I2Cchirp") {
      uint16_t A;
      float B;

      std::unique_ptr<ExternalSensor<I2CSoilMoistureSensor>> i2cSoilMoistureSensor(
          new ExternalSensor<I2CSoilMoistureSensor>(sensors[i].address));
      sensors[i].exSensor = i2cSoilMoistureSensor.release();
      sensors[i].exSensor->read(&A, &B);
    } else if ((sensors[i].reference) == "BME280") {
      float temp, humi, pres;

      std::unique_ptr<ExternalSensor<BME280>> bme280(
          new ExternalSensor<BME280>(sensors[i].address));
      sensors[i].exSensor = bme280.release();
      sensors[i].exSensor->begin();
      sensors[i].exSensor->read(&temp, &humi, &pres);
    } 
  }
}

void ExternalSensors::read(PrintAdapter streamer) {
  if (sensorsNumber > 0) {
    for (uint8_t i = 0; i < sensorsNumber; i++) {
      if (sensors[i].exSensor != NULL) {
        if (sensors[i].reference == "Adafruit_TCS34725") {
          int16_t r, g, b, c, colorTemp, lux;

          CoolMessagePack::msgpckMap(streamer, 4, sensors[i].key);
          sensors[i].exSensor->read(&r, &g, &b, &c, &colorTemp, &lux);
          CoolMessagePack::msgpckInt(streamer, r, sensors[i].kind0);
          CoolMessagePack::msgpckInt(streamer, g, sensors[i].kind1);
          CoolMessagePack::msgpckInt(streamer, b, sensors[i].kind2);
          CoolMessagePack::msgpckInt(streamer, c, sensors[i].kind3);
        } else if (sensors[i].reference == "Adafruit_CCS811") {
          int16_t C, V;
          float T;

          CoolMessagePack::msgpckMap(streamer, 3, sensors[i].key);
          sensors[i].exSensor->read(&C, &V, &T);
          CoolMessagePack::msgpckInt(streamer, C, sensors[i].kind0);
          CoolMessagePack::msgpckInt(streamer, V, sensors[i].kind1);
          CoolMessagePack::msgpckFloat(streamer, T, sensors[i].kind2);
        } else if ((sensors[i].reference == "Adafruit_ADS1015") ||
                   (sensors[i].reference == "Adafruit_ADS1115")) {
          int16_t channel0, channel1, channel2, channel3;
          int16_t gain0, gain1, gain2, gain3;

          CoolMessagePack::msgpckMap(streamer, 8, sensors[i].key);
          sensors[i].exSensor->read(&channel0, &gain0, &channel1, &gain1,
                                    &channel2, &gain2, &channel3, &gain3);
          gain0 = gain0 / 512;
          gain1 = gain1 / 512;
          gain2 = gain2 / 512;
          gain3 = gain3 / 512;
          CoolMessagePack::msgpckInt(streamer, channel0, "0_" + sensors[i].kind0);
          CoolMessagePack::msgpckInt(streamer, gain0, "G0_" + sensors[i].kind0);
          CoolMessagePack::msgpckInt(streamer, channel1, "1_" + sensors[i].kind1);
          CoolMessagePack::msgpckInt(streamer, gain1, "G1_" + sensors[i].kind1);
          CoolMessagePack::msgpckInt(streamer, channel2, "2_" + sensors[i].kind2);
          CoolMessagePack::msgpckInt(streamer, gain2, "G2_" + sensors[i].kind2);
          CoolMessagePack::msgpckInt(streamer, channel3, "3_" + sensors[i].kind3);
          CoolMessagePack::msgpckInt(streamer, gain3, "G3_" + sensors[i].kind3);
        } else if (sensors[i].reference == "CoolGauge") {
          uint32_t A, B, C;

          CoolMessagePack::msgpckMap(streamer, 3, sensors[i].key);
          sensors[i].exSensor->read(&A, &B, &C);
          CoolMessagePack::msgpckInt(streamer, A, sensors[i].kind0);
          CoolMessagePack::msgpckInt(streamer, B, sensors[i].kind1);
          CoolMessagePack::msgpckInt(streamer, C, sensors[i].kind2);
        } else if (sensors[i].reference == "SHT1x") {
          float A, B;

          CoolMessagePack::msgpckMap(streamer, 2, sensors[i].key);
          sensors[i].exSensor->read(&A, &B);
          CoolMessagePack::msgpckFloat(streamer, A, sensors[i].kind0);
          CoolMessagePack::msgpckFloat(streamer, B, sensors[i].kind1);
        } else if (sensors[i].reference == "SDS011") {
          float A, B;

          CoolMessagePack::msgpckMap(streamer, 2, sensors[i].key);
          sensors[i].exSensor->read(&A, &B);
          delay(200);
          CoolMessagePack::msgpckFloat(streamer, A, sensors[i].kind0);
          CoolMessagePack::msgpckFloat(streamer, B, sensors[i].kind1);
        } else if (sensors[i].reference == "MCP342X_4-20mA") {
          int16_t channel0, channel1, channel2, channel3;

          CoolMessagePack::msgpckMap(streamer, 4, sensors[i].key);
          sensors[i].exSensor->read(&channel0, &channel1, &channel2, &channel3);
          CoolMessagePack::msgpckInt(streamer, channel0, sensors[i].kind0);
          CoolMessagePack::msgpckInt(streamer, channel1, sensors[i].kind1);
          CoolMessagePack::msgpckInt(streamer, channel2, sensors[i].kind2);
          CoolMessagePack::msgpckInt(streamer, channel3, sensors[i].kind3);
          DEBUG_VAR("MCP342X Channel 1 Output:",channel0);
          DEBUG_VAR("MCP342X Channel 2 Output:",channel1);
          DEBUG_VAR("MCP342X Channel 3 Output:",channel2);
          DEBUG_VAR("MCP342X Channel 4 Output:",channel3);
        } else if (sensors[i].reference == "I2Cchirp") {
          uint16_t A;
          float B;
          sensors[i].exSensor->read(&A, &B);
          DEBUG_VAR("ChirpSoilMoisture Address:",sensors[i].address);
          DEBUG_VAR("SoilMoisture RAW:", A);
          DEBUG_VAR("SoilTemperature:", B);
          root[sensors[i].kind0] = A; 
          root[sensors[i].kind1] = B;
        } else if (sensors[i].reference == "BME280") {
          float A, B, C;
          sensors[i].exSensor->read(&A, &B, &C);
          root[sensors[i].kind0] = A; 
          root[sensors[i].kind1] = B;
          root[sensors[i].kind2] = C;
          DEBUG_VAR("external BME280 @ I2C address:", sensors[i].address);
          DEBUG_VAR("Temperature : ", A);
          DEBUG_VAR("Pressure : ", B);
          DEBUG_VAR("Humidity : ", C);
        } else {
          CoolMessagePack::msgpckMap(streamer, 1, sensors[i].key);
          CoolMessagePack::msgpckFloat(streamer, sensors[i].exSensor->read(), sensors[i].kind0);
        }
      } else {
        ERROR_VAR("Undefined (NULL) external sensor at index #", i);
      }
    }
  }
}

bool ExternalSensors::config() {
  CoolConfig config("/sensors.json");
  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read /sensors.json");
    return (false);
  }
  JsonObject &sensors = config.get();
  JsonArray &root = sensors["sensors"];
  for (auto kv : root) {
    if (kv["support"] == "external") {
      JsonObject &tryed = kv["measures"];

      CoolConfig::set<String>(kv, "reference",
                              this->sensors[this->sensorsNumber].reference);
      CoolConfig::set<String>(kv, "key",
                              this->sensors[this->sensorsNumber].key);
      if (kv["utils"]["address"].success()) {
        CoolConfig::set<uint8_t>(kv["utils"], "address", this->sensors[this->sensorsNumber].address);
      }
      for (JsonObject::iterator it = tryed.begin(); it != tryed.end(); ++it) {
        String test = it->key;
        if (this->sensors[this->sensorsNumber].kind0 == "") {
          this->sensors[this->sensorsNumber].kind0 = it->key;
        } else if (this->sensors[this->sensorsNumber].kind1 == "") {
          this->sensors[this->sensorsNumber].kind1 = it->key;
        } else if (this->sensors[this->sensorsNumber].kind2 == "") {
          this->sensors[this->sensorsNumber].kind2 = it->key;
        } else if (this->sensors[this->sensorsNumber].kind3 == "") {
          this->sensors[this->sensorsNumber].kind3 = it->key;
        }
      }
      this->sensorsNumber++;
    }
  }
  DEBUG_LOG("External sensors configuration loaded");
  this->printConf(this->sensors);
  return (true);
}

void ExternalSensors::printConf(Sensor sensors[]) {
  INFO_LOG("External sensors configuration");
  INFO_VAR("Number of external sensors =", this->sensorsNumber);

  for (uint8_t i = 0; i < this->sensorsNumber; i++) {
    INFO_VAR("Sensor #", i);
    INFO_VAR("  Reference =", sensors[i].reference);
    DEBUG_VAR("  Key      =", sensors[i].key);
    DEBUG_VAR("  Address  =", sensors[i].address);
    DEBUG_VAR("  Kind (0) =", sensors[i].kind0);
    DEBUG_VAR("  Kind (1) =", sensors[i].kind1);
    DEBUG_VAR("  Kind (2) =", sensors[i].kind2);
    DEBUG_VAR("  Kind (3) =", sensors[i].kind3);
  }
}