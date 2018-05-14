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

#define JETPACK_CLOCK_PIN 4
#define JETPACK_DATA_PIN 15
#define JETPACK_I2C_ENABLE_PIN 5

class Jetpack {

public:
  void begin();
  void write(byte action);
  void writeBit(byte pin, bool state);
  void doAction(JsonObject &root, int hour, int minute);
  void normalAction(int actorNumber, float measurment);
  void invertedAction(int actorNumber, float measurment);
  void temporalActionOff(int actorNumber);
  void temporalActionOn(int actorNumber);
  void mixedTemporalActionOff(int actorNumber, float measurment);
  void mixedTemporalActionOn(int actorNumber, float measurment);
  void hourAction(int actorNumber, int hour);
  void mixedHourAction(int actorNumber, int hour, float measurment);
  void minuteAction(int actorNumber, int minute);
  void mixedMinuteAction(int actorNumber, int minute, float measurment);
  void hourMinuteAction(int actorNumber, int hour, int minute);
  void mixedHourMinuteAction(int actorNumber, int hour, int minute,
                             float measurment);
  bool config();
  void printConf();
  byte action = B00000000;

private:
  struct {
    bool actif = false;
    bool temporal = false;
    bool inverted = false;
    String primaryType = "";
    String secondaryType = "";
    int rangeLow = 0;
    unsigned long timeLow = 0;
    int hourLow = 0;
    int minuteLow = 0;
    int rangeHigh = 0;
    unsigned long timeHigh = 0;
    int hourHigh = 0;
    int minuteHigh = 0;
    unsigned long actifTime = 0;
    unsigned long inactifTime = 0;
    bool failsave = false;
  } actors[8];

};

#endif
