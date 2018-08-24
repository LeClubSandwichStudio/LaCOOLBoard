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

#ifndef COOLMESSAGEPACK_H
#define COOLMESSAGEPACK_H

#include "msgpck.h"
#include "PrintEx.h"
#include "ArduinoJson.h"

class CoolMessagePack {

public:
  static void msgpckMap(PrintAdapter streamer, uint32_t data, String str = "");
  static void msgpckArray(PrintAdapter streamer, uint32_t data, String str = "");
  static void msgpckString(PrintAdapter streamer, String data, String str = "");
  static void msgpckBool(PrintAdapter streamer, bool data, String str = "");
  static void msgpckFloat(PrintAdapter streamer, float data, String str = "");
  static void msgpckNil(PrintAdapter streamer, String str = "");
  static void msgpckInt(PrintAdapter streamer, int8_t data, String str = "");
  static void msgpckInt(PrintAdapter streamer, int16_t data, String str = "");
  static void msgpckInt(PrintAdapter streamer, int32_t data, String str = "");
  static void msgpckInt(PrintAdapter streamer, uint8_t data, String str = "");
  static void msgpckInt(PrintAdapter streamer, uint16_t data, String str = "");
  static void msgpckInt(PrintAdapter streamer, uint32_t data, String str = "");

  static bool jsonToMsgpck(PrintAdapter streamer, JsonObject &json);
  static void is(PrintAdapter streamer, JsonObject &json, const char *key);
  static void jsonArrayToMP(PrintAdapter streamer, JsonArray &json, String key = "");
  static void jsonObjectToMP(PrintAdapter streamer, JsonObject &json, String key = "");
};

#endif
