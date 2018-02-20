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

#include "Arduino.h"
#include "FS.h"

#include "ArduinoJson.h"
#include "CoolBoardLed.h"
#include "CoolLog.h"
#include <NeoPixelBus.h>

/**
 *  CoolBoardLed::fade ( Red , Green , Blue, Time in seconds ):
 *  fade animation:  Fade In over T(seconds)
 *      Fade Out over T(seconds)
 */
void CoolBoardLed::fade(int R, int G, int B, float T) {
  DEBUG_LOG("Entering CoolBoardLed.fade()");
  DEBUG_VAR("Red value:", R);
  DEBUG_VAR("Green value:", G);
  DEBUG_VAR("Blue value:", B);
  DEBUG_VAR("Duration", T);

  if (this->ledActive == 1) {
    for (int k = 0; k < 1000; k++) {
      neoPixelLed->SetPixelColor(
          0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
      neoPixelLed->Show();
      delay(T);
    }

    for (int k = 1000; k >= 0; k--) {
      neoPixelLed->SetPixelColor(
          0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
      neoPixelLed->Show();
      delay(T);
    }
  }
}

/**
*  CoolBoardLed::blink( Red , Green , Blue , Time in seconds ):
*  Blink animation:  Led On for T seconds
                                Led off
*/
void CoolBoardLed::blink(int R, int G, int B, float T) {
  DEBUG_LOG("Entering CoolBoardLed.blink()");
  DEBUG_VAR("Red value:", R);
  DEBUG_VAR("Green value:", G);
  DEBUG_VAR("Blue value:", B);
  DEBUG_VAR("Duration", T);

  if (this->ledActive == 1) {
    neoPixelLed->SetPixelColor(0, RgbColor(R, G, B));
    neoPixelLed->Show();
    delay(T * 1000);
    neoPixelLed->SetPixelColor(0, RgbColor(0, 0, 0));
    neoPixelLed->Show();
  }
}

/**
 *  CoolBoardLed::fadeIn(Red , Green , Blue , Time in seconds)
 *  Fade In animation:  gradual increase over T(seconds)
 */
void CoolBoardLed::fadeIn(int R, int G, int B, float T) {
  DEBUG_LOG("Entering CoolBoardLed.fadeIn()");
  DEBUG_VAR("Red value:", R);
  DEBUG_VAR("Green value:", G);
  DEBUG_VAR("Blue value:", B);
  DEBUG_VAR("Duration", T);

  if (this->ledActive == 1) {
    for (int k = 0; k < 1000; k++) {
      neoPixelLed->SetPixelColor(
          0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
      neoPixelLed->Show();
      delay(T);
    }
  }
}

/**
 *  CoolBoardLed::fadeOut( Red , Green , Blue , Time in seconds)
 *  Fade Out animation:  gradual decrease over T(seconds)
 */
void CoolBoardLed::fadeOut(int R, int G, int B, float T) {
  DEBUG_LOG("Entering CoolBoardLed.fadeOut()");
  DEBUG_VAR("Red value:", R);
  DEBUG_VAR("Green value:", G);
  DEBUG_VAR("Blue value:", B);
  DEBUG_VAR("Duration", T);

  if (this->ledActive == 1) {
    for (int k = 1000; k >= 0; k--) {
      neoPixelLed->SetPixelColor(
          0, RgbColor(k * R / 1000, k * G / 1000, k * B / 1000));
      neoPixelLed->Show();
      delay(T);
    }
  }
}

/**
 *  CoolBoardLed::strobe(Red , Green , Blue , Time in seconds)
 *  Strobe animation:  blinks over T(seconds)
 */
void CoolBoardLed::strobe(int R, int G, int B, float T) {
  DEBUG_LOG("Entering CoolBoardLed.strobe()");
  DEBUG_VAR("Red value:", R);
  DEBUG_VAR("Green value:", G);
  DEBUG_VAR("Blue value:", B);
  DEBUG_VAR("Duration", T);

  if (this->ledActive == 1) {
    for (int k = 1000; k >= 0; k--) {
      neoPixelLed->SetPixelColor(0, RgbColor(R, G, B));
      neoPixelLed->Show();
      delay(T);
      neoPixelLed->SetPixelColor(0, RgbColor(0, 0, 0));
      neoPixelLed->Show();
      delay(T);
    }
  }
}

/**
 *   CoolBoardLed::end() :
 *  this method is provided to delete the dynamically created neoPixelLed
 */
void CoolBoardLed::end() {
  DEBUG_LOG("Entering CoolBoardLed.end()");
  delete neoPixelLed;
}

/**
 *  CoolBoardLed::begin():
 *  This method is provided to start the Led Object
 *  by setting the correct pin and creating a dynamic
 *  neoPixelBus
 */
void CoolBoardLed::begin() {
  DEBUG_LOG("Entering CoolBoardLed.begin()");
  yield();
  if (this->ledActive == 1) {
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    neoPixelLed = new NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod>(1, 2);
    neoPixelLed->Begin();
    neoPixelLed->Show();
  }
}

/**
 *  CoolBoardLed::write(Red,Green,Blue):
 *  This method is provided to set the
 *  Color of the Led
 */
void CoolBoardLed::write(int R, int G, int B) {
  DEBUG_LOG("Entering CoolBoardLed.write()");
  DEBUG_VAR("Red value:", R);
  DEBUG_VAR("Green value:", G);
  DEBUG_VAR("Blue value:", B);

  if (this->ledActive == 1) {
    neoPixelLed->SetPixelColor(0, RgbColor(R, G, B));
    neoPixelLed->Show();
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
  DEBUG_LOG("Entering CoolBoardLed.config()");

  File coolBoardLedConfig = SPIFFS.open("/coolBoardLedConfig.json", "r");

  if (!coolBoardLedConfig) {
    ERROR_LOG("Failed to read /coolBoardLedConfig.json");
    return (false);
  } else {
    size_t size = coolBoardLedConfig.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    coolBoardLedConfig.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
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
      coolBoardLedConfig.close();
      coolBoardLedConfig = SPIFFS.open("/coolBoardLedConfig.json", "w");
      if (!coolBoardLedConfig) {
        ERROR_LOG("Failed to write to /coolBoardLedConfig.json");
        return (false);
      }
      json.printTo(coolBoardLedConfig);
      coolBoardLedConfig.close();
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
  DEBUG_LOG("Entering CoolBoardLed.printConf()");
  INFO_LOG("LED configuration");
  INFO_VAR("  LED active:", ledActive);
}

/**
 *  CoolBoardLed::activate():
 *  This method is provided to activate the
 *  Led Object without the configuration
 *  file
 */
void CoolBoardLed::activate() { this->ledActive = 1; }
