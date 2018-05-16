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
#include "CoolLog.h"

/**
 *  CoolBoardLed::fade ( Red , Green , Blue, Time in seconds ):
 *  fade animation:  Fade In over T(seconds)
 *      Fade Out over T(seconds)
 */
void CoolBoardLed::fade(uint8_t R, uint8_t G, uint8_t B, float T) {
  TRACE_VAR("Red value:", R);
  TRACE_VAR("Green value:", G);
  TRACE_VAR("Blue value:", B);
  TRACE_VAR("Duration", T);

  if (this->ledActive == 1) {
    for (int k = 0; k < 1000; k++) {
      this->neoPixelLed.SetPixelColor(
          0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
      this->neoPixelLed.Show();
      delay(T);
    }

    for (int k = 1000; k >= 0; k--) {
      this->neoPixelLed.SetPixelColor(
          0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
      this->neoPixelLed.Show();
      delay(T);
    }
  }
}

/**
*  CoolBoardLed::blink( Red , Green , Blue , Time in seconds ):
*  Blink animation:  Led On for T seconds
                                Led off
*/
void CoolBoardLed::blink(uint8_t R, uint8_t G, uint8_t B, float T) {
  TRACE_VAR("Red value:", R);
  TRACE_VAR("Green value:", G);
  TRACE_VAR("Blue value:", B);
  TRACE_VAR("Duration", T);

  if (this->ledActive == 1) {
    this->neoPixelLed.SetPixelColor(0, RgbColor(R, G, B));
    this->neoPixelLed.Show();
    delay(T * 1000);
    this->neoPixelLed.SetPixelColor(0, RgbColor(0, 0, 0));
    this->neoPixelLed.Show();
  }
}

/**
 *  CoolBoardLed::fadeIn(Red , Green , Blue , Time in seconds)
 *  Fade In animation:  gradual increase over T(seconds)
 */
void CoolBoardLed::fadeIn(uint8_t R, uint8_t G, uint8_t B, float T) {
  TRACE_VAR("Red value:", R);
  TRACE_VAR("Green value:", G);
  TRACE_VAR("Blue value:", B);
  TRACE_VAR("Duration", T);

  if (this->ledActive == 1) {
    for (int k = 0; k < 1000; k++) {
      this->neoPixelLed.SetPixelColor(
          0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
      this->neoPixelLed.Show();
      delay(T);
    }
  }
}

/**
 *  CoolBoardLed::fadeOut( Red , Green , Blue , Time in seconds)
 *  Fade Out animation:  gradual decrease over T(seconds)
 */
void CoolBoardLed::fadeOut(uint8_t R, uint8_t G, uint8_t B, float T) {
  TRACE_VAR("Red value:", R);
  TRACE_VAR("Green value:", G);
  TRACE_VAR("Blue value:", B);
  TRACE_VAR("Duration", T);

  if (this->ledActive == 1) {
    for (int k = 1000; k >= 0; k--) {
      this->neoPixelLed.SetPixelColor(
          0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
      this->neoPixelLed.Show();
      delay(T);
    }
  }
}

/**
 *  CoolBoardLed::strobe(Red , Green , Blue , Time in seconds)
 *  Strobe animation:  blinks over T(seconds)
 */
void CoolBoardLed::strobe(uint8_t R, uint8_t G, uint8_t B, float T) {
  TRACE_VAR("Red value:", R);
  TRACE_VAR("Green value:", G);
  TRACE_VAR("Blue value:", B);
  TRACE_VAR("Duration", T);

  if (this->ledActive == 1) {
    for (int k = 1000; k >= 0; k--) {
      this->neoPixelLed.SetPixelColor(0, RgbColor(R, G, B));
      this->neoPixelLed.Show();
      delay(T);
      this->neoPixelLed.SetPixelColor(0, RgbColor(0, 0, 0));
      this->neoPixelLed.Show();
      delay(T);
    }
  }
}


/**
 *  CoolBoardLed::begin():
 *  This method is provided to start the Led Object
 *  by setting the correct pin and creating a dynamic
 *  neoPixelBus
 */
void CoolBoardLed::begin() {
  yield();
  if (this->ledActive == 1) {
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    this->neoPixelLed.Begin();
    this->neoPixelLed.Show();
  }
}

/**
 *  CoolBoardLed::write(Red,Green,Blue):
 *  This method is provided to set the
 *  Color of the Led
 */
void CoolBoardLed::write(uint8_t R, uint8_t G, uint8_t B) {
  TRACE_VAR("Red value:", R);
  TRACE_VAR("Green value:", G);
  TRACE_VAR("Blue value:", B);

  if (this->ledActive == 1) {
    this->neoPixelLed.SetPixelColor(0, RgbColor(R, G, B));
    this->neoPixelLed.Show();
  }
}

/**
 *  CoolBoardLed::config():
 *  This method is provided to configure
 *  the Led Object :  -ledActive=0 : deactivated
 *        -ledActive=1 : activated
 *  \return true if the configuration done,
 *  false otherwise
 */
bool CoolBoardLed::config() {
  File configFile = SPIFFS.open("/coolBoardLedConfig.json", "r");

  if (!configFile) {
    ERROR_LOG("Failed to read /coolBoardLedConfig.json");
    return (false);
  } else {
    String data = configFile.readString();
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(data);
    if (!json.success()) {
      ERROR_LOG("Failed to parse JSON LED config from file");
      return (false);
    } else {
      DEBUG_JSON("LED config JSON:", json);
      DEBUG_VAR("JSON buffer size:", jsonBuffer.size());
      if (json["ledActive"].success()) {
        this->ledActive = json["ledActive"];
      }
      json["ledActive"] = this->ledActive;
      configFile.close();
      configFile = SPIFFS.open("/coolBoardLedConfig.json", "w");
      if (!configFile) {
        ERROR_LOG("Failed to write to /coolBoardLedConfig.json");
        return (false);
      }
      json.printTo(configFile);
      configFile.close();
      INFO_LOG("Saved LED config to /coolBoardLedConfig.json");
      return (true);
    }
  }
}

/**
 *  CoolBoardLed::printConf():
 *  This method is provided to print the
 *  Led Object Configuration to the Serial
 *  Monitor
 */
void CoolBoardLed::printConf() {
  INFO_LOG("LED configuration");
  INFO_VAR("  LED active =", ledActive);
}

/**
 *  CoolBoardLed::activate():
 *  This method is provided to activate the
 *  Led Object without the configuration
 *  file
 */
void CoolBoardLed::activate() { this->ledActive = 1; }
