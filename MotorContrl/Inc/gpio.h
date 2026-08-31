/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

#define BSP_GPIO_X_LIMIT_L_BIT 0
#define BSP_GPIO_X_LIMIT_H_BIT 1
#define BSP_GPIO_X_LIMIT_R_BIT 2
#define BSP_GPIO_Y_LIMIT_L_BIT 3
#define BSP_GPIO_Y_LIMIT_H_BIT 4
#define BSP_GPIO_Y_LIMIT_R_BIT 5
#define BSP_GPIO_Z_LIMIT_L_BIT 6
#define BSP_GPIO_Z_LIMIT_H_BIT 7
#define BSP_GPIO_Z_LIMIT_R_BIT 8

/* USER CODE END Private defines */

void BspGpio_Init(void);

/* USER CODE BEGIN Prototypes */

GPIO_PinState BspGpio_Read(GPIO_TypeDef *port, uint16_t pin);
void BspGpio_Write(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState level);
void BspGpio_WriteXStepMode(uint32_t mode);
uint16_t BspGpio_ReadXLimitMask(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /*__ GPIO_H__ */
