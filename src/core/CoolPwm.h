

#ifndef COOLPWM_H
#define COOLPWM_H
#include <Adafruit_PWMServoDriver.h>
#include "CoolLog.h"

typedef struct {
  uint16_t intensity;
  bool fade;
} PwmStatus_t;

typedef PwmStatus_t PwmParams_t[16];

void initPwm();
void updatePwm(PwmParams_t params);
#endif