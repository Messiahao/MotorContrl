#include "app_limit.h"
#include "gpio.h"

void AppLimit_Init(AppLimitState *limits)
{
  limits->limit_gpio_poll_tick = HAL_GetTick();
}

void AppLimit_Process(AppLimitState *limits)
{
if ((HAL_GetTick() - limits->limit_gpio_poll_tick) >=
    LIMIT_GPIO_POLL_PERIOD_MS)
{
  limits->limit_gpio_poll_tick = HAL_GetTick();
  limits->limit_pc6_level = (uint8_t)BspGpio_Read(X_LIM_L_GPIO_Port,
                                               X_LIM_L_Pin);
  limits->limit_pb15_level = (uint8_t)BspGpio_Read(X_LIM_H_GPIO_Port,
                                                X_LIM_H_Pin);
  limits->limit_pb14_level = (uint8_t)BspGpio_Read(X_LIM_R_GPIO_Port,
                                                X_LIM_R_Pin);
  limits->limit_pc9_level = (uint8_t)BspGpio_Read(Y_LIM_L_GPIO_Port,
                                               Y_LIM_L_Pin);
  limits->limit_pc8_level = (uint8_t)BspGpio_Read(Y_LIM_H_GPIO_Port,
                                               Y_LIM_H_Pin);
  limits->limit_pc7_level = (uint8_t)BspGpio_Read(Y_LIM_R_GPIO_Port,
                                               Y_LIM_R_Pin);
  limits->limit_pa12_level = (uint8_t)BspGpio_Read(Z_LIM_L_GPIO_Port,
                                                Z_LIM_L_Pin);
  limits->limit_pa11_level = (uint8_t)BspGpio_Read(Z_LIM_H_GPIO_Port,
                                                Z_LIM_H_Pin);
  limits->limit_pa10_level = (uint8_t)BspGpio_Read(Z_LIM_R_GPIO_Port,
                                                Z_LIM_R_Pin);
  limits->limit_active_mask = 0U;
  if (limits->limit_pc6_level == GPIO_PIN_RESET) limits->limit_active_mask |= (1U << BSP_GPIO_X_LIMIT_L_BIT);
  if (limits->limit_pb15_level == GPIO_PIN_RESET) limits->limit_active_mask |= (1U << BSP_GPIO_X_LIMIT_H_BIT);
  if (limits->limit_pb14_level == GPIO_PIN_RESET) limits->limit_active_mask |= (1U << BSP_GPIO_X_LIMIT_R_BIT);
  if (limits->limit_pc9_level == GPIO_PIN_RESET) limits->limit_active_mask |= (1U << BSP_GPIO_Y_LIMIT_L_BIT);
  if (limits->limit_pc8_level == GPIO_PIN_RESET) limits->limit_active_mask |= (1U << BSP_GPIO_Y_LIMIT_H_BIT);
  if (limits->limit_pc7_level == GPIO_PIN_RESET) limits->limit_active_mask |= (1U << BSP_GPIO_Y_LIMIT_R_BIT);
  if (limits->limit_pa12_level == GPIO_PIN_RESET) limits->limit_active_mask |= (1U << BSP_GPIO_Z_LIMIT_L_BIT);
  if (limits->limit_pa11_level == GPIO_PIN_RESET) limits->limit_active_mask |= (1U << BSP_GPIO_Z_LIMIT_H_BIT);
  if (limits->limit_pa10_level == GPIO_PIN_RESET) limits->limit_active_mask |= (1U << BSP_GPIO_Z_LIMIT_R_BIT);
  limits->limit_gpio_sample_valid = 1U;
}
}
