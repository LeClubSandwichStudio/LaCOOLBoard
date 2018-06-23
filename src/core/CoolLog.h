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

#ifndef _COOL_LOG_H_
#define _COOL_LOG_H_

#include "CoolTime.h"

#define COOL_CRITICAL 0
#define COOL_ERROR 1
#define COOL_WARN 2
#define COOL_INFO 3
#define COOL_DEBUG 4
#define COOL_TRACE 5

#define __FILENAME__                                                           \
  (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1     \
                                    : __FILE__)

#ifndef COOL_LEVEL
#define COOL_LEVEL COOL_INFO
#endif

#ifdef COOL_TIMESTAMP_TRACE
#define ADD_TIMESTAMP                                                          \
  do {                                                                         \
    Serial.print(F("["));                                                      \
    Serial.print(CoolTime::getInstance().getIso8601DateTime());                \
    Serial.print(F("]"));                                                      \
  } while (0)
#else
#define ADD_TIMESTAMP                                                          \
  do {                                                                         \
  } while (0)
#endif

#ifdef COOL_HEAP_TRACE
#define ADD_HEAP                                                               \
  do {                                                                         \
    Serial.print(F("[HEAP:"));                                                 \
    Serial.print(ESP.getFreeHeap());                                           \
    Serial.print(F("]"));                                                      \
  } while (0)
#else
#define ADD_HEAP                                                               \
  do {                                                                         \
  } while (0)
#endif

#ifdef COOL_FUNC_TRACE
#define ADD_TRACE                                                              \
  do {                                                                         \
    Serial.print(F("["));                                                      \
    Serial.print(__FILENAME__);                                                \
    Serial.print(F("():"));                                                    \
    Serial.print(__FUNCTION__);                                                \
    Serial.print(F(":"));                                                      \
    Serial.print(__LINE__);                                                    \
    Serial.print(F("]"));                                                      \
  } while (0)
#else
#define ADD_TRACE                                                              \
  do {                                                                         \
  } while (0)
#endif

#define LOG(level, m)                                                          \
  do {                                                                         \
    ADD_TIMESTAMP;                                                             \
    ADD_TRACE;                                                                 \
    ADD_HEAP;                                                                  \
    Serial.print(F(level));                                                    \
    Serial.print(F(" "));                                                      \
    Serial.println(F(m));                                                      \
  } while (0)
#define VAR(level, m, v)                                                       \
  do {                                                                         \
    ADD_TIMESTAMP;                                                             \
    ADD_TRACE;                                                                 \
    ADD_HEAP;                                                                  \
    Serial.print(F(level));                                                    \
    Serial.print(F(" "));                                                      \
    Serial.print(F(m));                                                        \
    Serial.print(F(" "));                                                      \
    Serial.println(v);                                                         \
  } while (0)
#define NBR(level, m, n, f)                                                    \
  do {                                                                         \
    ADD_TIMESTAMP;                                                             \
    ADD_TRACE;                                                                 \
    ADD_HEAP;                                                                  \
    Serial.print(F(level));                                                    \
    Serial.print(F(" "));                                                      \
    Serial.print(F(m));                                                        \
    Serial.print(F(" "));                                                      \
    Serial.println(n, f);                                                      \
  } while (0)
#define JSON(level, m, j)                                                      \
  do {                                                                         \
    ADD_TIMESTAMP;                                                             \
    ADD_TRACE;                                                                 \
    ADD_HEAP;                                                                  \
    Serial.print(F(level));                                                    \
    Serial.print(F(" "));                                                      \
    Serial.print(F(m));                                                        \
    j.printTo(Serial);                                                         \
    Serial.println();                                                          \
  } while (0)

#if COOL_LEVEL >= COOL_CRITICAL
#define CRITICAL_LOG(m) LOG("ERROR:", m)
#define CRITICAL_VAR(m, v) VAR("ERROR:", m, v)
#define CRITICAL_NBR(m, v, f) NBR("ERROR: ", m, v, f)
#define CRITICAL_JSON(m, j) LOG("ERROR:", m, j)
#else
#define CRITICAL_LOG(m)                                                        \
  do {                                                                         \
  } while (0)
