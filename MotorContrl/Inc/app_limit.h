#ifndef MOTOR_APP_LIMIT_H
#define MOTOR_APP_LIMIT_H

#include "app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void AppLimit_Init(AppLimitState *limits);
void AppLimit_Process(AppLimitState *limits);
uint8_t AppLimit_OnExti(uint16_t gpio_pin, uint8_t active_axis);
uint16_t AppLimit_ConsumeInterruptMask(void);

extern volatile uint16_t g_limit_irq_active_mask;
extern volatile uint16_t g_limit_irq_event_mask;

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_APP_LIMIT_H */
