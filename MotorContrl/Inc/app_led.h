#ifndef MOTOR_APP_LED_H
#define MOTOR_APP_LED_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LED_BLINK_PERIOD_MS 500U

void AppLed_Init(void);
void AppLed_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_APP_LED_H */
