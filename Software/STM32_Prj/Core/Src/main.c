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
#include "fdcan.h"
#include "hrtim.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "iwdg.h"

//Board Supply Package
#include "AnalogSignal.h"
#include "VOFAplus_DataUpload.h"
#include "led.h"

//APP
#include "AlarmSystem.h"
#include "Display.h"
#include "LoopCtrl.h"
#include "Mode.h"
#include "UsrMsg.h"
#include "Safety.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
float Temp = 0.0;//temp
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
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_HRTIM1_Init();
  MX_TIM6_Init();
  MX_USART1_UART_Init();
  MX_FDCAN1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	//OLED清屏、初始化、开机界面
	OLED_Clear();
	LED_Init();
	Display_BootUpInterface();
	
	//闹钟系统初始化
	AlarmSystem_Init();
	
	//安全管理初始化
	Safety_Init();

	//模拟信号初始化
	AnalogSignal_AnalogData_Init(15, 32, 64, 15, 64);
	
	//HRTIM&ADC启动
//	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
//	HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)(&AnalogSignal_CapData_RxBuffer[0][0][0]), 96);
	HAL_ADC_Start_DMA(&hadc2, (uint32_t *)(&AnalogSignal_SRCData_RxBuffer[0][0][0]), 64);
	HAL_HRTIM_WaveformCountStart_IT(&hhrtim1, HRTIM_TIMERID_MASTER|HRTIM_TIMERID_TIMER_A|HRTIM_TIMERID_TIMER_B);
	
	//等待填满缓冲&校准模拟信号数据
	uint8_t InitDelay = AlarmSystem_ChannelActive(50, 0);
	while(AlarmSystem_GetBellRing(&InitDelay) == Clocking);
	AnalogSignal_AnalogData_Calibration();
	Safety_INA240cali_ItemCheck(AnalogSignal_FilteredData[Isource], AnalogSignal_FilteredData[Icap]);
	
	//开机界面时间尺度上可视(帅)，等待随逐周期控制的安全检查给出稳定评估结果
	InitDelay = AlarmSystem_ChannelActive(450, 0);
	while(AlarmSystem_GetBellRing(&InitDelay) == Clocking);
	OLED_Clear();
	InitDelay = AlarmSystem_ChannelActive(500, 0);
	Display_LogoInterface();
	while(AlarmSystem_GetBellRing(&InitDelay) == Clocking);
	
	//模式管理初始化
	Mode_Init();
	
	//用户通讯初始化
	UsrMsg_Init();
	
	//OLED清屏、准备界面
	OLED_Clear();
	Display_PrepareInterface();
	Display_Mode_DataUpdateInterface(Silent);
	Display_Exceed_DataUpdateInterface(0x00);
	
	//bxCAN(classic)启动
	MX_FDCAN_Start();
	UsrMsg_StdByFrame_Transmit(&hfdcan1, 0xFF);
	
	//任务时基
	uint8_t TempCheck_NoBlockDelay = AlarmSystem_ChannelActive(10, 1U);
	uint8_t FbFrameTx_NoBlockDelay = AlarmSystem_ChannelActive(500, 1U);
	uint8_t HeartBeatLED_Delay = AlarmSystem_ChannelActive(250, 1U);
	uint8_t OLED_Update_NoBlockDelay = AlarmSystem_ChannelActive(500, 1U);
	
	//看门狗使能
	MX_IWDG_Init();
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//安全检查
		HAL_IWDG_Refresh(&hiwdg);
		if(AlarmSystem_GetBellRing(&TempCheck_NoBlockDelay) == Ring)
		{
			Safety_Temp_ItemCheck(AnalogSignal_GetTemp());
		}
		
		//决策
		if(UsrMsg_InitData.UpdateTrigger == 1U)//更新电容组类型
		{
			UsrMsg_InitData.UpdateTrigger = 0U;
			
			Mode_CapType_Set(UsrMsg_InitData.CapTypeGet);
		}
		
		if(UsrMsg_CtrlData.Pchassis_UpdateTrigger == 1U)//更新裁判限制功率
		{
			UsrMsg_CtrlData.Pchassis_UpdateTrigger = 0U;
			
			Mode_ExpectPchassis_Set(UsrMsg_CtrlData.PchassisGet_f);
		}
		if(UsrMsg_CtrlData.Exceed_UpdateTrigger == 1U)//更新Exceed开启状态
		{
			UsrMsg_CtrlData.Exceed_UpdateTrigger = 0U;
			
			Mode_Exceed_Set(UsrMsg_CtrlData.ExceedGet);
		}
		if(UsrMsg_CtrlData.Mode_UpdateTrigger == 1U)//更新工作模式
		{
			UsrMsg_CtrlData.Mode_UpdateTrigger = 0U;
			
			switch(UsrMsg_CtrlData.ModeGet)
			{
				case 0x00:
				{
					Mode_Silent();
				}
				break;
				case 0x01:
				{
					Mode_Work();
				}
				break;
				case 0x02:
				{
					Mode_Charge();
				}
				break;
				default:;
			}
		}
		
		//上位机反馈
		if(AlarmSystem_GetBellRing(&FbFrameTx_NoBlockDelay) == Ring)
		{
			float WorkIntensity1 = __ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Icap], Icap) / 10.0f;
			WorkIntensity1 = (WorkIntensity1>=0 ? WorkIntensity1 : (WorkIntensity1*(-1.0f)));
			float WorkIntensity2 = __ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vbat], Vbat) * __ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Isource], Isource) / 205.0f;
			WorkIntensity2 = (WorkIntensity2>=0 ? WorkIntensity2 : (WorkIntensity2*(-1.0f)));
			
			UsrMsg_FbFrame_Transmit(&hfdcan1, 
															__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vcap], Vcap)-__CAP_ESR*__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Icap], Icap),
															WorkIntensity1,
															WorkIntensity2,
															__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vbat], Vbat) * __ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Ichassis], Ichassis));
		}
		
		//心跳灯
		if(AlarmSystem_GetBellRing(&HeartBeatLED_Delay)==Ring)
		{
			HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
		}
		
		//OLED显示数据更新任务
		if(AlarmSystem_GetBellRing(&OLED_Update_NoBlockDelay)==Ring)
		{
			Display_Vcap_DataUpdateInterface(__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vcap], Vcap));
		}
		if(Safety_ItemLevel_ChangeTrigger == 1U)
		{
			Safety_ItemLevel_ChangeTrigger = 0U;
			
			Display_SafetyCode_DataUpdateInterface(Safety_ItemLevel);
			UsrMsg_SafeFrame_Transmit(&hfdcan1, Safety_ItemLevel);
		}
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV5;
  RCC_OscInitStruct.PLL.PLLN = 68;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
	AnalogSignal_ADCDMA_OVRStop(hadc);
	Safety_DMAden_ItemCheck();
}

