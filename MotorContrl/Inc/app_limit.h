#ifndef MOTOR_APP_LIMIT_H
#define MOTOR_APP_LIMIT_H

#include "app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void AppLimit_Init(AppLimitState *limits);
void AppLimit_Process(AppLimitState *limits);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_APP_LIMIT_H */
