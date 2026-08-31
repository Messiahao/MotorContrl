#include "app_light.h"

void AppLight_Init(void)
{
  /* Placeholder only. Existing DAC power-on state is not changed. */
}

uint8_t AppLight_Process(uint8_t protocol_channel, uint8_t enabled)
{
  /* TODO: validate command and produce a DAC request after hardware verification.
     Do not register command 0x0500 or acknowledge success before implementation. */
  (void)protocol_channel;
  (void)enabled;
  return APP_LIGHT_NOT_IMPLEMENTED;
}