void HAL_HRTIM_RepetitionEventCallback(HRTIM_HandleTypeDef * hhrtim, uint32_t TimerIdx)
{
	//监测ADC是否发生溢出事件，若溢出重新恢复ADC转换与DMA传输
	//并跳过这REP周期的模拟信号数据处理与HRPWM调制
	uint8_t ADC_OVR_Flag=0;
	if(__HAL_ADC_GET_FLAG(&hadc1, ADC_FLAG_OVR)==1UL)
	{
		AnalogSignal_ADCDMA_OVRRecovery(&hadc1);
		ADC_OVR_Flag=1;
	}
	if(__HAL_ADC_GET_FLAG(&hadc2, ADC_FLAG_OVR)==1UL)
	{
		AnalogSignal_ADCDMA_OVRRecovery(&hadc2);
		ADC_OVR_Flag=1;
	}
	
	//模拟信号处理&环路控制
	if(ADC_OVR_Flag==0)
	{
		AnalogSignal_AnalogData_Update();
		if(AnalogSignal_CaliReady==1U)
		{
			Safety_Voltage_ItemCheck(__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vbat], Vbat),
															 __ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vcap], Vcap));
			Safety_Current_ItemCheck(__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Isource], Isource),
															 __ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Icap], Icap));
			Safety_deltaP_ItemCheck(__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vbat], Vbat),
															__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Isource], Isource),
															__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vcap], Vcap),
															__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Icap], Icap));
		}
		LoopCtrl_Main(
			__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Icap], Icap),
			__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Ichassis], Ichassis),
			__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vcap], Vcap),
			__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Isource], Isource),
			__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vbat], Vbat));
	}
	
	//向上位机回传数据
	float AnalogData_Upload[2];
	
//	AnalogData_Upload[0] = __ANALOGSIGNAL_DTS_CNVT(AnalogSignal_RealtimeData[Icap], Icap);
//	AnalogData_Upload[1] = __ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Icap], Icap);
	
	AnalogData_Upload[0] = __ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vbat], Vbat)*__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Ichassis], Ichassis);
	AnalogData_Upload[1] = __ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vcap], Vcap);

	VOFA_DataJustFloat_Upload(AnalogData_Upload, 2);
}

uint8_t RxFifo0NewMsg_InitLock = 0U;
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if(hfdcan==&hfdcan1
	&& RxFifo0ITs==FDCAN_IT_RX_FIFO0_NEW_MESSAGE)
	{
		FDCAN_RxHeaderTypeDef FDCAN_RxHeader;
		uint8_t FDCAN_RxData[8];
		HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &FDCAN_RxHeader, FDCAN_RxData);
		
		switch(FDCAN_RxHeader.Identifier)
		{
			case UsrMsgRx_CtrlID:
			{
				UsrMsg_CtrlFrame_Unpack(FDCAN_RxData);
			}
			break;
			case UsrMsgRx_InitID:
			{
				if(RxFifo0NewMsg_InitLock == 0U)
				{
					RxFifo0NewMsg_InitLock = 1U;
					UsrMsg_InitFrame_Unpack(FDCAN_RxData);
				}
			}
			break;
			default:;
		}
	}
}

void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
	Safety_bxCAN_ItemCheck();
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
	Mode_Silent();
	uint64_t FWEr_Counter = *((uint64_t *)(0x0807F800));
	if(FWEr_Counter<3)
	{
		Safety_FlashWrite64(0x0807F800, FWEr_Counter+1);
	}
	
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
