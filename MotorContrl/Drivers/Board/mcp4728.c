#include "mcp4728.h"
#include "i2c.h"

void BspMcp4728_Init(void)
{
  /* GPIO initialization already holds LDAC low for immediate VOUT updates. */
}

uint8_t BspMcp4728_Read(uint8_t channel, uint16_t *code)
{
  (void)channel;
  (void)code; /* Leave the caller's value untouched; do not fabricate a read. */
  return BSP_MCP4728_NOT_IMPLEMENTED;
}

uint8_t BspMcp4728_Write(uint8_t channel, uint16_t code)
{
  uint8_t data[3];

  if ((channel > BSP_MCP4728_CHANNEL_D) ||
      (code > BSP_MCP4728_CODE_FULL_SCALE))
  {
    return 0U;
  }

  /* Multi-Write: command, VREF=VDD/PD=normal/gain=1, code[11:0]. */
  data[0] = (uint8_t)(0x40U | (channel << 1));
  data[1] = (uint8_t)((code >> 8) & 0x0FU);
  data[2] = (uint8_t)code;

  return (BspI2c_Write(BSP_MCP4728_I2C_ADDRESS, data, 3U,
                       BSP_MCP4728_I2C_TIMEOUT_MS) == HAL_OK);
}
