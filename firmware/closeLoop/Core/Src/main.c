/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "opamp.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "foc.h"
#include "as5600.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define PI 3.14159265358979f
#define PHASE_SHIFT_ANGLE (float)(220.0f / 360.0f * 2.0f * PI)
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern DMA_HandleTypeDef hdma_usart3_tx;
uint8_t DataB1[32] = "LED1 Toggle\r\n";
uint8_t DataB2[32] = "LED2 Toggle\r\n";
uint8_t DataB3[32] = "LED1 and LED2 Open\r\n";
#define RXBUFFERSIZE 32
char RxBuffer[RXBUFFERSIZE];
uint8_t aRxBuffer;
uint8_t Uart1_Rx_Cnt = 0;
float Vbus, Ia, Ib, Ic;
uint16_t IA_Offset, IB_Offset, IC_Offset;
uint16_t adc1_in1, adc1_in2, adc1_in3, Vpoten, adc_vbus;
uint8_t ADC_offset = 0;
float temp[5];
uint8_t tempData[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x80, 0x7F};
float HallTemp = 0;
float HallThetaAdd = 0;
float HallTheta = 0;
float HallSpeed = 0;
float HallSpeedLast = 0;
float HallSpeedtest = 0;
float alpha = 0.3;
uint8_t HallReadTemp = 0;
bool recvDataToProcess;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_OPAMP1_Init();
  MX_OPAMP2_Init();
  MX_OPAMP3_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_OPAMP_Start(&hopamp1);
  HAL_OPAMP_Start(&hopamp2);
  HAL_OPAMP_Start(&hopamp3);
  HAL_UART_Receive_IT(&huart3, (uint8_t *)&aRxBuffer, 1);
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
  __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_JEOC);
  __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_EOC);
  __HAL_ADC_CLEAR_FLAG(&hadc2, ADC_FLAG_JEOC);
  HAL_ADCEx_InjectedStart_IT(&hadc1);
  HAL_ADCEx_InjectedStart(&hadc2);
  TIM1->ARR = 8000 - 1;
  TIM1->CCR4 = 8000 - 2;

  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  HAL_TIMEx_HallSensor_Start_IT(&htim4);
  as5600Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    float angle, angleWithoutTrack;
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    angleWithoutTrack = as5600GetAngleWithoutTrack();
    angle = as5600GetAngle();
    HAL_Delay(100);
    if (recvDataToProcess)
    {
      recvDataToProcess = 0;
      printf("%s", RxBuffer);
      memset(RxBuffer, '\0', sizeof(RxBuffer));
    }
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV3;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);
  if (Button2_Pin == GPIO_Pin)
  {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
  }
  if (Button3_Pin == GPIO_Pin)
  {
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
  }
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_GPIO_EXTI_Callback could be implemented in the user file
   */
}
MotorMode motorMode;
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc) // 10kHz ADC
{
  static uint8_t cnt;
  static uint16_t obsever_cnt;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  if (hadc == &hadc1)
  {
    if (ADC_offset == 0)
    {
      cnt++;
      adc1_in1 = hadc1.Instance->JDR1;
      adc1_in2 = hadc2.Instance->JDR1;
      adc1_in3 = hadc1.Instance->JDR2;
      IA_Offset += adc1_in1;
      IB_Offset += adc1_in2;
      IC_Offset += adc1_in3;
      if (cnt >= 10)
      {
        ADC_offset = 1;
        IA_Offset = IA_Offset / 10;
        IB_Offset = IB_Offset / 10;
        IC_Offset = IC_Offset / 10;
      }
      motorMode = OPEN_LOOP;
    }
    else
    {
      adc1_in1 = hadc1.Instance->JDR1;
      adc1_in3 = hadc1.Instance->JDR2;
      adc1_in2 = hadc2.Instance->JDR1;
      Ia = (adc1_in1 - IA_Offset) * 0.02197f;
      Ib = (adc1_in2 - IB_Offset) * 0.02197f;
      Ic = (adc1_in3 - IC_Offset) * 0.02197f; // 0.02197265625
                                              // open loop
      HallTheta = HallTheta + HallThetaAdd;
      if (HallTheta < 0.0f)
      {
        HallTheta += 2.0f * PI;
      }
      else if (HallTheta > (2.0f * PI))
      {
        HallTheta -= 2.0f * PI;
      }

      // switch (motorMode)
      // {
      // case OPEN_LOOP:
      //   static uint openLoopCnt = 0;
      //   if (++openLoopCnt >= 65535)
      //   {
      //     motorMode = CLOSE_LOOP;
      //   }
      openSpeedLoop(3, 60);
      //   break;

      // case CLOSE_LOOP:
      //   closeSpeedLoop(HallSpeed, 500, HallTheta, Ia, Ib, Ic, 20000);
      //   break;
      // }
      // //
      temp[0] = Ia;
      temp[1] = Ib;
      memcpy(tempData, (uint8_t *)&temp, sizeof(temp));
      // HAL_UART_Transmit_DMA(&huart3, (uint8_t *)tempData, 6 * 4);
    }
  }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  if (Uart1_Rx_Cnt >= RXBUFFERSIZE - 1)
  {
    Uart1_Rx_Cnt = 0;
    memset(RxBuffer, 0x00, sizeof(RxBuffer));
    HAL_UART_Transmit(&huart3, (uint8_t *)"????", 10, 0xFFFF);
  }
  else
  {
    RxBuffer[Uart1_Rx_Cnt++] = aRxBuffer;
    if (aRxBuffer == '\n')
    {
      Uart1_Rx_Cnt = 0;
      recvDataToProcess = 1;
    }
    //		Uart1_Rx_Cnt = 0;
    //		memset(RxBuffer,0x00,sizeof(RxBuffer));
  }
  HAL_UART_Receive_IT(&huart3, (uint8_t *)&aRxBuffer, 1);
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_TxHalfCpltCallback can be implemented in the user file.
   */
}

int fputc(int ch, FILE *f)
{
  while ((USART3->ISR & 0X40) == 0)
    ;
  USART3->TDR = (uint8_t)ch;
  return ch;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{ /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);
  if (htim == &htim4)
  {
    HallTemp = HAL_TIM_ReadCapturedValue(&htim4, TIM_CHANNEL_1);
    HallThetaAdd = (PI / 3) / (HallTemp / 10000000) / 10000;
    HallSpeed = (PI / 3) / (HallTemp / 10000000) * 30 / (2 * PI);
    HallReadTemp = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8);
    HallReadTemp |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) << 1;
    HallReadTemp |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) << 2;
    if (HallReadTemp == 0x05)
    {
      HallTheta = 0.0f + PHASE_SHIFT_ANGLE;
    }
    else if (HallReadTemp == 0x04)
    {
      HallTheta = (PI / 3.0f) + PHASE_SHIFT_ANGLE;
    }
    else if (HallReadTemp == 0x06)
    {
      HallTheta = (PI * 2.0f / 3.0f) + PHASE_SHIFT_ANGLE;
    }
    else if (HallReadTemp == 0x02)
    {
      HallTheta = PI + PHASE_SHIFT_ANGLE;
    }
    else if (HallReadTemp == 0x03)
    {
      HallTheta = (PI * 4.0f / 3.0f) + PHASE_SHIFT_ANGLE;
    }
    else if (HallReadTemp == 0x01)
    {
      HallTheta = (PI * 5.0f / 3.0f) + PHASE_SHIFT_ANGLE;
    }
    if (HallTheta < 0.0f)
    {
      HallTheta += 2.0f * PI;
    }
    else if (HallTheta > (2.0f * PI))
    {
      HallTheta -= 2.0f * PI;
    }
  }

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_IC_CaptureCallback could be implemented in the user file
   */
}
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

#ifdef USE_FULL_ASSERT
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
