#include "app_protocol.h"
#include "usart.h"

void AppProtocol_Init(AppProtocolState *protocol)
{
  protocol->serial_test_rx_index = 0U;
}

void AppProtocol_Process(AppProtocolState *protocol,
                         void (*process_frame)(void *, const uint8_t *),
                         void *context)
{
  uint8_t byte;
  uint8_t frame_ok;
  static const uint8_t response[SERIAL_TEST_FRAME_SIZE] = {
    SERIAL_RESPONSE_HEAD, SERIAL_RESPONSE_HEAD, SERIAL_COMMAND_TEST, SERIAL_SUBCOMMAND_DEFAULT,
    SERIAL_DATA_ZERO, SERIAL_DATA_ZERO, SERIAL_DATA_ZERO, SERIAL_DATA_ZERO,
    SERIAL_DATA_ZERO, SERIAL_DATA_ZERO, SERIAL_DATA_ZERO, SERIAL_DATA_ZERO,
    SERIAL_RESPONSE_TAIL, SERIAL_RESPONSE_TAIL
  };
  if (BspUsart_ReadOverrun() != 0U)
  {
    BspUsart_WriteClearOverrun();
  }

  while (BspUsart_ReadAvailable() != 0U)
  {
    byte = BspUsart_ReadByte();

    if ((protocol->serial_test_rx_index == 0U) && (byte != SERIAL_REQUEST_HEAD))
    {
      continue;
    }
    if ((protocol->serial_test_rx_index == 1U) && (byte != SERIAL_REQUEST_HEAD))
    {
      protocol->serial_test_rx_index = (byte == SERIAL_REQUEST_HEAD) ? 1U : 0U;
      continue;
    }

    protocol->serial_test_rx_frame[protocol->serial_test_rx_index++] = byte;
    if (protocol->serial_test_rx_index < SERIAL_TEST_FRAME_SIZE)
    {
      continue;
    }

    protocol->serial_test_rx_index = 0U;
    frame_ok =
        (protocol->serial_test_rx_frame[SERIAL_FRAME_HEAD0_INDEX] == SERIAL_REQUEST_HEAD) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_HEAD1_INDEX] == SERIAL_REQUEST_HEAD) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_COMMAND_INDEX] == SERIAL_COMMAND_TEST) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_SUBCOMMAND_INDEX] == SERIAL_DATA_ZERO) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_DATA0_INDEX] == SERIAL_DATA_ZERO) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_DATA1_INDEX] == SERIAL_DATA_ZERO) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_DATA2_INDEX] == SERIAL_DATA_ZERO) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_DATA3_INDEX] == SERIAL_DATA_ZERO) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_DATA4_INDEX] == SERIAL_DATA_ZERO) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_DATA5_INDEX] == SERIAL_DATA_ZERO) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_DATA6_INDEX] == SERIAL_DATA_ZERO) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_DATA7_INDEX] == SERIAL_DATA_ZERO) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_TAIL0_INDEX] == SERIAL_REQUEST_TAIL) &&
        (protocol->serial_test_rx_frame[SERIAL_FRAME_TAIL1_INDEX] == SERIAL_REQUEST_TAIL);

    if (frame_ok != 0U)
    {
      (void)BspUsart_Write((uint8_t *)response,
                           SERIAL_TEST_FRAME_SIZE, SERIAL_TX_TIMEOUT_MS);
    }
    else
    {
      process_frame(context, protocol->serial_test_rx_frame);
    }
  }
}
