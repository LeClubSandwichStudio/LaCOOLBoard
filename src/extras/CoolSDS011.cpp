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

#include "CoolSDS011.h"

#include <Wire.h>

SDS011::SDS011() {}

bool SDS011::start() {
  Wire.beginTransmission(SDS011_ADRESS);
  Wire.write(SDS011_START);
  Wire.endTransmission();
  state = true;
  return true;
}

bool SDS011::stop() {
  Wire.beginTransmission(SDS011_ADRESS);
  Wire.write(SDS011_STOP);
  Wire.endTransmission();
  state = false;
  return true;
}

bool SDS011::read() {
  char temp10[5];
  char temp25[5];
  int16_t ftemp10 = -1;
  int16_t ftemp25 = -1;

  Wire.beginTransmission(SDS011_ADRESS);
  Wire.write(SDS011_QUERY);
  Wire.endTransmission();
  delay(200);
  Wire.requestFrom(SDS011_ADRESS, 8);
  while (Wire.available() < 7) {
    delay(10);
  }
  if (Wire.available() >= 8) {
    temp10[0] = Wire.read();
    temp10[1] = Wire.read();
    temp10[2] = Wire.read();
    temp10[3] = Wire.read();
    temp10[4] = '\0';
    temp25[0] = Wire.read();
    temp25[1] = Wire.read();
    temp25[2] = Wire.read();
    temp25[3] = Wire.read();
    temp25[4] = '\0';
    ftemp10 = atoi(temp10);
    ftemp25 = atoi(temp25);
    lastPM10 = float(ftemp10) / 10.0;
    lastPM25 = float(ftemp25) / 10.0;
    return true;
  }
  return false;
}

float SDS011::pm10() {
  read();
  return lastPM10;
}

float SDS011::pm25() {
  read();
  return lastPM25;
}