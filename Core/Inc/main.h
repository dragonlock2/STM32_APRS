/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32l4xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KEY_7_Pin GPIO_PIN_13
#define KEY_7_GPIO_Port GPIOC
#define KEY_6_Pin GPIO_PIN_14
#define KEY_6_GPIO_Port GPIOC
#define KEY_R_Pin GPIO_PIN_15
#define KEY_R_GPIO_Port GPIOC
#define KEY_9_Pin GPIO_PIN_0
#define KEY_9_GPIO_Port GPIOH
#define KEY_L_Pin GPIO_PIN_1
#define KEY_L_GPIO_Port GPIOH
#define RX_DET_Pin GPIO_PIN_0
#define RX_DET_GPIO_Port GPIOA
#define TX_PTT_Pin GPIO_PIN_2
#define TX_PTT_GPIO_Port GPIOA
#define LED_3_Pin GPIO_PIN_1
#define LED_3_GPIO_Port GPIOB
#define LED_2_Pin GPIO_PIN_2
#define LED_2_GPIO_Port GPIOB
#define LED_1_Pin GPIO_PIN_10
#define LED_1_GPIO_Port GPIOB
#define LED_0_Pin GPIO_PIN_11
#define LED_0_GPIO_Port GPIOB
#define TX_LED_Pin GPIO_PIN_12
#define TX_LED_GPIO_Port GPIOB
#define RX_LED_Pin GPIO_PIN_13
#define RX_LED_GPIO_Port GPIOB
#define KEY_2_Pin GPIO_PIN_4
#define KEY_2_GPIO_Port GPIOB
#define KEY_1_Pin GPIO_PIN_5
#define KEY_1_GPIO_Port GPIOB
#define KEY_0_Pin GPIO_PIN_6
#define KEY_0_GPIO_Port GPIOB
#define KEY_5_Pin GPIO_PIN_7
#define KEY_5_GPIO_Port GPIOB
#define KEY_4_Pin GPIO_PIN_3
#define KEY_4_GPIO_Port GPIOH
#define KEY_3_Pin GPIO_PIN_8
#define KEY_3_GPIO_Port GPIOB
#define KEY_8_Pin GPIO_PIN_9
#define KEY_8_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
