
#ifndef CONCURRENT_H
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

void createI2cMutex();
void lockI2c();
void unlockI2c();

#endif