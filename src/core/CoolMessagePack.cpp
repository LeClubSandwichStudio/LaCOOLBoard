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

#include "CoolMessagePack.h"

int8_t CoolMessagePack::isNull(JsonObject &json, const char *key) {
  String data;
  String realKey = "\"" + String(key) + "\"";
  char *str;

  json.printTo(data);
  if (!(str = strstr(data.c_str(), key))) {
    return (-1);
  }
  int i = strlen(key) + 1;
  if (str[i] == ':') {
    i++;
  }
  if (str[i] == ' ') {
    i++;
  }
  if (str[i] == 'n') {
    return (1);
  } else {
    return (0);
  }
}

int8_t CoolMessagePack::isNullA(JsonArray &json, uint8_t index) {
  String data;
  uint8_t i = 0;
  json.printTo(data);
  const char *str = data.c_str();
  while (i != index) {
    *str++;
    while (*str && *str != ',') {
      *str++;
    }
    i++;
  }
  while (*str && (*str == ',' || *str == ' ' || *str == '[')) {
    *str++;
  }
  if (*str) {
    if (*str == '0' || *str == 'f') {
      return (0);
    } else {
      return (1);
    }
  }
  return (-1);
}

uint32_t CoolMessagePack::sizeJsonToMsgpck(JsonObject &json) {
  uint32_t size = json.size() + 1;
  if (!json.success()) {
    return (0);
  }
  DEBUG_JSON("json:", json);
  for (auto kv : json) {
    size += strlen(kv.key);
    if (!json[kv.key].is<float>() && json[kv.key] == NULL && CoolMessagePack::isNull(json, kv.key)) {
      size += 1;
    } else if (json[kv.key].is<bool>()) {
      size += 1;
    } else if (json[kv.key].is<char *>()) {
      size += strlen(json[kv.key]) + 1;
    } else if (json[kv.key].is<JsonArray>()) {
      size += CoolMessagePack::sizeArrayToMsgpck(json[kv.key]);
    } else if (json[kv.key].is<JsonObject>()) {
      size += CoolMessagePack::sizeJsonToMsgpck(json[kv.key]);
    } else if (!json[kv.key].is<signed int>()) {
      size += 5;
    } else if (json[kv.key] < 128 && json[kv.key] >= -32) {
      size += 1;
    } else if (json[kv.key] <= 255 && json[kv.key] >= -127) {
      size += 2;
    } else if (json[kv.key] <= 65535 && json[kv.key] >= -32767) {
      size += 3;
    } else {
      size += 4;
    }
  }
  return (size);
}

uint32_t CoolMessagePack::sizeArrayToMsgpck(JsonArray &json) {
  uint32_t size = 1;

  uint8_t i = 0;
  for (auto kv : json) {
    if (!kv.is<float>() && kv == NULL && CoolMessagePack::isNullA(json, i)) {
      size += 1;
    }else if (kv.is<bool>()) {
      size += 1;
    } else if (kv.is<char *>()) {
      size += strlen(kv) + 1;
    } else if (kv.is<JsonArray>()) {
      size += CoolMessagePack::sizeArrayToMsgpck(kv);
    } else if (kv.is<JsonObject>()) {
      size += CoolMessagePack::sizeJsonToMsgpck(kv);
    } else if (!kv.is<signed int>()) {
      size += 5;
    } else if (kv < 128 && kv >= -32) {
      size += 1;
    } else if (kv <= 255 && kv >= -127) {
      size += 2;
    } else if (kv <= 65535 && kv >= -32767) {
      size += 3;
    } else {
      size += 4;
    }
    i++;
  }
  return (size);
}

bool CoolMessagePack::jsonToMsgpck(PrintAdapter streamer, JsonObject &json) {
  if (!json.success()) {
    return (false);
  }
  msgpck_write_map_header(&streamer, json.size());
  for (auto kv : json) {
    CoolMessagePack::is(streamer, json, kv.key);
  }
  return (true);
}

void CoolMessagePack::is(PrintAdapter streamer, JsonObject &json,
                         const char *key) {
  if (!json[key].is<float>() && json[key] == NULL && CoolMessagePack::isNull(json, key)) {
    CoolMessagePack::msgpckNil(streamer, String(key));
  } else if (json[key].is<bool>()) {
    CoolMessagePack::msgpckBool(streamer, json[key], String(key));
  } else if (json[key].is<char *>()) {
    CoolMessagePack::msgpckString(streamer, String(json.get<char *>(key)),
                                  String(key));
  } else if (json[key].is<JsonArray>()) {
    CoolMessagePack::jsonArrayToMP(streamer, json[key], String(key));
  } else if (json[key].is<JsonObject>()) {
    CoolMessagePack::jsonObjectToMP(streamer, json[key], String(key));
  } else if (!json[key].is<signed int>()) {
    CoolMessagePack::msgpckFloat(streamer, json[key], String(key));
  } else if (json[key].is<signed short>() && json[key] < 255 && json[key] > -256) {
    CoolMessagePack::msgpckInt(streamer, json.get<int8_t>(key), String(key));
  } else if (json[key].is<signed int>() && json[key] < 32767 && json[key] > -32768) {
    CoolMessagePack::msgpckInt(streamer, json.get<int16_t>(key), String(key));
  } else {
    CoolMessagePack::msgpckInt(streamer, json.get<int32_t>(key), String(key));
  }
}

