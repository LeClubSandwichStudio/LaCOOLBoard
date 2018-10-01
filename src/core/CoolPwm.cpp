#include "CoolPwm.h"
#include "Concurrent.h"
#include "freertos/queue.h"
#include "freertos/task.h"

QueueHandle_t queue;
uint8_t queueSize = 1;

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(65);

void pwmTask(void *parameter) {
  DEBUG_VAR("Starting PWM task on core:", xPortGetCoreID());
  PwmParams_t element = {
      {4095, true}, {0, false}, {4095, true}, {0, false},
      {4095, true}, {0, false}, {4095, true}, {0, false},
      {4095, true}, {0, false}, {4095, true}, {0, false},
      {4095, true}, {0, false}, {4095, true}, {0, false},
  };

  for (uint8_t j = 0; j < 200; j++) {
    xQueueReceive(queue, &element, 0);
    lockI2c();
    for (uint8_t i = 0; i < 16; i++) {
      uint16_t high = element[i].intensity;
      if (element[i].fade) {
        if (j <= 100) {
          high = (high * j) / 100;
        } else {
          high = (high * (200 - j)) / 100;
        }
      }
      pwm.setPWM(i, 4095 - high, high);
    }
    unlockI2c();
    vTaskDelay((50L * configTICK_RATE_HZ) / 1000L);
  }
}

void initPwm() {
  lockI2c();
  pwm.begin();
  pwm.setPWMFreq(1600);
  unlockI2c();
  DEBUG_VAR("Initializing PWM on core:", xPortGetCoreID());

  queue = xQueueCreate(queueSize, sizeof(PwmParams_t));

  xTaskCreatePinnedToCore(pwmTask, "pwmTask", 10000, NULL, 1, NULL,
                          xPortGetCoreID());
}

void updatePwm(PwmParams_t params) {
  xQueueSend(queue, &params, portMAX_DELAY);
}
