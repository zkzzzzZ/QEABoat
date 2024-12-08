/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

void SystemClock_Config(void);

char buffer[256];  // 定义一个足够大的缓冲区
int printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  
  // 这里使用 vsnprintf 临时生成一个缓冲区
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  // 通过 HAL_UART_Transmit 发送输出字符
  HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
  
  return 0;
}

void AlignPWM(void){
  // 启动 PWM
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

  // 校准 ESC：最大值信号
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 2000); // 2ms
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 2000); // 2ms
  HAL_Delay(500);

  // 校准 ESC：最小值信号
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 1000); // 1ms
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 1000); // 1ms
  HAL_Delay(500);

  // 校准 ESC：中间值信号
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 1500); // 1.5ms
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 1500); // 1.5ms
  HAL_Delay(500);

  // 校准完成后，电机默认停转
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 1500);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 1500);
}

int main(void)
{
  uint8_t received_data[8] = {0x00};
  uint8_t transmit_data[11] = {0x12, 0x12, 0x17};

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();

  // 初始化无线模块
  uint8_t state = 0;
  while (state == 0){
    state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);
    //printf("State: %d\n", state);
    HAL_Delay(50);
  }
  //printf("Init complete.\n");
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

  uint8_t inited = 0;
  while (1){
    // 处理遥控数据
    HAL_UART_Receive(&huart3, received_data, 8, HAL_MAX_DELAY);
    //HAL_UART_Transmit(&huart2, received_data, 8, HAL_MAX_DELAY);
    //printf(received_data);

    switch (received_data[0]){
      case 0x01: { // 初始化
        if (inited){ // 发送错误码
          transmit_data[3] = 0x41;
        }
        else{
          AlignPWM();
          transmit_data[3] = 0x01;
          inited = 1;
        }
      }break;
      case 0x02: { // 发送控制信号
        if (inited){ // 若电机已初始化，则驱动
          transmit_data[3] = 0x02;

          // 为左右两电机发送控制信号
          __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, received_data[1] << 8 + received_data[2]);
          __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, received_data[3] << 8 + received_data[4]);
        }
        else{ // 发送错误码
          transmit_data[3] = 0x42;
        }
      }break;
    }
    HAL_UART_Transmit(&huart3, transmit_data, 11, HAL_MAX_DELAY);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  }

  /*
    PB14 -> MD0
    PB15 -> MD1
    PB13 -> AUX
    PB11(RX3) -> TXD
    PB10(TX3) -> RXD

    PA2(TX2) -> RXD(PC)
    PA3(RX2) -> TXD(PC)
  */

  /*
  HAL_Delay(1000);
  AlignPWM(TIM_CHANNEL_1);
  //AlignPWM(TIM_CHANNEL_2);

  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 1750);
  //uint16_t i = 1200;
  // 主循环：维持慢速运行
  while (1)
  {
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, i);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    HAL_Delay(3000);
    i = 3000 - i; // 双向！
  }
  */
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */