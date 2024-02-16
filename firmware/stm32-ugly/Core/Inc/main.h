/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void usb_rx_interrupt(uint8_t* Buf, uint32_t *Len);
void usb_rx_flush();

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PIN_GIRA_TX_Pin GPIO_PIN_2
#define PIN_GIRA_TX_GPIO_Port GPIOA
#define PIN_GIRA_TX_EN_Pin GPIO_PIN_3
#define PIN_GIRA_TX_EN_GPIO_Port GPIOA
#define PIN_IN1_Pin GPIO_PIN_5
#define PIN_IN1_GPIO_Port GPIOA
#define PIN_IN2_Pin GPIO_PIN_6
#define PIN_IN2_GPIO_Port GPIOA
#define PIN_IN3_Pin GPIO_PIN_7
#define PIN_IN3_GPIO_Port GPIOA
#define PIN_OUT3_Pin GPIO_PIN_1
#define PIN_OUT3_GPIO_Port GPIOB
#define PIN_OUT2_Pin GPIO_PIN_10
#define PIN_OUT2_GPIO_Port GPIOB
#define PIN_OUT1_Pin GPIO_PIN_11
#define PIN_OUT1_GPIO_Port GPIOB
#define PIN_LED1_Pin GPIO_PIN_12
#define PIN_LED1_GPIO_Port GPIOB
#define PIN_LED2_Pin GPIO_PIN_13
#define PIN_LED2_GPIO_Port GPIOB
#define PIN_GIRA_RX_Pin GPIO_PIN_5
#define PIN_GIRA_RX_GPIO_Port GPIOB
#define PIN_IN4_Pin GPIO_PIN_8
#define PIN_IN4_GPIO_Port GPIOB
#define PIN_IN4_EXTI_IRQn EXTI9_5_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
