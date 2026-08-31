#ifndef MOTOR_LED_H
#define MOTOR_LED_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void BspLed_Init(void);
void BspLed_Write(GPIO_PinState level);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_LED_H */
