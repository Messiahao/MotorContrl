#include "led.h"

void BspLed_Init(void)
{
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

void BspLed_Write(GPIO_PinState level)
{
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, level);
}
