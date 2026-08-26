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
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "tmc5160.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LIMIT_GPIO_STATIC_TEST 0U
#define LIMIT_GPIO_POLL_PERIOD_MS 10U
#define SERIAL_TEST_FRAME_SIZE 14U
#define SERIAL_PROTOCOL_STAGE1_TEST 1U
#define SERIAL_MOTION_MIN_SPEED_HZ 1000U
#define SERIAL_MOTION_MAX_SPEED_HZ 10000U
#define SERIAL_MOTION_MAX_DISTANCE_STEPS 12000U
#define SERIAL_MOTION_MAX_TIME_SECONDS 5U
#define SERIAL_MOTION_ERROR_NONE 0U
#define SERIAL_MOTION_ERROR_FRAME 1U
#define SERIAL_MOTION_ERROR_AXIS 2U
#define SERIAL_MOTION_ERROR_DIRECTION 3U
#define SERIAL_MOTION_ERROR_SPEED 4U
#define SERIAL_MOTION_ERROR_DISTANCE 5U
#define SERIAL_MOTION_ERROR_TIME 6U
#define SERIAL_MOTION_ERROR_BUSY 7U
#define SERIAL_MOTION_ERROR_TMC 8U
#define SERIAL_MOTION_ERROR_TIMER 9U
#define SERIAL_MOTION_STATE_IDLE 0U
#define SERIAL_MOTION_STATE_PENDING 1U
#define SERIAL_MOTION_STATE_ACTIVE 2U
#define SERIAL_MOTION_STATE_DONE 3U
#define SERIAL_MOTION_STATE_ERROR 4U
#define X_MOTION_TEST_DURATION_MS 1200U
#define X_MOTION_TEST_DISABLE_DELAY_MS 6000U
#define X_MOTION_IHOLD 0U
#define X_MOTION_IRUN 3U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static const TMC5160_HandleTypeDef x_tmc5160 = {
  X_CS_GPIO_Port, X_CS_Pin, X_EN_GPIO_Port, X_EN_Pin
};
static uint32_t x_tmc5160_spi_test_tick;
static uint8_t x_tmc5160_spi_test_pending;
volatile uint32_t x_tmc5160_ioin;
volatile uint8_t x_tmc5160_spi_ok;
volatile uint8_t x_tmc5160_spi_test_done;
volatile uint32_t x_tmc5160_gconf;
volatile uint8_t x_tmc5160_gconf_ok;
volatile uint8_t x_tmc5160_gconf_test_done;
volatile uint32_t x_tmc5160_gstat;
volatile uint8_t x_tmc5160_gstat_test_done;
static uint32_t x_tmc5160_gstat_clear_tick;
static uint8_t x_tmc5160_gstat_clear_pending;
volatile uint32_t x_tmc5160_gstat_after_clear;
volatile uint8_t x_tmc5160_gstat_uv_cp_clear_ok;
volatile uint8_t x_tmc5160_gstat_clear_test_done;
volatile uint32_t x_tmc5160_chopconf;
volatile uint8_t x_tmc5160_static_read_test_done;
volatile uint32_t x_tmc5160_chopconf_configured;
volatile uint8_t x_tmc5160_low_current_config_ok;
volatile uint8_t x_tmc5160_low_current_config_test_done;
static uint32_t x_tmc5160_enable_test_tick;
static uint8_t x_tmc5160_enable_test_pending;
volatile uint32_t x_tmc5160_gstat_enabled;
volatile uint32_t x_tmc5160_drv_status_enabled;
volatile uint8_t x_tmc5160_enable_test_ok;
volatile uint8_t x_tmc5160_enable_test_done;
volatile uint8_t x_tmc5160_enable_test_active;
static uint32_t x_tmc5160_step_test_tick;
static uint8_t x_tmc5160_step_test_state;
volatile uint32_t x_tmc5160_mscnt_before_step;
volatile uint32_t x_tmc5160_mscnt_after_step;
volatile uint16_t x_tmc5160_mscnt_step_delta;
volatile uint8_t x_tmc5160_step_test_ok;
volatile uint8_t x_tmc5160_step_test_done;
static uint32_t x_tmc5160_multi_step_test_tick;
static uint8_t x_tmc5160_multi_step_test_state;
static uint8_t x_tmc5160_multi_step_test_count;
volatile uint32_t x_tmc5160_mscnt_before_multi_step;
volatile uint32_t x_tmc5160_mscnt_after_multi_step;
volatile uint16_t x_tmc5160_mscnt_multi_step_delta;
volatile uint8_t x_tmc5160_multi_step_test_ok;
volatile uint8_t x_tmc5160_multi_step_test_done;
static uint32_t x_tmc5160_motion_test_start_tick;
static uint8_t x_tmc5160_motion_test_state;
volatile uint8_t x_tmc5160_motion_test_active;
volatile uint8_t x_tmc5160_motion_test_done;
static uint32_t limit_gpio_poll_tick;
volatile uint8_t limit_gpio_sample_valid;
volatile uint8_t limit_pc6_level;
volatile uint8_t limit_pb15_level;
volatile uint8_t limit_pb14_level;
volatile uint8_t limit_pc9_level;
volatile uint8_t limit_pc8_level;
volatile uint8_t limit_pc7_level;
volatile uint8_t limit_pa12_level;
volatile uint8_t limit_pa11_level;
volatile uint8_t limit_pa10_level;
volatile uint16_t limit_active_mask;
static uint8_t serial_test_rx_frame[SERIAL_TEST_FRAME_SIZE];
static uint8_t serial_test_rx_index;
volatile uint32_t serial_test_command_count;
volatile uint32_t serial_test_frame_error_count;
volatile uint32_t serial_test_rx_byte_count;
volatile uint32_t serial_test_uart_error_count;
volatile uint8_t serial_test_last_rx_byte;
volatile uint8_t serial_test_last_frame_ok;
volatile uint8_t serial_test_last_response_ok;
volatile uint32_t serial_test_build_marker;
volatile uint32_t serial_motion_command_count;
volatile uint8_t serial_motion_last_axis;
volatile uint8_t serial_motion_last_direction;
volatile uint16_t serial_motion_last_speed_hz;
volatile uint32_t serial_motion_last_distance_steps;
volatile uint8_t serial_motion_last_frame_ok;
volatile uint8_t serial_motion_last_response_ok;
volatile uint8_t serial_motion_error_code;
volatile uint8_t serial_motion_busy;
volatile uint8_t serial_motion_command_pending;
volatile uint8_t serial_motion_active;
volatile uint8_t serial_motion_done;
volatile uint8_t serial_motion_state;
volatile uint8_t serial_motion_start_ok;
volatile uint32_t serial_motion_pulses_done;
volatile uint32_t serial_motion_target_steps;
volatile uint8_t serial_motion_target_axis;
volatile uint8_t serial_motion_target_direction;
volatile uint16_t serial_motion_target_speed_hz;
volatile uint32_t serial_motion_target_distance_steps;
volatile uint8_t serial_motion_last_completion_status;
volatile uint8_t serial_motion_last_completion_error_code;
volatile uint8_t serial_motion_last_completion_response_ok;
volatile uint32_t serial_motion_mscnt_before;
volatile uint32_t serial_motion_mscnt_after;
volatile uint16_t serial_motion_mscnt_delta;
volatile uint8_t serial_motion_mscnt_ok;
/*
static uint32_t auxiliary_output_test_tick;
static uint8_t auxiliary_output_test_state;
*/
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void Serial_Test_Task(void);
static void Serial_Motion_Task(void);
static uint8_t Serial_Motion_PrepareAndStart(void);
static uint8_t Serial_Motion_SendResponse(uint8_t status, uint8_t error_code);
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Serial_Test_Task(void)
{
  uint8_t byte;
  uint8_t i;
  uint8_t motion_frame_ok;
  uint8_t motion_error_code;
  uint16_t motion_speed_hz;
  uint32_t motion_distance_steps;
  uint8_t motion_response[SERIAL_TEST_FRAME_SIZE];
  static const uint8_t response[SERIAL_TEST_FRAME_SIZE] = {
    0xAAU, 0xAAU, 0x01U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U,
    0x55U, 0x55U
  };

  if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_ORE) != RESET)
  {
    __HAL_UART_CLEAR_OREFLAG(&huart3);
    serial_test_uart_error_count++;
  }

  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) != RESET)
  {
    byte = (uint8_t)(huart3.Instance->DR & 0x00FFU);
    serial_test_rx_byte_count++;
    serial_test_last_rx_byte = byte;

    if ((serial_test_rx_index == 0U) && (byte != 0x55U))
    {
      continue;
    }
    if ((serial_test_rx_index == 1U) && (byte != 0x55U))
    {
      serial_test_rx_index = (byte == 0x55U) ? 1U : 0U;
      continue;
    }

    serial_test_rx_frame[serial_test_rx_index++] = byte;
    if (serial_test_rx_index < SERIAL_TEST_FRAME_SIZE)
    {
      continue;
    }

    serial_test_rx_index = 0U;
    serial_test_last_frame_ok =
        (serial_test_rx_frame[0] == 0x55U) &&
        (serial_test_rx_frame[1] == 0x55U) &&
        (serial_test_rx_frame[2] == 0x01U) &&
        (serial_test_rx_frame[3] == 0x00U) &&
        (serial_test_rx_frame[4] == 0x00U) &&
        (serial_test_rx_frame[5] == 0x00U) &&
        (serial_test_rx_frame[6] == 0x00U) &&
        (serial_test_rx_frame[7] == 0x00U) &&
        (serial_test_rx_frame[8] == 0x00U) &&
        (serial_test_rx_frame[9] == 0x00U) &&
        (serial_test_rx_frame[10] == 0x00U) &&
        (serial_test_rx_frame[11] == 0x00U) &&
        (serial_test_rx_frame[12] == 0xAAU) &&
        (serial_test_rx_frame[13] == 0xAAU);

    if (serial_test_last_frame_ok != 0U)
    {
      serial_test_command_count++;
      serial_test_last_response_ok =
          (HAL_UART_Transmit(&huart3, (uint8_t *)response,
                             SERIAL_TEST_FRAME_SIZE, 100U) == HAL_OK);
    }
    else if ((serial_test_rx_frame[2] == 0x02U) &&
             (serial_test_rx_frame[3] == 0x00U))
    {
      serial_test_last_frame_ok = 0U;
      serial_motion_command_count++;
      serial_motion_last_axis = serial_test_rx_frame[4];
      serial_motion_last_direction = serial_test_rx_frame[5];
      motion_speed_hz = (uint16_t)(((uint16_t)serial_test_rx_frame[6] << 8) |
                                   serial_test_rx_frame[7]);
      motion_distance_steps = ((uint32_t)serial_test_rx_frame[8] << 24) |
                              ((uint32_t)serial_test_rx_frame[9] << 16) |
                              ((uint32_t)serial_test_rx_frame[10] << 8) |
                              (uint32_t)serial_test_rx_frame[11];
      serial_motion_last_speed_hz = motion_speed_hz;
      serial_motion_last_distance_steps = motion_distance_steps;

      motion_frame_ok =
          (serial_test_rx_frame[12] == 0xAAU) &&
          (serial_test_rx_frame[13] == 0xAAU);
      motion_error_code = SERIAL_MOTION_ERROR_NONE;
      if (motion_frame_ok == 0U)
      {
        motion_error_code = SERIAL_MOTION_ERROR_FRAME;
      }
      else if (serial_motion_last_axis != 0x01U)
      {
        motion_error_code = SERIAL_MOTION_ERROR_AXIS;
      }
      else if (serial_motion_last_direction > 0x01U)
      {
        motion_error_code = SERIAL_MOTION_ERROR_DIRECTION;
      }
      else if ((motion_speed_hz < SERIAL_MOTION_MIN_SPEED_HZ) ||
               (motion_speed_hz > SERIAL_MOTION_MAX_SPEED_HZ))
      {
        motion_error_code = SERIAL_MOTION_ERROR_SPEED;
      }
      else if ((motion_distance_steps == 0U) ||
               (motion_distance_steps > SERIAL_MOTION_MAX_DISTANCE_STEPS))
      {
        motion_error_code = SERIAL_MOTION_ERROR_DISTANCE;
      }
      else if (motion_distance_steps >
               ((uint32_t)motion_speed_hz * SERIAL_MOTION_MAX_TIME_SECONDS))
      {
        motion_error_code = SERIAL_MOTION_ERROR_TIME;
      }
      else if (serial_motion_busy != 0U)
      {
        motion_error_code = SERIAL_MOTION_ERROR_BUSY;
      }

      serial_motion_error_code = motion_error_code;
      serial_motion_last_frame_ok = (motion_error_code ==
                                     SERIAL_MOTION_ERROR_NONE);
      if (serial_motion_last_frame_ok != 0U)
      {
        serial_motion_target_axis = serial_motion_last_axis;
        serial_motion_target_direction = serial_motion_last_direction;
        serial_motion_target_speed_hz = motion_speed_hz;
        serial_motion_target_distance_steps = motion_distance_steps;
        serial_motion_target_steps = motion_distance_steps;
        serial_motion_pulses_done = 0U;
        serial_motion_command_pending = 1U;
        serial_motion_busy = 1U;
        serial_motion_state = SERIAL_MOTION_STATE_PENDING;
      }
      motion_response[0] = 0xAAU;
      motion_response[1] = 0xAAU;
      motion_response[2] = 0x02U;
      motion_response[3] = (serial_motion_last_frame_ok != 0U) ?
                           0x00U : 0xFFU;
      if (serial_motion_last_frame_ok != 0U)
      {
        for (i = 0U; i < 8U; i++)
        {
          motion_response[4U + i] = serial_test_rx_frame[4U + i];
        }
      }
      else
      {
        motion_response[4] = motion_error_code;
        for (i = 5U; i < 12U; i++)
        {
          motion_response[i] = 0U;
        }
        serial_test_frame_error_count++;
      }
      motion_response[12] = 0x55U;
      motion_response[13] = 0x55U;
      serial_motion_last_response_ok =
          (HAL_UART_Transmit(&huart3, motion_response,
                             SERIAL_TEST_FRAME_SIZE, 100U) == HAL_OK);
      serial_test_last_response_ok = serial_motion_last_response_ok;
    }
    else
    {
      serial_test_last_frame_ok = 0U;
      serial_test_frame_error_count++;
      serial_test_last_response_ok = 0U;
    }
  }
}

static uint8_t Serial_Motion_SendResponse(uint8_t status, uint8_t error_code)
{
  uint8_t i;
  uint8_t response[SERIAL_TEST_FRAME_SIZE];

  response[0] = 0xAAU;
  response[1] = 0xAAU;
  response[2] = 0x02U;
  response[3] = status;
  if ((status == 0x00U) || (status == 0x01U))
  {
    response[4] = serial_motion_target_axis;
    response[5] = serial_motion_target_direction;
    response[6] = (uint8_t)(serial_motion_target_speed_hz >> 8);
    response[7] = (uint8_t)serial_motion_target_speed_hz;
    response[8] = (uint8_t)(serial_motion_target_distance_steps >> 24);
    response[9] = (uint8_t)(serial_motion_target_distance_steps >> 16);
    response[10] = (uint8_t)(serial_motion_target_distance_steps >> 8);
    response[11] = (uint8_t)serial_motion_target_distance_steps;
  }
  else
  {
    response[4] = error_code;
    for (i = 5U; i < 12U; i++)
    {
      response[i] = 0U;
    }
  }
  response[12] = 0x55U;
  response[13] = 0x55U;

  return (HAL_UART_Transmit(&huart3, response,
                            SERIAL_TEST_FRAME_SIZE, 100U) == HAL_OK);
}

