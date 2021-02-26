/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */
//TIM1定时器，任务在定时器中不断循环
#include <stdio.h>
#include <string.h>
#include "bmp.h"
#include "oled.h"
#include "ina226.h"

char char_V[20], char_I[20], char_W[20];

HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim1)
    {
        Data_Refresh_Task(); //每10ms更新一次数据
        Oled_Data_List();
    }
}

void Data_Refresh_Task()
{
    uint16_t V_int = 0, V_dec = 0, I_int = 0, I_dec = 0, W_int = 0, W_dec = 0;

    get_power(); //INA226读取电压(mv)，读取电�?(mA),功率(W)
    V_int = (uint16_t)(INA226_data.voltageVal / 1000);
    V_dec = (uint16_t)((INA226_data.voltageVal / 1000 - V_int) * 1000);
    I_int = (uint16_t)(INA226_data.Shunt_Current / 1000);
    I_dec = (uint16_t)((INA226_data.Shunt_Current / 1000 - I_int) * 1000);
    W_int = (uint16_t)INA226_data.Power_Val;
    W_dec = (uint16_t)((INA226_data.Power_Val - W_int) * 1000);
    sprintf(char_V, "Voltage:%d.%03dV ", V_int, V_dec);
    sprintf(char_I, "Current:%d.%03dA ", I_int, I_dec);
    sprintf(char_W, "Power:  %d.%03dW ", W_int, W_dec);
}

void Oled_Data_List()
{
    OLED_ShowString(40, 0, "Power_X", 16);
    OLED_ShowString(0, 2, char_V, 16);
    OLED_ShowString(0, 4, char_I, 16);
    OLED_ShowString(0, 6, char_W, 16);
}
/* USER CODE END 0 */

TIM_HandleTypeDef htim1;

/* TIM1 init function */
void MX_TIM1_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 720 - 1;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 1000 - 1;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *tim_baseHandle)
{

    if (tim_baseHandle->Instance == TIM1)
    {
        /* USER CODE BEGIN TIM1_MspInit 0 */

        /* USER CODE END TIM1_MspInit 0 */
        /* TIM1 clock enable */
        __HAL_RCC_TIM1_CLK_ENABLE();

        /* TIM1 interrupt Init */
        HAL_NVIC_SetPriority(TIM1_UP_IRQn, 2, 1);
        HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
        /* USER CODE BEGIN TIM1_MspInit 1 */

        /* USER CODE END TIM1_MspInit 1 */
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *tim_baseHandle)
{

    if (tim_baseHandle->Instance == TIM1)
    {
        /* USER CODE BEGIN TIM1_MspDeInit 0 */

        /* USER CODE END TIM1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM1_CLK_DISABLE();

        /* TIM1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(TIM1_UP_IRQn);
        /* USER CODE BEGIN TIM1_MspDeInit 1 */

        /* USER CODE END TIM1_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
