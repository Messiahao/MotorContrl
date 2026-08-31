/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */



/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void BspUsart2_Init(void);
void BspUsart3_Init(void);

/* USER CODE BEGIN Prototypes */

#define BSP_USART_BAUD_RATE 115200
#define BSP_USART_BYTE_MASK 0x00FFU

uint8_t BspUsart_ReadOverrun(void);
void BspUsart_WriteClearOverrun(void);
uint8_t BspUsart_ReadAvailable(void);
uint8_t BspUsart_ReadByte(void);
HAL_StatusTypeDef BspUsart_Write(uint8_t *data, uint16_t length, uint32_t timeout);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */
