/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, I2C1_LDAC_Pin|LED_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, X_CS_Pin|X_EN_Pin|Y_CS_Pin|Y_EN_Pin, GPIO_PIN_SET);
	
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Y_DIR_Pin|VOUT_5_Pin|VOUT_24_Pin
                          |Z_DIR_Pin, GPIO_PIN_RESET);
	
	/*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Z_CS_Pin|Z_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : I2C1_LDAC_Pin LED_Pin X_CS_Pin X_EN_Pin
                           Y_CS_Pin Y_EN_Pin */
  GPIO_InitStruct.Pin = I2C1_LDAC_Pin|LED_Pin|X_CS_Pin|X_EN_Pin
                          |Y_CS_Pin|Y_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : I2C1_RDY_Pin */
  GPIO_InitStruct.Pin = I2C1_RDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(I2C1_RDY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : X_DIR_Pin */
  GPIO_InitStruct.Pin = X_DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(X_DIR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Y_DIR_Pin VOUT_5_Pin VOUT_24_Pin Z_EN_Pin
                           Z_DIR_Pin Z_CS_Pin */
  GPIO_InitStruct.Pin = Y_DIR_Pin|VOUT_5_Pin|VOUT_24_Pin|Z_EN_Pin
                          |Z_DIR_Pin|Z_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : X_LIM_R_Pin X_LIM_H_Pin */
  GPIO_InitStruct.Pin = X_LIM_R_Pin|X_LIM_H_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : X_LIM_L_Pin Y_LIM_R_Pin Y_LIM_H_Pin Y_LIM_L_Pin */
  GPIO_InitStruct.Pin = X_LIM_L_Pin|Y_LIM_R_Pin|Y_LIM_H_Pin|Y_LIM_L_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : Z_LIM_R_Pin Z_LIM_H_Pin Z_LIM_L_Pin */
  GPIO_InitStruct.Pin = Z_LIM_R_Pin|Z_LIM_H_Pin|Z_LIM_L_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
