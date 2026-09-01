#ifndef MOTOR_APP_LIMIT_H
#define MOTOR_APP_LIMIT_H

#include "app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void AppLimit_Init(AppLimitState *limits);
void AppLimit_Process(AppLimitState *limits);
uint8_t AppLimit_OnExti(uint16_t gpio_pin);
uint16_t AppLimit_ConsumeInterruptMask(void);

/* Debug snapshot; refreshed by AppLimit_Process without affecting control flow. */
extern volatile AppLimitState g_limit_debug;
extern volatile uint32_t g_limit_debug_sample_count;
extern volatile uint32_t g_limit_debug_gpio_c_idr;
extern volatile uint8_t g_limit_debug_pc6_direct;
extern volatile uint16_t g_limit_irq_active_mask;
extern volatile uint16_t g_limit_irq_event_mask;
extern volatile uint32_t g_limit_irq_event_count;

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_APP_LIMIT_H */
