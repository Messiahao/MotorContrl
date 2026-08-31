#ifndef MOTOR_APP_SELF_TEST_H
#define MOTOR_APP_SELF_TEST_H

#include "app_types.h"
#include "tmc5160.h"

#ifdef __cplusplus
extern "C" {
#endif

void AppSelfTest_Init(AppTmcState *tmc);
void AppSelfTest_Process(AppTmcState *tmc, const TMC5160_HandleTypeDef *dev);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_APP_SELF_TEST_H */
