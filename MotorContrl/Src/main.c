/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "config.h"
#include "system_clock.h"
#include "app_scheduler.h"
#include "app_limit.h"

/* ISR-owned variables remain in this translation unit with their original types.
   Foreground modules receive their addresses through AppMotionIrq. */
static volatile uint8_t serial_motion_active;
static volatile uint8_t serial_motion_done;
static volatile uint8_t serial_motion_axis;
static volatile uint8_t serial_motion_continuous;
static volatile uint32_t serial_motion_pulses_done;
static volatile uint32_t serial_motion_target_steps;
static volatile uint16_t serial_motion_target_speed_hz;
static volatile uint16_t serial_motion_current_speed_hz;
static volatile uint16_t serial_motion_peak_speed_hz;
static volatile uint8_t serial_motion_profile_phase;
static volatile uint32_t serial_motion_profile_accel_steps;
static volatile uint32_t serial_motion_profile_cruise_steps;
static volatile uint32_t serial_motion_profile_decel_steps;
static volatile uint32_t serial_motion_profile_update_count;
static volatile uint32_t serial_motion_last_period_ticks;
static volatile uint8_t serial_motion_profile_error;

/* Frozen ISR call chain: keep bodies and location in main.c unchanged. */
static uint32_t Serial_Motion_IntegerSqrt(uint32_t value)
{
  uint32_t result = 0U;
  uint32_t bit = 1UL << 30;

  while (bit > value)
  {
    bit >>= 2;
  }
  while (bit != 0U)
  {
    if (value >= (result + bit))
    {
      value -= result + bit;
      result = (result >> 1) + bit;
    }
    else
    {
      result >>= 1;
    }
    bit >>= 2;
  }

  return result;
}

static uint16_t Serial_Motion_ProfileSpeed(uint32_t pulse_index)
{
  uint32_t start_speed_sq;
  uint32_t target_speed_sq;
  uint32_t accel_limit_sq;
  uint32_t speed_sq;
  uint32_t decel_limit_sq;
  uint32_t remaining_after_pulse;
  uint16_t speed_hz;

  start_speed_sq = SERIAL_MOTION_MIN_SPEED_HZ *
                   SERIAL_MOTION_MIN_SPEED_HZ;
  target_speed_sq = (uint32_t)serial_motion_target_speed_hz *
                    serial_motion_target_speed_hz;
  accel_limit_sq = start_speed_sq +
                   (2UL * SERIAL_MOTION_ACCELERATION_STEPS_S2 *
                    pulse_index);
  speed_sq = (accel_limit_sq < target_speed_sq) ?
             accel_limit_sq : target_speed_sq;

  if (serial_motion_continuous == 0U)
  {
    /* The last pulse is evaluated with zero distance remaining. */
    remaining_after_pulse = serial_motion_target_steps - pulse_index - 1U;
    decel_limit_sq = start_speed_sq +
                     (2UL * SERIAL_MOTION_DECELERATION_STEPS_S2 *
                      remaining_after_pulse);
    if (decel_limit_sq < speed_sq)
    {
      speed_sq = decel_limit_sq;
    }
  }

  speed_hz = (uint16_t)Serial_Motion_IntegerSqrt(speed_sq);
  if (speed_hz < SERIAL_MOTION_MIN_SPEED_HZ)
  {
    speed_hz = SERIAL_MOTION_MIN_SPEED_HZ;
  }
  if (speed_hz > serial_motion_target_speed_hz)
  {
    speed_hz = serial_motion_target_speed_hz;
  }

  if (serial_motion_continuous == 0U)
  {
    if (pulse_index >= serial_motion_target_steps)
    {
      serial_motion_profile_phase = SERIAL_MOTION_PROFILE_DECEL;
    }
    else if (speed_hz == serial_motion_target_speed_hz)
    {
      serial_motion_profile_phase = SERIAL_MOTION_PROFILE_CRUISE;
    }
    else if (remaining_after_pulse < pulse_index)
    {
      serial_motion_profile_phase = SERIAL_MOTION_PROFILE_DECEL;
    }
    else
    {
      serial_motion_profile_phase = SERIAL_MOTION_PROFILE_ACCEL;
    }
  }
  else
  {
    serial_motion_profile_phase =
        (speed_hz == serial_motion_target_speed_hz) ?
        SERIAL_MOTION_PROFILE_CRUISE : SERIAL_MOTION_PROFILE_ACCEL;
  }

  return speed_hz;
}

