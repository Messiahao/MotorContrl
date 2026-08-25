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
/*
static uint32_t auxiliary_output_test_tick;
static uint8_t auxiliary_output_test_state;
*/
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  TMC5160_DISABLE(&x_tmc5160);
  x_tmc5160_spi_test_tick = HAL_GetTick();
  x_tmc5160_spi_test_pending = 1U;
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
      TMC5160_WriteRegister(&x_tmc5160, TMC5160_IHOLD_IRUN, 0U);
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
        ((HAL_GetTick() - x_tmc5160_enable_test_tick) >= 5000U))
    {
      TMC5160_DISABLE(&x_tmc5160);
      x_tmc5160_enable_test_active = 0U;
    }
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
