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

#ifndef CoolOnBoardLed_H
#define CoolOnBoardLed_H

#include <Arduino.h>

#include <NeoPixelBus.h>

#define OFF 0, 0, 0
#define RED 50, 0, 0
#define BRIGHT_RED 255, 0, 0
#define GREEN 0, 50, 0
#define BRIGHT_GREEN 0, 255, 0
#define WHITE 20, 20, 20
#define BRIGHT_WHITE 255, 255, 255
#define YELLOW 30, 30, 0
#define BLUE 0, 0, 50
#define FUCHSIA 30, 0, 30
#define ORANGE 50, 25, 0

class CoolBoardLed {

public:
  CoolBoardLed() : neoPixelLed(1, 2) {}
  void begin();
  void write(uint8_t R, uint8_t G, uint8_t B);
  bool config();
  void activate();
  void printConf();
  void fade(uint8_t R, uint8_t G, uint8_t B, float T);
  void blink(uint8_t R, uint8_t G, uint8_t B, float T);
  void fadeIn(uint8_t R, uint8_t G, uint8_t B, float T);
  void fadeOut(uint8_t R, uint8_t G, uint8_t B, float T);
  void strobe(uint8_t R, uint8_t G, uint8_t B, float T);

private:
  NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> neoPixelLed;
  bool ledActive = 1;
};

#endif
