/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32g4xx_hal.h"

#include "stm32g4xx_ll_spi.h"
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_dma.h"

#include "stm32g4xx_ll_exti.h"

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
#define CAP_Vdet_Pin GPIO_PIN_0
#define CAP_Vdet_GPIO_Port GPIOA
#define DCcap_Idet_Pin GPIO_PIN_1
#define DCcap_Idet_GPIO_Port GPIOA
#define BAT_Vdet_Pin GPIO_PIN_4
#define BAT_Vdet_GPIO_Port GPIOA
#define DCsource_Idet_Pin GPIO_PIN_6
#define DCsource_Idet_GPIO_Port GPIOA
#define Chassis_Idet_Pin GPIO_PIN_0
#define Chassis_Idet_GPIO_Port GPIOB
#define TemDet_Pin GPIO_PIN_1
#define TemDet_GPIO_Port GPIOB
#define LED_RED_Pin GPIO_PIN_11
#define LED_RED_GPIO_Port GPIOB
#define LED_BLUE_Pin GPIO_PIN_12
#define LED_BLUE_GPIO_Port GPIOB
#define CapDriver_Enable_Pin GPIO_PIN_15
#define CapDriver_Enable_GPIO_Port GPIOB
#define CapDriver_HighPWM_Pin GPIO_PIN_8
#define CapDriver_HighPWM_GPIO_Port GPIOA
#define CapDriver_LowPWM_Pin GPIO_PIN_9
#define CapDriver_LowPWM_GPIO_Port GPIOA
#define SourceDriver_HighPWM_Pin GPIO_PIN_10
#define SourceDriver_HighPWM_GPIO_Port GPIOA
#define SourceDriver_LowPWM_Pin GPIO_PIN_11
#define SourceDriver_LowPWM_GPIO_Port GPIOA
#define SourceDriver_Enable_Pin GPIO_PIN_12
#define SourceDriver_Enable_GPIO_Port GPIOA
#define OLED_DC_Pin GPIO_PIN_15
#define OLED_DC_GPIO_Port GPIOA
#define OLED_SCK_Pin GPIO_PIN_3
#define OLED_SCK_GPIO_Port GPIOB
#define OLED_RES_Pin GPIO_PIN_4
#define OLED_RES_GPIO_Port GPIOB
#define OLED_MOSI_Pin GPIO_PIN_5
#define OLED_MOSI_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define CCMRAM __attribute__((section("ccmram")))
#define __CAP_ESR (0.17f)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