void CoolMessagePack::jsonObjectToMP(PrintAdapter streamer, JsonObject &json,
                                     String key) {
  CoolMessagePack::msgpckMap(streamer, json.size(), key);
  for (auto kv : json) {
    CoolMessagePack::is(streamer, json, kv.key);
  }
}

void CoolMessagePack::jsonArrayToMP(PrintAdapter streamer, JsonArray &json,
                                    String key) {
  CoolMessagePack::msgpckArray(streamer, (uint32_t)json.size(), key);
  uint8_t i = 0;
  for (auto kv : json) {
    if (!kv.is<float>() && kv == NULL && CoolMessagePack::isNullA(json, i)) {
      CoolMessagePack::msgpckNil(streamer);
    } else if (kv.is<bool>()) {
      CoolMessagePack::msgpckBool(streamer, kv);
    } else if (kv.is<const char *>()) {
      CoolMessagePack::msgpckString(streamer,
                                    String(json.get<const char *>(i)));
    } else if (kv.is<char *>()) {
      CoolMessagePack::msgpckString(streamer, String(json.get<char *>(i)));
    } else if (kv.is<JsonArray>()) {
      CoolMessagePack::jsonArrayToMP(streamer, kv);
    } else if (kv.is<JsonObject>()) {
      CoolMessagePack::jsonObjectToMP(streamer, kv);
    } else if (!kv.is<signed int>()) {
      CoolMessagePack::msgpckFloat(streamer, kv);
    } else if (kv.is<signed short>() && kv < 255 && kv > -256) {
      CoolMessagePack::msgpckInt(streamer, json.get<int8_t>(i));
    } else if (kv.is<signed int>() && kv < 32767 && kv > -32768) {
      CoolMessagePack::msgpckInt(streamer, json.get<int16_t>(i));
    } else {
      CoolMessagePack::msgpckInt(streamer, json.get<int32_t>(i));
    }
    i++;
  }
}

void CoolMessagePack::msgpckMap(PrintAdapter streamer, uint32_t data,
                                String str) {
  if (str != "") {
    msgpck_write_string(&streamer, str);
  }
  msgpck_write_map_header(&streamer, data);
}

void CoolMessagePack::msgpckArray(PrintAdapter streamer, uint32_t data,
                                  String str) {
  if (str != "") {
    msgpck_write_string(&streamer, str);
  }
  msgpck_write_array_header(&streamer, data);
}

void CoolMessagePack::msgpckString(PrintAdapter streamer, String data,
                                   String str) {
  if (str != "") {
    msgpck_write_string(&streamer, str);
  }
  msgpck_write_string(&streamer, data);
}

void CoolMessagePack::msgpckBool(PrintAdapter streamer, bool data, String str) {
  if (str != "") {
    msgpck_write_string(&streamer, str);
  }
  msgpck_write_bool(&streamer, data);
}

void CoolMessagePack::msgpckFloat(PrintAdapter streamer, float data,
                                  String str) {
  if (str != "") {
    msgpck_write_string(&streamer, str);
  }
  msgpck_write_float(&streamer, data);
}

void CoolMessagePack::msgpckNil(PrintAdapter streamer, String str) {
  if (str != "") {
    msgpck_write_string(&streamer, str);
  }
  msgpck_write_nil(&streamer);
}

void CoolMessagePack::msgpckInt(PrintAdapter streamer, int8_t data,
                                String str) {
  if (str != "") {
    msgpck_write_string(&streamer, str);
  }
  msgpck_write_integer(&streamer, data);
}

void CoolMessagePack::msgpckInt(PrintAdapter streamer, int16_t data,
                                String str) {
  if (str != "") {
    msgpck_write_string(&streamer, str);
  }
  msgpck_write_integer(&streamer, data);
}

void CoolMessagePack::msgpckInt(PrintAdapter streamer, int32_t data,
                                String str) {
  if (str != "") {
    msgpck_write_string(&streamer, str);
  }
  msgpck_write_integer(&streamer, data);
}

void CoolMessagePack::msgpckInt(PrintAdapter streamer, uint8_t data,
                                String str) {
  if (str != "") {
    msgpck_write_string(&streamer, str);
  }
  msgpck_write_integer(&streamer, data);
}

void CoolMessagePack::msgpckInt(PrintAdapter streamer, uint16_t data,
                                String str) {
  if (str != "") {
    msgpck_write_string(&streamer, str);
  }
  msgpck_write_integer(&streamer, data);
}

void CoolMessagePack::msgpckInt(PrintAdapter streamer, uint32_t data,
                                String str) {
  if (str != "") {
    msgpck_write_string(&streamer, str);
  }
  msgpck_write_integer(&streamer, data);
}