#define CRITICAL_VAR(m, v)                                                     \
  do {                                                                         \
  } while (0)
#define CRITICAL_NBR(m, v, f)                                                  \
  do {                                                                         \
  } while (0)
#define CRITICAL_JSON(m, j)                                                    \
  do {                                                                         \
  } while (0)
#endif

#if COOL_LEVEL >= COOL_ERROR
#define ERROR_LOG(m) LOG("ERROR:", m)
#define ERROR_VAR(m, v) VAR("ERROR:", m, v)
#define ERROR_NBR(m, v, f) NBR("ERROR: ", m, v, f)
#define ERROR_JSON(m, j) LOG("ERROR:", m, j)
#else
#define ERROR_LOG(m)                                                           \
  do {                                                                         \
  } while (0)
#define ERROR_VAR(m, v)                                                        \
  do {                                                                         \
  } while (0)
#define ERROR_NBR(m, v, f)                                                     \
  do {                                                                         \
  } while (0)
#define ERROR_JSON(m, j)                                                       \
  do {                                                                         \
  } while (0)
#endif

#if COOL_LEVEL >= COOL_WARN
#define WARN_LOG(m) LOG("WARN: ", m)
#define WARN_VAR(m, v) VAR("WARN: ", m, v)
#define WARN_NBR(m, v, f) NBR("WARN: ", m, v, f)
#define WARN_JSON(m, j) LOG("WARN: ", m, j)
#else
#define WARN_LOG(m)                                                            \
  do {                                                                         \
  } while (0)
#define WARN_VAR(m, v)                                                         \
  do {                                                                         \
  } while (0)
#define WARN_NBR(m, v, f)                                                      \
  do {                                                                         \
  } while (0)
#define WARN_JSON(m, j)                                                        \
  do {                                                                         \
  } while (0)
#endif

#if COOL_LEVEL >= COOL_INFO
#define INFO_LOG(m) LOG("INFO: ", m)
#define INFO_VAR(m, v) VAR("INFO: ", m, v)
#define INFO_NBR(m, v, f) NBR("INFO: ", m, v, f)
#define INFO_JSON(m, j) LOG("INFO: ", m, j)
#else
#define INFO_LOG(m)                                                            \
  do {                                                                         \
  } while (0)
#define INFO_VAR(m, v)                                                         \
  do {                                                                         \
  } while (0)
#define INFO_NBR(m, v, f)                                                      \
  do {                                                                         \
  } while (0)
#define INFO_JSON(m, j)                                                        \
  do {                                                                         \
  } while (0)
#endif

#if COOL_LEVEL >= COOL_DEBUG
#define DEBUG_LOG(m) LOG("DEBUG:", m)
#define DEBUG_VAR(m, v) VAR("DEBUG:", m, v)
#define DEBUG_NBR(m, v, f) NBR("DEBUG: ", m, v, f)
#define DEBUG_JSON(m, j) JSON("DEBUG:", m, j)
#else
#define DEBUG_LOG(m)                                                           \
  do {                                                                         \
  } while (0)
#define DEBUG_VAR(m, v)                                                        \
  do {                                                                         \
  } while (0)
#define DEBUG_NBR(m, v, f)                                                     \
  do {                                                                         \
  } while (0)
#define DEBUG_JSON(m, j)                                                       \
  do {                                                                         \
  } while (0)
#endif

#if COOL_LEVEL >= COOL_TRACE
#define TRACE_LOG(m) LOG("TRACE:", m)
#define TRACE_VAR(m, v) VAR("TRACE:", m, v)
#define TRACE_NBR(m, v, f) NBR("TRACE: ", m, v, f)
#define TRACE_JSON(m, j) JSON("TRACE:", m, j)
#else
#define TRACE_LOG(m)                                                           \
  do {                                                                         \
  } while (0)
#define TRACE_VAR(m, v)                                                        \
  do {                                                                         \
  } while (0)
#define TRACE_NBR(m, v, f)                                                     \
  do {                                                                         \
  } while (0)
#define TRACE_JSON(m, j)                                                       \
  do {                                                                         \
  } while (0)
#endif

#endif