#ifndef MOTOR_APP_SCHEDULER_H
#define MOTOR_APP_SCHEDULER_H

#include "app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void AppScheduler_Init(const AppMotionIrq *irq);
void AppScheduler_Process(void);
void AppScheduler_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_APP_SCHEDULER_H */
