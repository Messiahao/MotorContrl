#ifndef MOTOR_APP_AUX_OUTPUT_H
#define MOTOR_APP_AUX_OUTPUT_H

#include "app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void AppAuxOutput_Init(void);
void AppAuxOutput_Process(AppAuxState *aux, AppProtocolState *protocol, const uint8_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_APP_AUX_OUTPUT_H */
