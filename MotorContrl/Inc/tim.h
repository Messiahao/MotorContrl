/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* HAL/ISR ABI exception: declaration only. The unchanged IRQ uses &htim2.
   Application modules must use BspTim_* instead of sharing this handle. */
extern TIM_HandleTypeDef htim2;



/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void BspTim2_Init(void);
void BspTim3_Init(void);
void BspTim4_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN Prototypes */

#define BSP_TIM_PRESCALER 71
#define BSP_TIM2_INITIAL_PERIOD 99
#define BSP_TIM34_INITIAL_PERIOD 999
#define BSP_TIM_INITIAL_PULSE 50
#define BSP_TIM2_IRQ_PRIORITY 2U
#define BSP_TIM2_IRQ_SUBPRIORITY 0U

void BspTim_WriteXSetupInterrupt(void);
HAL_StatusTypeDef BspTim_WriteXStart(void);
HAL_StatusTypeDef BspTim_WriteXStop(void);
void BspTim_WriteXDisableUpdate(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */
