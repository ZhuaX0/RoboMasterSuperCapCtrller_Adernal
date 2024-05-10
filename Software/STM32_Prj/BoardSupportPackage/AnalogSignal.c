#include "AnalogSignal.h"
#include "adc.h"
#include "math.h"

AnalogDataTypology_t  AnalogDataTypology;
WindowFilter_HandleTypeDef WindowFilter[5];
uint16_t AnalogSignal_FilteredData[5];
uint16_t AnalogSignal_RealtimeData[5];
uint16_t AnalogSignal_CapData_RxBuffer[2][16][3];
uint16_t AnalogSignal_SRCData_RxBuffer[2][16][2];
AnalogUnitCnvt_HandleTypedef AnalogUnitCnvtParm[5];
uint8_t AnalogSignal_CaliReady=0U;
	
CCMRAM void AnalogSignal_ADCDMA_OVRStop(ADC_HandleTypeDef *hadc)
{
	hadc->Instance->CR |= 0x00000010;//ADC->CR->ADSTP write 1
}

CCMRAM void AnalogSignal_ADCDMA_OVRRecovery(ADC_HandleTypeDef *hadc)
{
	//清除ADC_OVR错误，使ADC转换转换准备阶段
	__HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_OVR);
	hadc->Instance->CFGR |= 0x00000001;//ADC->CFGR->DMAEN write 1
	
	//清除DMA传输错误，使DMA进入DMA申请接收准备状态
	__HAL_DMA_CLEAR_FLAG(hadc->DMA_Handle, DMA_FLAG_TE1);
	hadc->DMA_Handle->Instance->CCR |= 0x00000001;//DMA->CCRx->EN write 1
	
	//开启ADC
	while(__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_RDY) == 0UL)
	{
		;
	}
	hadc->Instance->CR |= 0x00000004;//ADC->CR->ADSTART write 1
}

void AnalogSignal_AnalogData_Init(uint8_t Icap_WinLength, uint8_t Icha_WinLength, uint8_t Vcap_WinLength,
																	uint8_t Isrc_WinLength, uint8_t Vbat_WinLength)
{
	//初始化滤波器
	AnalogSignal_WindowFilter_Init(&WindowFilter[0], Icap_WinLength);
	AnalogSignal_WindowFilter_Init(&WindowFilter[1], Icha_WinLength);
	AnalogSignal_WindowFilter_Init(&WindowFilter[2], Vcap_WinLength);
	AnalogSignal_WindowFilter_Init(&WindowFilter[3], Isrc_WinLength);
	AnalogSignal_WindowFilter_Init(&WindowFilter[4], Vbat_WinLength);

	//启动测温ADC
	HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);
	HAL_ADC_Start(&hadc3);
}

CCMRAM void AnalogSignal_AnalogData_Update(void)
{
	//判定ADC1&DMA1缓冲1 or 缓冲2应存入内存
	uint8_t Cap_BufferNbr;
	if(__HAL_DMA_GET_FLAG(hadc1.DMA_Handle, DMA_FLAG_HT1) == SET)
	{
		__HAL_DMA_CLEAR_FLAG(hadc1.DMA_Handle, DMA_FLAG_HT1);
		Cap_BufferNbr=0;
	}
	else
	{
		Cap_BufferNbr=1;
	}
	
	//判定ADC2&DMA2缓冲1 or 缓冲2应存入内存
	uint8_t Src_BufferNbr;
	if(__HAL_DMA_GET_FLAG(hadc2.DMA_Handle, DMA_FLAG_HT1) == SET)
	{
		__HAL_DMA_CLEAR_FLAG(hadc2.DMA_Handle, DMA_FLAG_HT1);
		Src_BufferNbr=0;
	}
	else
	{
		Src_BufferNbr=1;
	}
	
	uint8_t i,j;
	for(i=0; i<3; i++)//电容侧实时数据导出
	{
		uint16_t Sum=0;
		for(j=0; j<16; j++)
		{
			Sum += AnalogSignal_CapData_RxBuffer[Cap_BufferNbr][j][i];
		}
		AnalogSignal_RealtimeData[i] = (Sum>>4);
	}
	
	for(i=0; i<2; i++)//源极侧实时数据导出
	{
		uint16_t Sum=0;
		for(j=0; j<16; j++)
		{
			Sum += AnalogSignal_SRCData_RxBuffer[Src_BufferNbr][j][i];
		}
		AnalogSignal_RealtimeData[i+3] = (Sum>>4);
	}
	
	//窗口滤波后导出滤波后数据
	for(i=0; i<5; i++)
	{
		AnalogSignal_FilteredData[i] = AnalogSignal_WindowFilter_DataUpdate(&WindowFilter[i], AnalogSignal_RealtimeData[i]);
	}
}

void AnalogSignal_AnalogData_Calibration(void)
{
	//等待硬件上电后趋于稳定
	//等待FilteredWindow开启ADC后的接收数据填充
	//HAL_Delay(1);用HAL延时函数会导致系统卡死，考虑使用闹钟系统
	
	AnalogUnitCnvtParm[Icap].Gain = -102.958f;
	AnalogUnitCnvtParm[Icap].Offset = AnalogSignal_FilteredData[Icap];
	
	AnalogUnitCnvtParm[Ichassis].Gain = 375.034f;
	AnalogUnitCnvtParm[Ichassis].Offset = -38.7973f;
	
	AnalogUnitCnvtParm[Vcap].Gain = 113.236f;
	AnalogUnitCnvtParm[Vcap].Offset = -63.6101f;
	
	AnalogUnitCnvtParm[Isource].Gain = -102.958f;
	AnalogUnitCnvtParm[Isource].Offset = AnalogSignal_FilteredData[Isource];
	
	AnalogUnitCnvtParm[Vbat].Gain = 112.658f;
	AnalogUnitCnvtParm[Vbat].Offset = -55.3274f;
	
	AnalogSignal_CaliReady = 1U;
}

CCMRAM float AnalogSignal_GetTemp(void)
{
	float B = 3434.0f;
	float R25 = 10000.0f;
	
	float GetV = 0.001f * __HAL_ADC_CALC_DATA_TO_VOLTAGE(3300, HAL_ADC_GetValue(&hadc3), ADC_RESOLUTION_12B);
	float Rntc = (3.3f-GetV)/(GetV/10000.0f);
	
	return ((1.0f /
					((1.0f/B)*((float)log((double)(Rntc/R25))) + 1.0f/298.15f)) - 273.15f);
}

/*******************************************底层函数，用户禁止修改****************************************************/
void AnalogSignal_WindowFilter_Init(WindowFilter_HandleTypeDef *hWindowFilter, uint8_t SetLength)
{
	uint8_t i;
	for(i=0; i<SetLength; i++)
	{
		hWindowFilter->FilterWindow[i]=0;
	}
	
	hWindowFilter->Length=SetLength;
	hWindowFilter->Probe=0;
	hWindowFilter->Sum=0;
}

CCMRAM uint16_t AnalogSignal_WindowFilter_DataUpdate(WindowFilter_HandleTypeDef *hWindowFilter, uint16_t InputData)
{
	hWindowFilter->Sum -= hWindowFilter->FilterWindow[hWindowFilter->Probe];
	hWindowFilter->Sum += InputData;//更新sum值
	hWindowFilter->FilterWindow[hWindowFilter->Probe] = InputData;//更新窗口
	
	(hWindowFilter->Probe)++;
	if(hWindowFilter->Probe == hWindowFilter->Length)
	{
		hWindowFilter->Probe=0;
	}
	return (uint16_t)(hWindowFilter->Sum/hWindowFilter->Length);
}

