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

#ifndef COOLCONFIG_H
#define COOLCONFIG_H

#include "ArduinoJson.h"
#include <Arduino.h>

class CoolConfig {

private:
  const char *path;
  JsonVariant json;
  DynamicJsonBuffer buffer;

public:
  CoolConfig(const char *path);
  bool readFileAsJson();
  void setConfig(JsonVariant json);
  JsonObject &get();
  bool writeJsonToFile();
  template <typename T>
  static void set(JsonObject &json, const char *key, T &val, bool overwrite = false) {
    if (!overwrite && json[key].success()) {
      val = json[key].as<T>();
    } else {
      json[key] = val;
    }
  };
  template <typename T>
  static void setArray(JsonObject &json, const char *key, const uint8_t i, T &val, bool overwrite = false) {
    if (!overwrite && json[key][i].success()) {
      val = json[key][i].as<T>();
    } else {
      json[key][i] = val;
    }
  };
};

#endif