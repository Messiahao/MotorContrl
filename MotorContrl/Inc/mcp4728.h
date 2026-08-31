#ifndef MOTOR_MCP4728_H
#define MOTOR_MCP4728_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_MCP4728_CHANNEL_A 0U
#define BSP_MCP4728_CHANNEL_B 1U
#define BSP_MCP4728_CHANNEL_C 2U
#define BSP_MCP4728_CHANNEL_D 3U
#define BSP_MCP4728_CODE_OFF 0U
#define BSP_MCP4728_CODE_FULL_SCALE 4095U
#define BSP_MCP4728_NOT_IMPLEMENTED 0U

void BspMcp4728_Init(void);
uint8_t BspMcp4728_Read(uint8_t channel, uint16_t *code);
uint8_t BspMcp4728_Write(uint8_t channel, uint16_t code);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_MCP4728_H */
