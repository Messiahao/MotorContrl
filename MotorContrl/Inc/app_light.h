#ifndef MOTOR_APP_LIGHT_H
#define MOTOR_APP_LIGHT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APP_LIGHT_CHANNEL_1 1U /* MCP4728 A -> CN10-1 */
#define APP_LIGHT_CHANNEL_2 2U /* MCP4728 B -> CN10-2 */
#define APP_LIGHT_CHANNEL_3 3U /* MCP4728 C -> CN10-3 */
#define APP_LIGHT_CHANNEL_4 4U /* MCP4728 D -> CN10-4 */
#define APP_LIGHT_NOT_IMPLEMENTED 0U

void AppLight_Init(void);
uint8_t AppLight_Process(uint8_t protocol_channel, uint8_t enabled);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_APP_LIGHT_H */