static uint8_t Serial_Motion_ApplySpeed(uint16_t speed_hz)
{
  uint32_t period_ticks;

  if ((speed_hz < SERIAL_MOTION_MIN_SPEED_HZ) ||
      (speed_hz > SERIAL_MOTION_MAX_SPEED_HZ))
  {
    return 0U;
  }

  period_ticks = SERIAL_MOTION_TIMER_HZ / speed_hz;
  if ((period_ticks < 2U) || (period_ticks > 65535U))
  {
    return 0U;
  }

  if (BspTim_WriteAxisPeriod(serial_motion_axis, period_ticks) == 0U)
  {
    return 0U;
  }
  serial_motion_current_speed_hz = speed_hz;
  serial_motion_last_period_ticks = period_ticks;
  if (speed_hz > serial_motion_peak_speed_hz)
  {
    serial_motion_peak_speed_hz = speed_hz;
  }
  serial_motion_profile_update_count++;
  if (serial_motion_profile_phase == SERIAL_MOTION_PROFILE_ACCEL)
  {
    serial_motion_profile_accel_steps++;
  }
  else if (serial_motion_profile_phase == SERIAL_MOTION_PROFILE_CRUISE)
  {
    serial_motion_profile_cruise_steps++;
  }
  else if (serial_motion_profile_phase == SERIAL_MOTION_PROFILE_DECEL)
  {
    serial_motion_profile_decel_steps++;
  }

  return 1U;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if ((serial_motion_active != 0U) &&
      (BspTim_IsAxisTimer(serial_motion_axis, htim) != 0U))
  {
    serial_motion_pulses_done++;
    if ((serial_motion_continuous == 0U) &&
        (serial_motion_pulses_done >= serial_motion_target_steps))
    {
      (void)BspTim_WriteAxisStop(serial_motion_axis);
      BspTim_WriteAxisDisableUpdate(serial_motion_axis);
      serial_motion_active = 0U;
      serial_motion_done = 1U;
    }
    else if (Serial_Motion_ApplySpeed(
                 Serial_Motion_ProfileSpeed(serial_motion_pulses_done)) == 0U)
    {
      (void)BspTim_WriteAxisStop(serial_motion_axis);
      BspTim_WriteAxisDisableUpdate(serial_motion_axis);
      serial_motion_profile_error = SERIAL_MOTION_ERROR_TIMER;
      serial_motion_active = 0U;
      serial_motion_done = 1U;
    }
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (AppLimit_OnExti(GPIO_Pin, serial_motion_axis) != 0U)
  {
    serial_motion_active = 0U;
    serial_motion_done = 0U;
  }
}

/* Foreground-only bridge; the ISR above does not call through a pointer. */
static uint8_t Main_WriteInitialSpeed(void)
{
  return Serial_Motion_ApplySpeed(Serial_Motion_ProfileSpeed(0U));
}

static const AppMotionIrq motion_irq = {
  &serial_motion_active,
  &serial_motion_done,
  &serial_motion_axis,
  &serial_motion_continuous,
  &serial_motion_pulses_done,
  &serial_motion_target_steps,
  &serial_motion_target_speed_hz,
  &serial_motion_current_speed_hz,
  &serial_motion_peak_speed_hz,
  &serial_motion_profile_phase,
  &serial_motion_profile_accel_steps,
  &serial_motion_profile_cruise_steps,
  &serial_motion_profile_decel_steps,
  &serial_motion_profile_update_count,
  &serial_motion_last_period_ticks,
  &serial_motion_profile_error,
  Main_WriteInitialSpeed
};

int main(void)
{
  /* 1. HAL/SysTick followed by the original system clock setup. */
  HAL_Init();
  SystemClock_Config();

  /* 2. Original-order peripheral and module initialization. */
  AppScheduler_Init(&motion_irq);

  /* 3. Fixed-order foreground polling. */
  AppScheduler_Run();
}
