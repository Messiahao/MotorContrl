#ifndef MOTOR_APP_PROTOCOL_H
#define MOTOR_APP_PROTOCOL_H

#include "app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void AppProtocol_Init(AppProtocolState *protocol);
void AppProtocol_Process(AppProtocolState *protocol,
                         void (*process_frame)(void *, const uint8_t *),
                         void *context);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_APP_PROTOCOL_H */
