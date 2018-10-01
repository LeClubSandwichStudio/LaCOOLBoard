#include "Concurrent.h"
#include "CoolLog.h"

SemaphoreHandle_t i2cMutex;

void createI2cMutex() {
  i2cMutex = xSemaphoreCreateMutex();

  if (i2cMutex == NULL) {
    ERROR_LOG("Could not create I2C Mutex");
  } else {
    INFO_LOG("I2C Mutex successfully created!");
  }
}

void lockI2c() {
  xSemaphoreTake(i2cMutex, portMAX_DELAY);
}

void unlockI2c() {
  xSemaphoreGive(i2cMutex);
}