static uint8_t Serial_Motion_PrepareAndStart(void)
{
  uint32_t expected_chopconf;
  uint32_t period_ticks;
  GPIO_InitTypeDef x_step_gpio = {0};

  TMC5160_DISABLE(&x_tmc5160);
  x_tmc5160_ioin = TMC5160_ReadRegister(&x_tmc5160, TMC5160_IOIN);
  x_tmc5160_spi_ok = ((x_tmc5160_ioin >> 24) == 0x30U);
  x_tmc5160_spi_test_done = 1U;
  if (x_tmc5160_spi_ok == 0U)
  {
    return SERIAL_MOTION_ERROR_TMC;
  }

  TMC5160_WriteRegister(&x_tmc5160, TMC5160_GCONF,
                        TMC5160_GCONF_INIT);
  x_tmc5160_gconf = TMC5160_ReadRegister(&x_tmc5160, TMC5160_GCONF);
  x_tmc5160_gconf_ok = (x_tmc5160_gconf == TMC5160_GCONF_INIT);
  x_tmc5160_gconf_test_done = 1U;
  if (x_tmc5160_gconf_ok == 0U)
  {
    return SERIAL_MOTION_ERROR_TMC;
  }

  x_tmc5160_gstat = TMC5160_ReadRegister(&x_tmc5160, TMC5160_GSTAT);
  x_tmc5160_gstat_test_done = 1U;
  TMC5160_WriteRegister(&x_tmc5160, TMC5160_GSTAT,
                        TMC5160_GSTAT_INIT);
  HAL_Delay(100U);
  x_tmc5160_gstat_after_clear =
      TMC5160_ReadRegister(&x_tmc5160, TMC5160_GSTAT);
  x_tmc5160_gstat_uv_cp_clear_ok =
      ((x_tmc5160_gstat_after_clear & TMC5160_GSTAT_UV_CP) == 0U);
  x_tmc5160_gstat_clear_test_done = 1U;
  if (x_tmc5160_gstat_uv_cp_clear_ok == 0U)
  {
    return SERIAL_MOTION_ERROR_TMC;
  }

  x_tmc5160_chopconf =
      TMC5160_ReadRegister(&x_tmc5160, TMC5160_CHOPCONF);
  x_tmc5160_static_read_test_done = 1U;
  TMC5160_WriteRegister(&x_tmc5160, TMC5160_IHOLD_IRUN,
                        TMC5160_IHOLD(X_MOTION_IHOLD) |
                        TMC5160_IRUN(X_MOTION_IRUN));
  expected_chopconf = (x_tmc5160_chopconf & ~TMC5160_TOFF_MASK) |
                      TMC5160_TOFF(3U);
  TMC5160_WriteRegister(&x_tmc5160, TMC5160_CHOPCONF,
                        expected_chopconf);
  x_tmc5160_chopconf_configured =
      TMC5160_ReadRegister(&x_tmc5160, TMC5160_CHOPCONF);
  x_tmc5160_low_current_config_ok =
      (x_tmc5160_chopconf_configured == expected_chopconf);
  x_tmc5160_low_current_config_test_done = 1U;
  if (x_tmc5160_low_current_config_ok == 0U)
  {
    return SERIAL_MOTION_ERROR_TMC;
  }

  TMC5160_ENABLE(&x_tmc5160);
  x_tmc5160_enable_test_active = 1U;
  HAL_Delay(200U);
  x_tmc5160_gstat_enabled =
      TMC5160_ReadRegister(&x_tmc5160, TMC5160_GSTAT);
  x_tmc5160_drv_status_enabled =
      TMC5160_ReadRegister(&x_tmc5160, TMC5160_DRV_STATUS);
  x_tmc5160_enable_test_ok =
      ((x_tmc5160_gstat_enabled &
        (TMC5160_GSTAT_DRV_ERR | TMC5160_GSTAT_UV_CP)) == 0U);
  x_tmc5160_enable_test_done = 1U;
  if (x_tmc5160_enable_test_ok == 0U)
  {
    TMC5160_DISABLE(&x_tmc5160);
    x_tmc5160_enable_test_active = 0U;
    return SERIAL_MOTION_ERROR_TMC;
  }

  HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin,
                    (serial_motion_target_direction != 0U) ?
                    GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
  HAL_Delay(1U);
  x_step_gpio.Pin = X_STEP_Pin;
  x_step_gpio.Mode = GPIO_MODE_AF_PP;
  x_step_gpio.Pull = GPIO_NOPULL;
  x_step_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(X_STEP_GPIO_Port, &x_step_gpio);
  HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
  serial_motion_mscnt_before =
      TMC5160_ReadRegister(&x_tmc5160, TMC5160_MSCNT);
  serial_motion_mscnt_ok = 0U;
  period_ticks = 1000000U / serial_motion_target_speed_hz;
  if ((period_ticks < 2U) || (period_ticks > 65535U))
  {
    TMC5160_DISABLE(&x_tmc5160);
    x_tmc5160_enable_test_active = 0U;
    return SERIAL_MOTION_ERROR_TIMER;
  }
  __HAL_TIM_SET_AUTORELOAD(&htim2, period_ticks - 1U);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, period_ticks / 2U);
  __HAL_TIM_SET_COUNTER(&htim2, 0U);
  __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC2 | TIM_FLAG_UPDATE);
  HAL_NVIC_SetPriority(TIM2_IRQn, 2U, 0U);
  HAL_NVIC_ClearPendingIRQ(TIM2_IRQn);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
  serial_motion_pulses_done = 0U;
  serial_motion_active = 1U;
  if (HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_2) != HAL_OK)
  {
    serial_motion_active = 0U;
    TMC5160_DISABLE(&x_tmc5160);
    x_tmc5160_enable_test_active = 0U;
    return SERIAL_MOTION_ERROR_TIMER;
  }
  serial_motion_state = SERIAL_MOTION_STATE_ACTIVE;
  serial_motion_start_ok = 1U;
  return SERIAL_MOTION_ERROR_NONE;
}

