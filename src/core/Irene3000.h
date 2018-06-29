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

#ifndef IRENE3000_H
#define IRENE3000_H

#include <Arduino.h>

#include "CoolAdafruit_ADS1015.h"

#include "CoolBoardLed.h"

#define ADC_MAXIMUM_VALUE 32767
#define REFERENCE_VOLTAGE_GAIN_4 1.024
#define V_GAIN_2 0.0625
#define V_GAIN_4 0.03125
#define V_GAIN_8 0.015625

#define REF_VOLTAGE 1.024
#define OPAMP_GAIN 5.25

#define BUTTON_CHANNEL 0
#define TEMP_CHANNEL 1
#define PH_CHANNEL 3
#define FREE_ADC_CHANNEL 2

class Irene3000 {

public:
  void begin();
  bool config(bool overWrite = false);
  void printConf();
  void read(JsonObject &root);
  int readButton();
  int readADSChannel2();
  void readPh(JsonObject &root);
  void readTemp(JsonObject &root);
  void resetParams();
  void calibratepH7();
  void calibratepH4();
  void saveCalibrationDate();
  void readLastCalibrationDate(JsonObject &root);
  void calcpHSlope();
  adsGain_t gainConvert(uint16_t tempGain);
  void waitForButtonPress();
  bool isButtonPressed();
  void calibrate(CoolBoardLed &led);

private:
  Adafruit_ADS1115 ads;
  struct {
    int pH7Cal = 0;
    int pH4Cal = 0;
    float pHStep = 1;
    String calibrationDate = "0000-00-00T00:00:00Z";
  } params;
  struct {
    bool active = 0;
    int gain = GAIN_TWOTHIRDS;
    String type = "unknown";
  } waterTemp, phProbe, adc2;
};

#endif