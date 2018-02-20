#ifndef _LOG_H_

#define _LOG_H_

#define DEBUG

#  define LOG(level, m) do { \
  Serial.print(F(level)); \
  Serial.print(F(" ")); \
  Serial.println(F(m)); \
} while (0)
#  define VAR(level, m, v) do { \
  Serial.print(F(level)); \
  Serial.print(F(" ")); \
  Serial.print(F(m)); \
  Serial.print(F(" ")); \
  Serial.println(v); \
} while(0)
#  define NBR(level, m, n, f) do { \
  Serial.print(F(level)); \
  Serial.print(F(" ")); \
  Serial.print(F(m)); \
  Serial.print(F(" ")); \
  Serial.println(n, f); \
} while(0)
#  define JSON(level, m, j) do { \
  Serial.print(F(level)); \
  Serial.print(F(" ")); \
  Serial.print(F(m)); \
  j.printTo(Serial); \
  Serial.println(); \
} while(0)

#  define INFO_LOG(m)       LOG("INFO: ", m)
#  define INFO_VAR(m, v)    VAR("INFO: ", m, v)
#  define INFO_NBR(m, v, f) NBR("INFO: ", m, v, f)
#  define INFO_JSON(m, j)   LOG("INFO: ", m, j)

#  define WARN_LOG(m)       LOG("WARN: ", m)
#  define WARN_VAR(m, v)    VAR("WARN: ", m, v)
#  define WARN_NBR(m, v, f) NBR("INFO: ", m, v, f)
#  define WARN_JSON(m, j)   LOG("WARN: ", m, j)

#  define ERROR_LOG(m)        LOG("ERROR:", m)
#  define ERROR_VAR(m, v)     VAR("ERROR:", m, v)
#  define ERROR_NBR(m, v, f)  NBR("INFO: ", m, v, f)
#  define ERROR_JSON(m, j)    LOG("ERROR:", m, j)

#  ifdef DEBUG
#    define DEBUG_LOG(m)        LOG("DEBUG:", m)
#    define DEBUG_VAR(m, v)     VAR("DEBUG:", m, v)
#    define DEBUG_NBR(m, v, f)  NBR("INFO: ", m, v, f)
#    define DEBUG_JSON(m, j)    JSON("DEBUG:", m, j)
#  else
#    define DEBUG_LOG(m) do {}  while (0)
#    define DEBUG_VAR(m, v) do {}  while (0)
#    define DEBUG_NBR(m, v, f) do {} while (0)
#    define DEBUG_JSON(m, j) do {} while (0)
#  endif

#endif