static void Serial_Motion_Task(void)
{
  uint8_t start_error_code;
  GPIO_InitTypeDef x_step_gpio = {0};

  if ((serial_motion_command_pending != 0U) &&
      (serial_motion_active == 0U))
  {
    serial_motion_command_pending = 0U;
    serial_motion_start_ok = 0U;
    start_error_code = Serial_Motion_PrepareAndStart();
    if (start_error_code != SERIAL_MOTION_ERROR_NONE)
    {
      serial_motion_error_code = start_error_code;
      serial_motion_busy = 0U;
      serial_motion_state = SERIAL_MOTION_STATE_ERROR;
      serial_motion_last_completion_status = 0xFFU;
      serial_motion_last_completion_error_code = start_error_code;
      serial_motion_last_completion_response_ok =
          Serial_Motion_SendResponse(0xFFU, start_error_code);
      serial_test_last_response_ok =
          serial_motion_last_completion_response_ok;
    }
  }

  if (serial_motion_done != 0U)
  {
    serial_motion_done = 0U;
    x_step_gpio.Pin = X_STEP_Pin;
    x_step_gpio.Mode = GPIO_MODE_OUTPUT_PP;
    x_step_gpio.Pull = GPIO_NOPULL;
    x_step_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(X_STEP_GPIO_Port, &x_step_gpio);
    HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
    serial_motion_mscnt_after =
        TMC5160_ReadRegister(&x_tmc5160, TMC5160_MSCNT);
    serial_motion_mscnt_delta =
        (uint16_t)((serial_motion_mscnt_after -
                    serial_motion_mscnt_before) & 0x03FFU);
    serial_motion_mscnt_ok =
        ((serial_motion_mscnt_delta ==
          (serial_motion_target_steps & 0x03FFU)) ||
         (serial_motion_mscnt_delta ==
          ((0x0400U - (serial_motion_target_steps & 0x03FFU)) &
           0x03FFU)));
    TMC5160_DISABLE(&x_tmc5160);
    x_tmc5160_enable_test_active = 0U;
    serial_motion_busy = 0U;
    serial_motion_state = SERIAL_MOTION_STATE_DONE;
    serial_motion_error_code = SERIAL_MOTION_ERROR_NONE;
    serial_motion_last_completion_status = 0x01U;
    serial_motion_last_completion_error_code = SERIAL_MOTION_ERROR_NONE;
    serial_motion_last_completion_response_ok =
        Serial_Motion_SendResponse(0x01U, SERIAL_MOTION_ERROR_NONE);
    serial_test_last_response_ok = serial_motion_last_completion_response_ok;
  }
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
  if ((htim->Instance == TIM2) && (serial_motion_active != 0U))
  {
    serial_motion_pulses_done++;
    if (serial_motion_pulses_done >= serial_motion_target_steps)
    {
      HAL_TIM_PWM_Stop_IT(htim, TIM_CHANNEL_2);
      serial_motion_active = 0U;
      serial_motion_done = 1U;
    }
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  LED_Init();
  HAL_GPIO_WritePin(GPIOC, X_EN_Pin | Y_EN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, Z_EN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Y_DIR_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(Z_DIR_GPIO_Port, Z_DIR_Pin, GPIO_PIN_RESET);
  limit_gpio_poll_tick = HAL_GetTick();
  x_tmc5160_spi_test_tick = HAL_GetTick();
  x_tmc5160_spi_test_pending = 1U;
  serial_test_build_marker = 0x20260826U;
  /*
  auxiliary_output_test_tick = HAL_GetTick();
  */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    LED_Task();
    /* USER CODE BEGIN 3 */
    Serial_Test_Task();
    Serial_Motion_Task();
    if ((HAL_GetTick() - limit_gpio_poll_tick) >=
        LIMIT_GPIO_POLL_PERIOD_MS)
    {
      limit_gpio_poll_tick = HAL_GetTick();
      limit_pc6_level = (uint8_t)HAL_GPIO_ReadPin(X_LIM_L_GPIO_Port,
                                                   X_LIM_L_Pin);
      limit_pb15_level = (uint8_t)HAL_GPIO_ReadPin(X_LIM_H_GPIO_Port,
                                                    X_LIM_H_Pin);
      limit_pb14_level = (uint8_t)HAL_GPIO_ReadPin(X_LIM_R_GPIO_Port,
                                                    X_LIM_R_Pin);
      limit_pc9_level = (uint8_t)HAL_GPIO_ReadPin(Y_LIM_L_GPIO_Port,
                                                   Y_LIM_L_Pin);
      limit_pc8_level = (uint8_t)HAL_GPIO_ReadPin(Y_LIM_H_GPIO_Port,
                                                   Y_LIM_H_Pin);
      limit_pc7_level = (uint8_t)HAL_GPIO_ReadPin(Y_LIM_R_GPIO_Port,
                                                   Y_LIM_R_Pin);
      limit_pa12_level = (uint8_t)HAL_GPIO_ReadPin(Z_LIM_L_GPIO_Port,
                                                    Z_LIM_L_Pin);
      limit_pa11_level = (uint8_t)HAL_GPIO_ReadPin(Z_LIM_H_GPIO_Port,
                                                    Z_LIM_H_Pin);
      limit_pa10_level = (uint8_t)HAL_GPIO_ReadPin(Z_LIM_R_GPIO_Port,
                                                    Z_LIM_R_Pin);
      limit_active_mask = 0U;
      if (limit_pc6_level == GPIO_PIN_RESET) limit_active_mask |= (1U << 0);
      if (limit_pb15_level == GPIO_PIN_RESET) limit_active_mask |= (1U << 1);
      if (limit_pb14_level == GPIO_PIN_RESET) limit_active_mask |= (1U << 2);
      if (limit_pc9_level == GPIO_PIN_RESET) limit_active_mask |= (1U << 3);
      if (limit_pc8_level == GPIO_PIN_RESET) limit_active_mask |= (1U << 4);
      if (limit_pc7_level == GPIO_PIN_RESET) limit_active_mask |= (1U << 5);
      if (limit_pa12_level == GPIO_PIN_RESET) limit_active_mask |= (1U << 6);
      if (limit_pa11_level == GPIO_PIN_RESET) limit_active_mask |= (1U << 7);
      if (limit_pa10_level == GPIO_PIN_RESET) limit_active_mask |= (1U << 8);
      limit_gpio_sample_valid = 1U;
    }
#if !LIMIT_GPIO_STATIC_TEST && !SERIAL_PROTOCOL_STAGE1_TEST
    if ((x_tmc5160_spi_test_pending != 0U) &&
        ((HAL_GetTick() - x_tmc5160_spi_test_tick) >= 100U))
    {
      x_tmc5160_ioin = TMC5160_ReadRegister(&x_tmc5160, TMC5160_IOIN);
      x_tmc5160_spi_ok = ((x_tmc5160_ioin >> 24) == 0x30U);
      x_tmc5160_spi_test_done = 1U;
      x_tmc5160_spi_test_pending = 0U;

      if (x_tmc5160_spi_ok != 0U)
      {
        TMC5160_WriteRegister(&x_tmc5160, TMC5160_GCONF,
                              TMC5160_GCONF_INIT);
        x_tmc5160_gconf = TMC5160_ReadRegister(&x_tmc5160, TMC5160_GCONF);
        x_tmc5160_gconf_ok = (x_tmc5160_gconf == TMC5160_GCONF_INIT);
        x_tmc5160_gconf_test_done = 1U;

        if (x_tmc5160_gconf_ok != 0U)
        {
          x_tmc5160_gstat = TMC5160_ReadRegister(&x_tmc5160, TMC5160_GSTAT);
          x_tmc5160_gstat_test_done = 1U;
          TMC5160_WriteRegister(&x_tmc5160, TMC5160_GSTAT,
                                TMC5160_GSTAT_INIT);
          x_tmc5160_gstat_clear_tick = HAL_GetTick();
          x_tmc5160_gstat_clear_pending = 1U;
        }
      }
    }

    if ((x_tmc5160_gstat_clear_pending != 0U) &&
        ((HAL_GetTick() - x_tmc5160_gstat_clear_tick) >= 100U))
    {
      x_tmc5160_gstat_after_clear =
          TMC5160_ReadRegister(&x_tmc5160, TMC5160_GSTAT);
      x_tmc5160_gstat_uv_cp_clear_ok =
          ((x_tmc5160_gstat_after_clear & TMC5160_GSTAT_UV_CP) == 0U);
      x_tmc5160_gstat_clear_test_done = 1U;
      x_tmc5160_gstat_clear_pending = 0U;
    }

    if ((x_tmc5160_gstat_clear_test_done != 0U) &&
        (x_tmc5160_static_read_test_done == 0U))
    {
      x_tmc5160_chopconf =
          TMC5160_ReadRegister(&x_tmc5160, TMC5160_CHOPCONF);
      x_tmc5160_static_read_test_done = 1U;
    }

    if ((x_tmc5160_static_read_test_done != 0U) &&
        (x_tmc5160_low_current_config_test_done == 0U))
    {
      TMC5160_WriteRegister(&x_tmc5160, TMC5160_IHOLD_IRUN,
                            TMC5160_IHOLD(X_MOTION_IHOLD) |
                            TMC5160_IRUN(X_MOTION_IRUN));
      TMC5160_WriteRegister(&x_tmc5160, TMC5160_CHOPCONF,
                            (x_tmc5160_chopconf & ~TMC5160_TOFF_MASK) |
                            TMC5160_TOFF(3U));
      x_tmc5160_chopconf_configured =
          TMC5160_ReadRegister(&x_tmc5160, TMC5160_CHOPCONF);
      x_tmc5160_low_current_config_ok =
          (x_tmc5160_chopconf_configured ==
           ((x_tmc5160_chopconf & ~TMC5160_TOFF_MASK) |
            TMC5160_TOFF(3U)));
      x_tmc5160_low_current_config_test_done = 1U;
    }

    if ((x_tmc5160_low_current_config_ok != 0U) &&
        (x_tmc5160_enable_test_done == 0U) &&
        (x_tmc5160_enable_test_pending == 0U))
    {
      TMC5160_ENABLE(&x_tmc5160);
      x_tmc5160_enable_test_tick = HAL_GetTick();
      x_tmc5160_enable_test_pending = 1U;
      x_tmc5160_enable_test_active = 1U;
    }

    if ((x_tmc5160_enable_test_pending != 0U) &&
        ((HAL_GetTick() - x_tmc5160_enable_test_tick) >= 200U))
    {
      x_tmc5160_gstat_enabled =
          TMC5160_ReadRegister(&x_tmc5160, TMC5160_GSTAT);
      x_tmc5160_drv_status_enabled =
          TMC5160_ReadRegister(&x_tmc5160, TMC5160_DRV_STATUS);
      x_tmc5160_enable_test_ok =
          ((x_tmc5160_gstat_enabled &
            (TMC5160_GSTAT_DRV_ERR | TMC5160_GSTAT_UV_CP)) == 0U);
      if (x_tmc5160_enable_test_ok == 0U)
      {
        TMC5160_DISABLE(&x_tmc5160);
        x_tmc5160_enable_test_active = 0U;
      }
      x_tmc5160_enable_test_done = 1U;
      x_tmc5160_enable_test_pending = 0U;
    }

    if ((x_tmc5160_enable_test_active != 0U) &&
        (x_tmc5160_enable_test_ok != 0U) &&
        (x_tmc5160_step_test_done == 0U))
    {
      if (x_tmc5160_step_test_state == 0U)
      {
        GPIO_InitTypeDef x_step_gpio = {0};

        x_step_gpio.Pin = X_STEP_Pin;
        x_step_gpio.Mode = GPIO_MODE_OUTPUT_PP;
        x_step_gpio.Pull = GPIO_NOPULL;
        x_step_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(X_STEP_GPIO_Port, &x_step_gpio);
        HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
        x_tmc5160_step_test_tick = HAL_GetTick();
        x_tmc5160_step_test_state = 1U;
      }
      else if ((x_tmc5160_step_test_state == 1U) &&
               ((HAL_GetTick() - x_tmc5160_step_test_tick) >= 1U))
      {
        x_tmc5160_mscnt_before_step =
            TMC5160_ReadRegister(&x_tmc5160, TMC5160_MSCNT);
        HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_SET);
        x_tmc5160_step_test_tick = HAL_GetTick();
        x_tmc5160_step_test_state = 2U;
      }
      else if ((x_tmc5160_step_test_state == 2U) &&
               ((HAL_GetTick() - x_tmc5160_step_test_tick) >= 1U))
      {
        HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
        x_tmc5160_step_test_tick = HAL_GetTick();
        x_tmc5160_step_test_state = 3U;
      }
      else if ((x_tmc5160_step_test_state == 3U) &&
               ((HAL_GetTick() - x_tmc5160_step_test_tick) >= 1U))
      {
        GPIO_InitTypeDef x_step_gpio = {0};

        x_tmc5160_mscnt_after_step =
            TMC5160_ReadRegister(&x_tmc5160, TMC5160_MSCNT);
        x_tmc5160_mscnt_step_delta =
            (uint16_t)((x_tmc5160_mscnt_after_step -
                        x_tmc5160_mscnt_before_step) & 0x03FFU);
        x_tmc5160_step_test_ok =
            ((x_tmc5160_mscnt_step_delta == 1U) ||
             (x_tmc5160_mscnt_step_delta == 0x03FFU));
        x_tmc5160_step_test_done = 1U;

        x_step_gpio.Pin = X_STEP_Pin;
        x_step_gpio.Mode = GPIO_MODE_AF_PP;
        x_step_gpio.Pull = GPIO_NOPULL;
        x_step_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(X_STEP_GPIO_Port, &x_step_gpio);
      }
    }

    if ((x_tmc5160_enable_test_active != 0U) &&
        (x_tmc5160_step_test_ok != 0U) &&
        (x_tmc5160_multi_step_test_done == 0U))
    {
      if (x_tmc5160_multi_step_test_state == 0U)
      {
        GPIO_InitTypeDef x_step_gpio = {0};

        x_step_gpio.Pin = X_STEP_Pin;
        x_step_gpio.Mode = GPIO_MODE_OUTPUT_PP;
        x_step_gpio.Pull = GPIO_NOPULL;
        x_step_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(X_STEP_GPIO_Port, &x_step_gpio);
        HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
        x_tmc5160_mscnt_before_multi_step =
            TMC5160_ReadRegister(&x_tmc5160, TMC5160_MSCNT);
        x_tmc5160_multi_step_test_tick = HAL_GetTick();
        x_tmc5160_multi_step_test_state = 1U;
      }
      else if ((x_tmc5160_multi_step_test_state == 1U) &&
               ((HAL_GetTick() - x_tmc5160_multi_step_test_tick) >= 1U))
      {
        HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_SET);
        x_tmc5160_multi_step_test_tick = HAL_GetTick();
        x_tmc5160_multi_step_test_state = 2U;
      }
      else if ((x_tmc5160_multi_step_test_state == 2U) &&
               ((HAL_GetTick() - x_tmc5160_multi_step_test_tick) >= 1U))
      {
        HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
        x_tmc5160_multi_step_test_tick = HAL_GetTick();
        x_tmc5160_multi_step_test_state = 3U;
      }
      else if ((x_tmc5160_multi_step_test_state == 3U) &&
               ((HAL_GetTick() - x_tmc5160_multi_step_test_tick) >= 49U))
      {
        x_tmc5160_multi_step_test_count++;
        if (x_tmc5160_multi_step_test_count >= 16U)
        {
          GPIO_InitTypeDef x_step_gpio = {0};

          x_tmc5160_mscnt_after_multi_step =
              TMC5160_ReadRegister(&x_tmc5160, TMC5160_MSCNT);
          x_tmc5160_mscnt_multi_step_delta =
              (uint16_t)((x_tmc5160_mscnt_after_multi_step -
                          x_tmc5160_mscnt_before_multi_step) & 0x03FFU);
          x_tmc5160_multi_step_test_ok =
              ((x_tmc5160_mscnt_multi_step_delta == 16U) ||
               (x_tmc5160_mscnt_multi_step_delta == 0x03F0U));
          x_tmc5160_multi_step_test_done = 1U;

          x_step_gpio.Pin = X_STEP_Pin;
          x_step_gpio.Mode = GPIO_MODE_AF_PP;
          x_step_gpio.Pull = GPIO_NOPULL;
          x_step_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
          HAL_GPIO_Init(X_STEP_GPIO_Port, &x_step_gpio);
        }
        else
        {
          x_tmc5160_multi_step_test_state = 1U;
        }
      }
    }

    if ((x_tmc5160_enable_test_active != 0U) &&
        (x_tmc5160_multi_step_test_ok != 0U) &&
        (x_tmc5160_motion_test_done == 0U))
    {
      if (x_tmc5160_motion_test_state == 0U)
      {
        HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
        x_tmc5160_motion_test_start_tick = HAL_GetTick();
        if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2) == HAL_OK)
        {
          x_tmc5160_motion_test_active = 1U;
          x_tmc5160_motion_test_state = 1U;
        }
        else
        {
          x_tmc5160_motion_test_done = 1U;
        }
      }
      else if ((HAL_GetTick() - x_tmc5160_motion_test_start_tick) >=
               X_MOTION_TEST_DURATION_MS)
      {
        GPIO_InitTypeDef x_step_gpio = {0};

        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
        x_step_gpio.Pin = X_STEP_Pin;
        x_step_gpio.Mode = GPIO_MODE_OUTPUT_PP;
        x_step_gpio.Pull = GPIO_NOPULL;
        x_step_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(X_STEP_GPIO_Port, &x_step_gpio);
        HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
        x_tmc5160_motion_test_active = 0U;
        x_tmc5160_motion_test_done = 1U;
      }
    }

    if ((x_tmc5160_enable_test_active != 0U) &&
        ((HAL_GetTick() - x_tmc5160_enable_test_tick) >=
         X_MOTION_TEST_DISABLE_DELAY_MS))
    {
      TMC5160_DISABLE(&x_tmc5160);
      x_tmc5160_enable_test_active = 0U;
    }
#endif
    /*
    if ((auxiliary_output_test_state == 0U) &&
        ((HAL_GetTick() - auxiliary_output_test_tick) >= 5000U))
    {
      HAL_GPIO_WritePin(GPIOB, VOUT_5_Pin | VOUT_24_Pin, GPIO_PIN_SET);
      auxiliary_output_test_tick = HAL_GetTick();
      auxiliary_output_test_state = 1U;
    }
    else if ((auxiliary_output_test_state == 1U) &&
             ((HAL_GetTick() - auxiliary_output_test_tick) >= 5000U))
    {
      HAL_GPIO_WritePin(GPIOB, VOUT_5_Pin | VOUT_24_Pin, GPIO_PIN_RESET);
      auxiliary_output_test_state = 2U;
    }
    */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
