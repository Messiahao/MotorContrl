/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
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
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */


/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void BspI2c_Init(void);

/* USER CODE BEGIN Prototypes */

#define BSP_I2C_CLOCK_HZ 100000

/* address is a HAL-format address: 7-bit address << 1. */
HAL_StatusTypeDef BspI2c_Write(uint16_t address, uint8_t *data, uint16_t length, uint32_t timeout);
HAL_StatusTypeDef BspI2c_Read(uint16_t address, uint8_t *data, uint16_t length, uint32_t timeout);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */
