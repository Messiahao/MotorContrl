#include "mcp4728.h"

void BspMcp4728_Init(void)
{
  /* Placeholder: no I2C, LDAC, RDY or EEPROM access. */
}

uint8_t BspMcp4728_Read(uint8_t channel, uint16_t *code)
{
  (void)channel;
  (void)code; /* Leave the caller's value untouched; do not fabricate a read. */
  return BSP_MCP4728_NOT_IMPLEMENTED;
}

uint8_t BspMcp4728_Write(uint8_t channel, uint16_t code)
{
  (void)channel;
  (void)code;
  return BSP_MCP4728_NOT_IMPLEMENTED;
}
