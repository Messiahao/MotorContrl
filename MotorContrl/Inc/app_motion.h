#ifndef MOTOR_APP_MOTION_H
#define MOTOR_APP_MOTION_H

#include "app_types.h"
#include "tmc5160.h"

#ifdef __cplusplus
extern "C" {
#endif

void AppMotion_Init(void);
void AppMotion_ProcessStart(AppMotionState *motion, const AppMotionIrq *irq, AppProtocolState *protocol, const uint8_t *frame);
void AppMotion_ProcessStop(AppMotionState *motion, const AppMotionIrq *irq, AppProtocolState *protocol, const uint8_t *frame);
void AppMotion_ProcessStatus(AppMotionState *motion, const AppMotionIrq *irq, AppProtocolState *protocol, const uint8_t *frame);
void AppMotion_Process(AppMotionState *motion, const AppMotionIrq *irq, AppProtocolState *protocol,
                       AppTmcState *tmc, const TMC5160_HandleTypeDef *dev);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_APP_MOTION_H */
