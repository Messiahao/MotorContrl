#include "app_light.h"
#include "mcp4728.h"
#include "usart.h"

void AppLight_Init(void)
{
  /* MCP4728 init keeps the existing power-on DAC values unchanged. */
}

static uint8_t Serial_Light_SendResponse(const uint8_t *frame)
{
  uint8_t response[SERIAL_TEST_FRAME_SIZE] = {
    SERIAL_RESPONSE_HEAD, SERIAL_RESPONSE_HEAD,
    frame[SERIAL_FRAME_COMMAND_INDEX], frame[SERIAL_FRAME_SUBCOMMAND_INDEX],
    frame[SERIAL_FRAME_DATA0_INDEX], frame[SERIAL_FRAME_DATA1_INDEX],
    frame[SERIAL_FRAME_DATA2_INDEX], frame[SERIAL_FRAME_DATA3_INDEX],
    frame[SERIAL_FRAME_DATA4_INDEX], frame[SERIAL_FRAME_DATA5_INDEX],
    frame[SERIAL_FRAME_DATA6_INDEX], frame[SERIAL_FRAME_DATA7_INDEX],
    SERIAL_RESPONSE_TAIL, SERIAL_RESPONSE_TAIL
  };

  return (BspUsart_Write(response, SERIAL_TEST_FRAME_SIZE,
                         SERIAL_TX_TIMEOUT_MS) == HAL_OK);
}

void AppLight_Process(AppProtocolState *protocol, const uint8_t *frame)
{
  uint8_t action;
  uint8_t protocol_channel;
  uint8_t frame_ok;
  uint8_t data_ok;
  uint8_t action_ok;
  uint8_t channel_ok;
  uint16_t code;

  protocol->serial_test_last_frame_ok = 0U;
  action = frame[SERIAL_FRAME_DATA0_INDEX];
  protocol_channel = frame[SERIAL_FRAME_DATA2_INDEX];
  frame_ok = (frame[SERIAL_FRAME_TAIL0_INDEX] == SERIAL_REQUEST_TAIL) &&
             (frame[SERIAL_FRAME_TAIL1_INDEX] == SERIAL_REQUEST_TAIL);
  data_ok = (frame[SERIAL_FRAME_DATA1_INDEX] == SERIAL_DATA_ZERO) &&
            (frame[SERIAL_FRAME_DATA3_INDEX] == SERIAL_DATA_ZERO) &&
            (frame[SERIAL_FRAME_DATA4_INDEX] == SERIAL_DATA_ZERO) &&
            (frame[SERIAL_FRAME_DATA5_INDEX] == SERIAL_DATA_ZERO) &&
            (frame[SERIAL_FRAME_DATA6_INDEX] == SERIAL_DATA_ZERO) &&
            (frame[SERIAL_FRAME_DATA7_INDEX] == SERIAL_DATA_ZERO);
  action_ok = (action == SERIAL_AUX_ACTION_ON) ||
              (action == SERIAL_AUX_ACTION_OFF);
  channel_ok = (protocol_channel >= APP_LIGHT_CHANNEL_1) &&
               (protocol_channel <= APP_LIGHT_CHANNEL_4);

  if ((frame_ok == 0U) || (data_ok == 0U) ||
      (action_ok == 0U) || (channel_ok == 0U))
  {
    protocol->serial_test_frame_error_count++;
    protocol->serial_test_last_response_ok = 0U;
    return;
  }

  code = (action == SERIAL_AUX_ACTION_ON) ?
         BSP_MCP4728_CODE_FULL_SCALE : BSP_MCP4728_CODE_OFF;
  if (BspMcp4728_Write((uint8_t)(protocol_channel - 1U), code) == 0U)
  {
    protocol->serial_test_frame_error_count++;
    protocol->serial_test_last_response_ok = 0U;
    return;
  }

  protocol->serial_test_last_response_ok = Serial_Light_SendResponse(frame);
}
