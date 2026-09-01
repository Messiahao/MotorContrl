#include "app_limit.h"
#include "gpio.h"
#include "tim.h"

volatile AppLimitState g_limit_debug;
volatile uint32_t g_limit_debug_sample_count;
volatile uint32_t g_limit_debug_gpio_c_idr;
volatile uint8_t g_limit_debug_pc6_direct;
volatile uint16_t g_limit_irq_active_mask;
volatile uint16_t g_limit_irq_event_mask;
volatile uint32_t g_limit_irq_event_count;

void AppLimit_Init(AppLimitState *limits)
{
  g_limit_irq_active_mask = BspGpio_ReadLimitActiveMask();
  g_limit_irq_event_mask = 0U;
  g_limit_irq_event_count = 0U;
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
  g_limit_debug_gpio_c_idr = X_LIM_L_GPIO_Port->IDR;
  g_limit_debug_pc6_direct =
      ((g_limit_debug_gpio_c_idr & X_LIM_L_Pin) != 0U) ? 1U : 0U;
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
  g_limit_debug_sample_count++;
  g_limit_debug = *limits;
}

uint8_t AppLimit_OnExti(uint16_t gpio_pin)
{
  uint16_t limit_bit;

  limit_bit = BspGpio_LimitBitFromPin(gpio_pin);
  if (limit_bit == 0U)
  {
    return 0U;
  }
  if (BspGpio_ReadLimitPin(gpio_pin) == LIMIT_GPIO_ACTIVE_LEVEL)
  {
    g_limit_irq_active_mask |= limit_bit;
    g_limit_irq_event_mask |= limit_bit;
    g_limit_irq_event_count++;
    if ((limit_bit & X_LIMIT_ACTIVE_MASK) != 0U)
    {
      BspTim_WriteXEmergencyStop();
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
