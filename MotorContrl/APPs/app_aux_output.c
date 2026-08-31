#include "app_aux_output.h"
#include "gpio.h"
#include "usart.h"

/* GPIO initialization already establishes the original OFF levels. */
void AppAuxOutput_Init(void)
{
}

static uint8_t Serial_Aux_SendResponse(uint8_t command_code, uint8_t action_code)
{
  uint8_t response[SERIAL_TEST_FRAME_SIZE] = {
    SERIAL_RESPONSE_HEAD, SERIAL_RESPONSE_HEAD, command_code, SERIAL_DATA_ZERO,
    action_code, SERIAL_DATA_ZERO, SERIAL_DATA_ZERO, SERIAL_DATA_ZERO,
    SERIAL_DATA_ZERO, SERIAL_DATA_ZERO, SERIAL_DATA_ZERO, SERIAL_DATA_ZERO,
    SERIAL_RESPONSE_TAIL, SERIAL_RESPONSE_TAIL
  };

  return (BspUsart_Write(response,
                            SERIAL_TEST_FRAME_SIZE, SERIAL_TX_TIMEOUT_MS) == HAL_OK);
}

void AppAuxOutput_Process(AppAuxState *aux, AppProtocolState *protocol, const uint8_t *frame)
{
  uint8_t aux_command_code;
  uint8_t aux_action_code;
  uint8_t aux_frame_ok;
  uint8_t aux_data_ok;

  protocol->serial_test_last_frame_ok = 0U;
  aux_command_code = frame[SERIAL_FRAME_COMMAND_INDEX];
  aux_action_code = frame[SERIAL_FRAME_DATA0_INDEX];
  aux_frame_ok = (frame[SERIAL_FRAME_TAIL0_INDEX] == SERIAL_REQUEST_TAIL) &&
                 (frame[SERIAL_FRAME_TAIL1_INDEX] == SERIAL_REQUEST_TAIL);
  aux_data_ok = (frame[SERIAL_FRAME_DATA1_INDEX] == SERIAL_DATA_ZERO) &&
                (frame[SERIAL_FRAME_DATA2_INDEX] == SERIAL_DATA_ZERO) &&
                (frame[SERIAL_FRAME_DATA3_INDEX] == SERIAL_DATA_ZERO) &&
                (frame[SERIAL_FRAME_DATA4_INDEX] == SERIAL_DATA_ZERO) &&
                (frame[SERIAL_FRAME_DATA5_INDEX] == SERIAL_DATA_ZERO) &&
                (frame[SERIAL_FRAME_DATA6_INDEX] == SERIAL_DATA_ZERO) &&
                (frame[SERIAL_FRAME_DATA7_INDEX] == SERIAL_DATA_ZERO);
  if (aux_command_code == SERIAL_COMMAND_RELAY)
  {
    aux->serial_relay_command_count++;
    aux->serial_relay_last_frame_ok = aux_frame_ok && aux_data_ok &&
                                  ((aux_action_code == SERIAL_AUX_ACTION_ON) ||
                                   (aux_action_code == SERIAL_AUX_ACTION_OFF));
    if (aux_frame_ok == 0U)
    {
      aux->serial_relay_error_code = SERIAL_AUX_ERROR_FRAME;
    }
    else if (aux_data_ok == 0U)
    {
      aux->serial_relay_error_code = SERIAL_AUX_ERROR_DATA;
    }
    else if (aux->serial_relay_last_frame_ok == 0U)
    {
      aux->serial_relay_error_code = SERIAL_AUX_ERROR_ACTION;
    }
    else
    {
      aux->serial_relay_error_code = SERIAL_AUX_ERROR_NONE;
    }
    if (aux->serial_relay_last_frame_ok != 0U)
    {
      BspGpio_Write(VOUT_5_GPIO_Port, VOUT_5_Pin,
                        (aux_action_code == SERIAL_AUX_ACTION_ON) ?
                        GPIO_PIN_SET : GPIO_PIN_RESET);
      aux->serial_relay_state = (aux_action_code == SERIAL_AUX_ACTION_ON) ? 1U : 0U;
      aux->serial_relay_last_response_ok =
          Serial_Aux_SendResponse(aux_command_code, aux_action_code);
    }
    else
    {
      aux->serial_relay_last_response_ok = 0U;
      protocol->serial_test_frame_error_count++;
    }
    protocol->serial_test_last_response_ok = aux->serial_relay_last_response_ok;
  }
  else
  {
    aux->serial_brake_command_count++;
    aux->serial_brake_last_frame_ok = aux_frame_ok && aux_data_ok &&
                                 ((aux_action_code == SERIAL_AUX_ACTION_ON) ||
                                  (aux_action_code == SERIAL_AUX_ACTION_OFF));
    if (aux_frame_ok == 0U)
    {
      aux->serial_brake_error_code = SERIAL_AUX_ERROR_FRAME;
    }
    else if (aux_data_ok == 0U)
    {
      aux->serial_brake_error_code = SERIAL_AUX_ERROR_DATA;
    }
    else if (aux->serial_brake_last_frame_ok == 0U)
    {
      aux->serial_brake_error_code = SERIAL_AUX_ERROR_ACTION;
    }
    else
    {
      aux->serial_brake_error_code = SERIAL_AUX_ERROR_NONE;
    }
    if (aux->serial_brake_last_frame_ok != 0U)
    {
      BspGpio_Write(VOUT_24_GPIO_Port, VOUT_24_Pin,
                        (aux_action_code == SERIAL_AUX_ACTION_ON) ?
                        GPIO_PIN_SET : GPIO_PIN_RESET);
      aux->serial_brake_state = (aux_action_code == SERIAL_AUX_ACTION_ON) ? 1U : 0U;
      aux->serial_brake_last_response_ok =
          Serial_Aux_SendResponse(aux_command_code, aux_action_code);
    }
    else
    {
      aux->serial_brake_last_response_ok = 0U;
      protocol->serial_test_frame_error_count++;
    }
    protocol->serial_test_last_response_ok = aux->serial_brake_last_response_ok;
  }
}
