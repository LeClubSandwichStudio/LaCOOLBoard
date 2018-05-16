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

#include "ExternalSensors.h"

OneWire oneWire(0);

void ExternalSensors::begin() {

  Sensor *sensors = new Sensor[this->sensorsNumber];

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
    }
  }
  delete[] sensors; // need to test if we delete the struct array, sensors rest
}

void ExternalSensors::read(JsonObject &root) {

  Sensor *sensors = new Sensor[this->sensorsNumber];

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
        } else {
          root[sensors[i].type] = sensors[i].exSensor->read();
        }
      } else {
        ERROR_VAR("Undefined (NULL) external sensor at index #", i);
      }
    }
  }
  DEBUG_JSON("External sensors data:", root);
  delete[] sensors;
}

bool ExternalSensors::config() {
  File configFile = SPIFFS.open("/externalSensorsConfig.json", "r");

  if (!configFile) {
    ERROR_LOG("Failed to read /externalSensorsConfig.json");
    return (false);
  } else {
    String data = configFile.readString();
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(data);

    if (!json.success()) {
      ERROR_LOG("Failed to parse external sensors config from file");
      return (false);
    } else {
      DEBUG_JSON("External sensor config JSON:", json);
      DEBUG_VAR("JSON buffer size:", jsonBuffer.size());

      if (json["sensorsNumber"] != NULL) {
        this->sensorsNumber = json["sensorsNumber"];

        Sensor *sensors = new Sensor[sensorsNumber];

        for (uint8_t i = 0; i < sensorsNumber; i++) {
          String name = "sensor" + String(i);

          if (json[name].success()) {
            JsonObject &sensorJson = json[name];

            if (sensorJson["reference"].success()) {
              sensors[i].reference = sensorJson["reference"].as<String>();
            }
            sensorJson["reference"] = sensors[i].reference;

            if (sensorJson["type"].success()) {
              sensors[i].type = sensorJson["type"].as<String>();
            }
            sensorJson["type"] = sensors[i].type;

            if (sensorJson["address"].success()) {
              sensors[i].address = sensorJson["address"];
            }
            sensorJson["address"] = sensors[i].address;

            if (sensorJson["kind0"].success()) {
              sensors[i].kind0 = sensorJson["kind0"].as<String>();
            }
            sensorJson["kind0"] = sensors[i].kind0;

            if (sensorJson["kind1"].success()) {
              sensors[i].kind1 = sensorJson["kind1"].as<String>();
            }
            sensorJson["kind1"] = sensors[i].kind1;

            if (sensorJson["kind2"].success()) {
              sensors[i].kind2 = sensorJson["kind2"].as<String>();
            }
            sensorJson["kind2"] = sensors[i].kind2;

            if (sensorJson["kind3"].success()) {
              sensors[i].kind3 = sensorJson["kind3"].as<String>();
            }
            sensorJson["sensor3"] = sensors[i].kind3;
          }
          json[name]["reference"] = sensors[i].reference;
          json[name]["type"] = sensors[i].type;
          json[name]["address"] = sensors[i].address;
          json[name]["kind0"] = sensors[i].kind0;
          json[name]["kind1"] = sensors[i].kind1;
          json[name]["kind2"] = sensors[i].kind2;
          json[name]["kind3"] = sensors[i].kind3;
        }
        this->printConf(sensors);
        delete[] sensors;
      }
      json["sensorsNumber"] = this->sensorsNumber;
      configFile.close();
      configFile = SPIFFS.open("/externalSensorsConfig.json", "w");

      if (!configFile) {
        ERROR_LOG("Failed to write to /externalSensorsConfig.json");
        return (false);
      }
      json.printTo(configFile);
      configFile.close();
      DEBUG_LOG("Saved external sensors config to /externalSensorsConfig.json");
      return (true);
    }
  }
}

void ExternalSensors::printConf(Sensor sensors[]) {
  INFO_LOG("External sensors configuration");
  INFO_VAR("Number of external sensors:", this->sensorsNumber);

  for (uint8_t i = 0; i < this->sensorsNumber; i++) {
    DEBUG_VAR("Sensor #", i);
    DEBUG_VAR("  Reference:", sensors[i].reference);
    DEBUG_VAR("  Type     :", sensors[i].type);
    DEBUG_VAR("  Address  :", sensors[i].address);
  }
}