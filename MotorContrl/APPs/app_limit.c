#include "app_limit.h"
#include "gpio.h"
#include "tim.h"

volatile uint16_t g_limit_irq_active_mask;
volatile uint16_t g_limit_irq_event_mask;

void AppLimit_Init(AppLimitState *limits)
{
  g_limit_irq_active_mask = BspGpio_ReadLimitActiveMask();
  g_limit_irq_event_mask = 0U;
  limits->limit_gpio_sample_valid = 0U;
  AppLimit_Process(limits);
  BspGpio_EnableLimitInterrupts();
}

void AppLimit_Process(AppLimitState *limits)
{
  uint16_t active_mask;

  active_mask = g_limit_irq_active_mask;
  if ((limits->limit_gpio_sample_valid != 0U) &&
      (limits->limit_active_mask == active_mask))
  {
    return;
  }
  limits->limit_pc6_level =
      ((active_mask & (1U << BSP_GPIO_X_LIMIT_L_BIT)) != 0U) ? 1U : 0U;
  limits->limit_pb15_level =
      ((active_mask & (1U << BSP_GPIO_X_LIMIT_H_BIT)) != 0U) ? 1U : 0U;
  limits->limit_pb14_level =
      ((active_mask & (1U << BSP_GPIO_X_LIMIT_R_BIT)) != 0U) ? 1U : 0U;
  limits->limit_pc9_level =
      ((active_mask & (1U << BSP_GPIO_Y_LIMIT_L_BIT)) != 0U) ? 1U : 0U;
  limits->limit_pc8_level =
      ((active_mask & (1U << BSP_GPIO_Y_LIMIT_H_BIT)) != 0U) ? 1U : 0U;
  limits->limit_pc7_level =
      ((active_mask & (1U << BSP_GPIO_Y_LIMIT_R_BIT)) != 0U) ? 1U : 0U;
  limits->limit_pa12_level =
      ((active_mask & (1U << BSP_GPIO_Z_LIMIT_L_BIT)) != 0U) ? 1U : 0U;
  limits->limit_pa11_level =
      ((active_mask & (1U << BSP_GPIO_Z_LIMIT_H_BIT)) != 0U) ? 1U : 0U;
  limits->limit_pa10_level =
      ((active_mask & (1U << BSP_GPIO_Z_LIMIT_R_BIT)) != 0U) ? 1U : 0U;
  limits->limit_active_mask = active_mask;
  limits->limit_gpio_sample_valid = 1U;
}

uint8_t AppLimit_OnExti(uint16_t gpio_pin, uint8_t active_axis)
{
  uint16_t limit_bit;
  uint16_t axis_limit_mask;

  limit_bit = BspGpio_LimitBitFromPin(gpio_pin);
  axis_limit_mask = BspGpio_LimitMaskForAxis(active_axis);
  if (limit_bit == 0U)
  {
    return 0U;
  }
  if (BspGpio_ReadLimitPin(gpio_pin) == LIMIT_GPIO_ACTIVE_LEVEL)
  {
    g_limit_irq_active_mask |= limit_bit;
    g_limit_irq_event_mask |= limit_bit;
    if ((limit_bit & axis_limit_mask) != 0U)
    {
      BspTim_WriteAxisEmergencyStop(active_axis);
      /* Keep ENN low so the stopped motor retains holding torque. */
      return 1U;
    }
    return 0U;
  }
  g_limit_irq_active_mask &= (uint16_t)~limit_bit;
  return 0U;
}

uint16_t AppLimit_ConsumeInterruptMask(void)
{
  uint16_t event_mask;
  uint32_t primask;

  primask = __get_PRIMASK();
  __disable_irq();
  event_mask = g_limit_irq_event_mask;
  g_limit_irq_event_mask = 0U;
  if (primask == 0U)
  {
    __enable_irq();
  }
  return event_mask;
}
