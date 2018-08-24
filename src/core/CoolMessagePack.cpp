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
  if (json[key].is<bool>()) {
    CoolMessagePack::msgpckBool(streamer, json[key], String(key));
  } else if (json[key].is<const char *>()) {
      CoolMessagePack::msgpckString(streamer, String(json.get<const char *>(key)),
                                  String(key));
  } else if (json[key].is<char *>()) {
    CoolMessagePack::msgpckString(streamer, String(json.get<char *>(key)),
                                  String(key));
  } else if (json[key].is<double>() || json[key].is<float>()) {
    CoolMessagePack::msgpckFloat(streamer, json[key], String(key));
  } else if (json[key].is<signed int>()) {
    CoolMessagePack::msgpckInt(streamer, json.get<int16_t>(key), String(key));
  } else if (json[key].is<signed long>()) {
    CoolMessagePack::msgpckInt(streamer, json.get<int32_t>(key), String(key));
  } else if (json[key].is<signed short>()) {
    CoolMessagePack::msgpckInt(streamer, json.get<int8_t>(key), String(key));
  } else if (json[key].is<unsigned int>()) {
    CoolMessagePack::msgpckInt(streamer, json.get<uint16_t>(key), String(key));
  } else if (json[key].is<unsigned long>()) {
    CoolMessagePack::msgpckInt(streamer, json.get<uint32_t>(key), String(key));
  } else if (json[key].is<unsigned short>()) {
    CoolMessagePack::msgpckInt(streamer, json.get<uint8_t>(key), String(key));
  } else if (json[key].is<JsonArray>()) {
    CoolMessagePack::jsonArrayToMP(streamer, json[key], String(key));
  } else if (json[key].is<JsonObject>()) {
    CoolMessagePack::jsonObjectToMP(streamer, json[key], String(key));
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
  int i = 0;
  for (auto kv : json) {
    if (kv.is<bool>()) {
      CoolMessagePack::msgpckBool(streamer, kv);
    } else if (kv.is<const char *>()) {
      CoolMessagePack::msgpckString(streamer, String(json.get<const char *>(i)));
    } else if (kv.is<char *>()) {
      CoolMessagePack::msgpckString(streamer, String(json.get<char *>(i)));
    } else if (kv.is<double>() || kv.is<float>()) {
      CoolMessagePack::msgpckFloat(streamer, kv);
    } else if (kv.is<signed int>()) {
      CoolMessagePack::msgpckInt(streamer, json.get<int16_t>(i));
    } else if (kv.is<signed long>()) {
      CoolMessagePack::msgpckInt(streamer, json.get<int32_t>(i));
    } else if (kv.is<signed short>()) {
      CoolMessagePack::msgpckInt(streamer, json.get<int8_t>(i));
    } else if (kv.is<unsigned int>()) {
      CoolMessagePack::msgpckInt(streamer, json.get<uint16_t>(i));
    } else if (kv.is<unsigned long>()) {
      CoolMessagePack::msgpckInt(streamer, json.get<uint32_t>(i));
    } else if (kv.is<unsigned short>()) {
      CoolMessagePack::msgpckInt(streamer, json.get<uint8_t>(i));
    } else if (kv.is<JsonArray>()) {
      CoolMessagePack::jsonArrayToMP(streamer, kv);
    } else if (kv.is<JsonObject>()) {
      CoolMessagePack::jsonObjectToMP(streamer, kv);
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