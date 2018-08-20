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

#include <ArduinoJson.h>
#include <NeoPixelBus.h>

#include "CoolBoardLed.h"
#include "CoolConfig.h"
#include "CoolLog.h"

void CoolBoardLed::fade(uint8_t r, uint8_t g, uint8_t b, float t) {
  TRACE_VAR("Red value:", r);
  TRACE_VAR("Green value:", g);
  TRACE_VAR("Blue value:", b);
  TRACE_VAR("Duration", t);

  if (this->ledActive == 1) {
    for (int k = 0; k < 1000; k++) {
      this->neoPixelLed.SetPixelColor(
          0, RgbColor(k * r / 1000, k * g / 1000, k * b / 1000));
      this->neoPixelLed.Show();
      delay(t);
    }

    for (int k = 1000; k >= 0; k--) {
      this->neoPixelLed.SetPixelColor(
          0, RgbColor(k * r / 1000, k * g / 1000, k * b / 1000));
      this->neoPixelLed.Show();
      delay(t);
    }
  }
}

void CoolBoardLed::blink(uint8_t r, uint8_t g, uint8_t b, float t) {
  TRACE_VAR("Red value:", r);
  TRACE_VAR("Green value:", g);
  TRACE_VAR("Blue value:", b);
  TRACE_VAR("Duration", t);

  if (this->ledActive == 1) {
    this->neoPixelLed.SetPixelColor(0, RgbColor(r, g, b));
    this->neoPixelLed.Show();
    delay(t * 1000);
    this->neoPixelLed.SetPixelColor(0, RgbColor(0, 0, 0));
    this->neoPixelLed.Show();
    delay(t * 1000);
  }
}

void CoolBoardLed::fadeIn(uint8_t r, uint8_t g, uint8_t b, float t) {
  TRACE_VAR("Red value:", r);
  TRACE_VAR("Green value:", g);
  TRACE_VAR("Blue value:", b);
  TRACE_VAR("Duration", t);

  if (this->ledActive == 1) {
    for (int k = 0; k < 1000; k++) {
      this->neoPixelLed.SetPixelColor(
          0, RgbColor(k * r / 1000, k * g / 1000, k * b / 1000));
      this->neoPixelLed.Show();
      delay(t);
    }
  }
}

void CoolBoardLed::fadeOut(uint8_t r, uint8_t g, uint8_t b, float t) {
  TRACE_VAR("Red value:", r);
  TRACE_VAR("Green value:", g);
  TRACE_VAR("Blue value:", b);
  TRACE_VAR("Duration", t);

  if (this->ledActive == 1) {
    for (int k = 1000; k >= 0; k--) {
      this->neoPixelLed.SetPixelColor(
          0, RgbColor(k * r / 1000, k * g / 1000, k * b / 1000));
      this->neoPixelLed.Show();
      delay(t);
    }
  }
}

void CoolBoardLed::strobe(uint8_t r, uint8_t g, uint8_t b, float t) {
  TRACE_VAR("Red value:", r);
  TRACE_VAR("Green value:", g);
  TRACE_VAR("Blue value:", b);
  TRACE_VAR("Duration", t);

  if (this->ledActive == 1) {
    for (int k = 1000; k >= 0; k--) {
      this->neoPixelLed.SetPixelColor(0, RgbColor(r, g, b));
      this->neoPixelLed.Show();
      delay(t);
      this->neoPixelLed.SetPixelColor(0, RgbColor(0, 0, 0));
      this->neoPixelLed.Show();
      delay(t);
    }
  }
}

void CoolBoardLed::begin() {
  yield();
  if (this->ledActive == 1) {
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    this->neoPixelLed.Begin();
    this->neoPixelLed.Show();
  }
}

void CoolBoardLed::write(uint8_t r, uint8_t g, uint8_t b) {
  TRACE_VAR("Red value:", r);
  TRACE_VAR("Green value:", g);
  TRACE_VAR("Blue value:", b);

  if (this->ledActive == 1) {
    this->neoPixelLed.SetPixelColor(0, RgbColor(r, g, b));
    this->neoPixelLed.Show();
  }
}

bool CoolBoardLed::config() {
  CoolConfig config("/coolBoardLedConfig.json");

  if (!config.readFileAsJson()) {
    ERROR_LOG("Failed to read LED configuration");
    return (false);
  }
  JsonObject &json = config.get();
  config.set(json, "ledActive", this->ledActive);
  INFO_LOG("LED configuration loaded");
  return (true);
}

void CoolBoardLed::printConf() {
  INFO_LOG("LED configuration");
  INFO_VAR("  LED active =", ledActive);
}

void CoolBoardLed::activate() { this->ledActive = 1; }
