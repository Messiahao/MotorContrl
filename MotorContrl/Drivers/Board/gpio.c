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
#include "config.h"

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
static void MX_GPIO_Init(void)
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
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : X_LIM_L_Pin Y_LIM_R_Pin Y_LIM_H_Pin Y_LIM_L_Pin */
  GPIO_InitStruct.Pin = X_LIM_L_Pin|Y_LIM_R_Pin|Y_LIM_H_Pin|Y_LIM_L_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : Z_LIM_R_Pin Z_LIM_H_Pin Z_LIM_L_Pin */
  GPIO_InitStruct.Pin = Z_LIM_R_Pin|Z_LIM_H_Pin|Z_LIM_L_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

void BspGpio_Init(void)
{
  MX_GPIO_Init();
}

void BspGpio_EnableLimitInterrupts(void)
{
  HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
  HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
  __HAL_GPIO_EXTI_CLEAR_IT(X_LIM_L_Pin | Y_LIM_R_Pin | Y_LIM_H_Pin | Y_LIM_L_Pin);
  __HAL_GPIO_EXTI_CLEAR_IT(Z_LIM_R_Pin | Z_LIM_H_Pin | Z_LIM_L_Pin |
                           X_LIM_R_Pin | X_LIM_H_Pin);
  HAL_NVIC_SetPriority(EXTI9_5_IRQn,
                       LIMIT_GPIO_EXTI_PRIORITY,
                       LIMIT_GPIO_EXTI_SUBPRIORITY);
  HAL_NVIC_SetPriority(EXTI15_10_IRQn,
                       LIMIT_GPIO_EXTI_PRIORITY,
                       LIMIT_GPIO_EXTI_SUBPRIORITY);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

GPIO_PinState BspGpio_Read(GPIO_TypeDef *port, uint16_t pin)
{
  return HAL_GPIO_ReadPin(port, pin);
}

void BspGpio_Write(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState level)
{
  HAL_GPIO_WritePin(port, pin, level);
}

void BspGpio_WriteStepMode(GPIO_TypeDef *port, uint16_t pin, uint32_t mode)
{
  GPIO_InitTypeDef step_gpio = {0};
  step_gpio.Pin = pin;
  step_gpio.Mode = mode;
  step_gpio.Pull = GPIO_NOPULL;
  step_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(port, &step_gpio);
}

GPIO_PinState BspGpio_ReadLimitPin(uint16_t pin)
{
  switch (pin)
  {
    case X_LIM_L_Pin: return HAL_GPIO_ReadPin(X_LIM_L_GPIO_Port, X_LIM_L_Pin);
    case X_LIM_H_Pin: return HAL_GPIO_ReadPin(X_LIM_H_GPIO_Port, X_LIM_H_Pin);
    case X_LIM_R_Pin: return HAL_GPIO_ReadPin(X_LIM_R_GPIO_Port, X_LIM_R_Pin);
    case Y_LIM_L_Pin: return HAL_GPIO_ReadPin(Y_LIM_L_GPIO_Port, Y_LIM_L_Pin);
    case Y_LIM_H_Pin: return HAL_GPIO_ReadPin(Y_LIM_H_GPIO_Port, Y_LIM_H_Pin);
    case Y_LIM_R_Pin: return HAL_GPIO_ReadPin(Y_LIM_R_GPIO_Port, Y_LIM_R_Pin);
    case Z_LIM_L_Pin: return HAL_GPIO_ReadPin(Z_LIM_L_GPIO_Port, Z_LIM_L_Pin);
    case Z_LIM_H_Pin: return HAL_GPIO_ReadPin(Z_LIM_H_GPIO_Port, Z_LIM_H_Pin);
    case Z_LIM_R_Pin: return HAL_GPIO_ReadPin(Z_LIM_R_GPIO_Port, Z_LIM_R_Pin);
    default: return GPIO_PIN_RESET;
  }
}

uint16_t BspGpio_LimitBitFromPin(uint16_t pin)
{
  switch (pin)
  {
    case X_LIM_L_Pin: return (1U << BSP_GPIO_X_LIMIT_L_BIT);
    case X_LIM_H_Pin: return (1U << BSP_GPIO_X_LIMIT_H_BIT);
    case X_LIM_R_Pin: return (1U << BSP_GPIO_X_LIMIT_R_BIT);
    case Y_LIM_L_Pin: return (1U << BSP_GPIO_Y_LIMIT_L_BIT);
    case Y_LIM_H_Pin: return (1U << BSP_GPIO_Y_LIMIT_H_BIT);
    case Y_LIM_R_Pin: return (1U << BSP_GPIO_Y_LIMIT_R_BIT);
    case Z_LIM_L_Pin: return (1U << BSP_GPIO_Z_LIMIT_L_BIT);
    case Z_LIM_H_Pin: return (1U << BSP_GPIO_Z_LIMIT_H_BIT);
    case Z_LIM_R_Pin: return (1U << BSP_GPIO_Z_LIMIT_R_BIT);
    default: return 0U;
  }
}

uint16_t BspGpio_ReadLimitActiveMask(void)
{
  static const uint16_t limit_pins[] = {
    X_LIM_L_Pin, X_LIM_H_Pin, X_LIM_R_Pin,
    Y_LIM_L_Pin, Y_LIM_H_Pin, Y_LIM_R_Pin,
    Z_LIM_L_Pin, Z_LIM_H_Pin, Z_LIM_R_Pin
  };
  uint16_t limit_mask = 0U;
  uint8_t i;

  for (i = 0U; i < (sizeof(limit_pins) / sizeof(limit_pins[0])); i++)
  {
    if (BspGpio_ReadLimitPin(limit_pins[i]) == LIMIT_GPIO_ACTIVE_LEVEL)
    {
      limit_mask |= BspGpio_LimitBitFromPin(limit_pins[i]);
    }
  }
  return limit_mask;
}

uint16_t BspGpio_LimitMaskForAxis(uint8_t axis)
{
  switch (axis)
  {
    case SERIAL_MOTION_AXIS_X: return X_LIMIT_ACTIVE_MASK;
    case SERIAL_MOTION_AXIS_Y: return Y_LIMIT_ACTIVE_MASK;
    case SERIAL_MOTION_AXIS_Z: return Z_LIMIT_CHECK_MASK;
    default: return 0U;
  }
}

uint16_t BspGpio_ReadAxisLimitMask(uint8_t axis)
{
  return (uint16_t)(BspGpio_ReadLimitActiveMask() &
                    BspGpio_LimitMaskForAxis(axis));
}
