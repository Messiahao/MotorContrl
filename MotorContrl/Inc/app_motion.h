#ifndef MOTOR_APP_MOTION_H
#define MOTOR_APP_MOTION_H

#include "app_types.h"
#include "tmc5160.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  uint8_t protocol_axis;
  TMC5160_HandleTypeDef tmc;
  GPIO_TypeDef *direction_port;
  uint16_t direction_pin;
  GPIO_TypeDef *step_port;
  uint16_t step_pin;
  uint16_t limit_mask;
  uint8_t ihold;
  uint8_t irun;
  uint8_t toff;
} AppMotionAxisConfig;

void AppMotion_Init(const AppMotionAxisConfig *axes, uint8_t axis_count);
void AppMotion_ProcessStart(AppMotionState *motion, const AppMotionIrq *irq,
                            AppProtocolState *protocol, const uint8_t *frame,
                            const AppMotionAxisConfig *axes, uint8_t axis_count);
void AppMotion_ProcessStop(AppMotionState *motion, const AppMotionIrq *irq,
                           AppProtocolState *protocol, const uint8_t *frame,
                           const AppMotionAxisConfig *axes, uint8_t axis_count);
void AppMotion_ProcessStatus(AppMotionState *motion, const AppMotionIrq *irq, AppProtocolState *protocol, const uint8_t *frame);
void AppMotion_Process(AppMotionState *motion, const AppMotionIrq *irq, AppProtocolState *protocol,
                       const AppMotionAxisConfig *axes, uint8_t axis_count,
                       uint16_t limit_event_mask);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_APP_MOTION_H */
