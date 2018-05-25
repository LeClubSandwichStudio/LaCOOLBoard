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

#ifndef JETPACK_H
#define JETPACK_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "CoolBoardActuator.h"

#define JETPACK_CLOCK_PIN 4
#define JETPACK_DATA_PIN 15
#define JETPACK_I2C_ENABLE_PIN 5

class Jetpack {

public:
  void begin();
  void write(byte action);
  void writeBit(byte pin, bool state);
  void doAction(JsonObject &root, int hour, int minute);
  bool config();
  void printConf();

private:

  uint8_t action = B00000000;
  CoolBoardActuator actuatorList[8];
};

#endif
