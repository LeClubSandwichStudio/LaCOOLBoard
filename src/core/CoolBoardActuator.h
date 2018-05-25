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

#ifndef COOLBOARDACTUATOR_H
#define COOLBOARDACTUATOR_H

#include <Arduino.h>
#include <ArduinoJson.h>

#define ONBOARD_ACTUATOR_PIN 15

class CoolBoardActuator {

public:
  void begin();
  bool getStatus();
  void write(bool action);
  bool doAction(JsonObject &data, uint8_t hour, uint8_t minute);
  void normalAction(float measurment);
  void invertedAction(float measurment);
  void temporalActionOff();
  void temporalActionOn();
  void mixedTemporalActionOff(float measurment);
  void mixedTemporalActionOn(float measurment);
  void hourAction(uint8_t hour);
  void mixedHourAction(uint8_t hour, float measurment);
  void minuteAction(uint8_t minute);
  void mixedMinuteAction(uint8_t minute, float measurment);
  void hourMinuteAction(uint8_t hour, uint8_t minute);
  void mixedHourMinuteAction(uint8_t hour, uint8_t minute, float measurment);
  bool config();
  void printConf();

  bool state = 0;
  bool actif = false;
  bool temporal = false;
  bool inverted = false;
  String primaryType = "";
  String secondaryType = "";
  int rangeLow = 0;
  unsigned long timeLow = 0;
  uint8_t hourLow = 0;
  uint8_t minuteLow = 0;
  int rangeHigh = 0;
  unsigned long timeHigh = 0;
  uint8_t hourHigh = 0;
  uint8_t minuteHigh = 0;
  unsigned long actifTime = 0;
  unsigned long inactifTime = 0;
  bool failsave = false;
};

#endif
