#include "DCDC.h"
#include "hrtim.h"

/**
  ************************************************************************** 
  ** @name          : DCDC_WorkStart
  ** @brief         : Switch on the PWM output of the DCDC.
  ** @param         : DutyRatioInit,Ratio of the duty cycle of the two half-bridges.
  ** @retval        : HAL_StatusTypeDef,The result of HAL_HRTIM_WaveformOutputStart
  ************************************************************************** 
**/
HAL_StatusTypeDef DCDC_WorkStart(float DutyRatioInit)
{
	//初始化两个半桥占空比
	DCDC_PWM_DutyChange(DutyRatioInit);
	
	//使能栅极驱动器
	HAL_GPIO_WritePin(SourceDriver_Enable_GPIO_Port, SourceDriver_Enable_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CapDriver_Enable_GPIO_Port, CapDriver_Enable_Pin, GPIO_PIN_RESET);
	
	//输出PWM
	return HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1|HRTIM_OUTPUT_TA2|
																		             HRTIM_OUTPUT_TB1|HRTIM_OUTPUT_TB2);
}

/**
  ************************************************************************** 
  ** @name          : DCDC_WorkStop
  ** @brief         : Turn off the PWM output of the DCDC.
  ** @param         : void.
  ** @retval        : HAL_StatusTypeDef,The result of HAL_HRTIM_WaveformOutputStop
  ************************************************************************** 
**/
CCMRAM HAL_StatusTypeDef DCDC_WorkStop(void)
{
	//停止输出PWM
	HAL_StatusTypeDef ReturnValue = HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1|HRTIM_OUTPUT_TA2|
																															 HRTIM_OUTPUT_TB1|HRTIM_OUTPUT_TB2);

	//失能栅极驱动器
	SourceDriver_Enable_GPIO_Port->BSRR = SourceDriver_Enable_Pin;
	CapDriver_Enable_GPIO_Port->BSRR = CapDriver_Enable_Pin;
	
	return ReturnValue;
}

/**
  ************************************************************************** 
  ** @name          : DCDC_PWM_DutyChange
  ** @brief         : Output PWM duty cycle via duty cycle ratio.
  ** @param         : DutyRatio,Ratio of the duty cycle of the two half-bridges.
  ** @retval        : void
  ************************************************************************** 
**/
CCMRAM void DCDC_PWM_DutyChange(float DutyRatio)
{
	//两个半桥占空比生成
	float Dsource,Dcap;
	if(DutyRatio>0.0f && DutyRatio<=1.0f)
	{
		Dcap = 0.9f;
		Dsource  = DutyRatio * 0.9f;
	}
	else if(DutyRatio > 1.0f)
	{
		Dsource  = 0.9f;
		Dcap = 0.9f / DutyRatio;
	}
	
	//CAP_Driver TA
	uint16_t TA_CmpOffset = (uint16_t)(34000*Dcap);
	uint16_t TA_CmpHalfOffset = (uint16_t)(TA_CmpOffset / 2);
	__HAL_HRTIM_SETCOMPARE(&hhrtim1, 0x0, HRTIM_COMPAREUNIT_1, 17000-TA_CmpHalfOffset-(TA_CmpOffset%2));
	__HAL_HRTIM_SETCOMPARE(&hhrtim1, 0x0, HRTIM_COMPAREUNIT_2, 17000+TA_CmpHalfOffset);
	//SRC_Driver TB
	uint16_t TB_CmpOffset = (uint16_t)(34000*Dsource);
	uint16_t TB_CmpHalfOffset = (uint16_t)(TB_CmpOffset / 2);
	__HAL_HRTIM_SETCOMPARE(&hhrtim1, 0x1, HRTIM_COMPAREUNIT_1, 17000-TB_CmpHalfOffset-(TB_CmpOffset%2));
	__HAL_HRTIM_SETCOMPARE(&hhrtim1, 0x1, HRTIM_COMPAREUNIT_2, 17000+TB_CmpHalfOffset);
}

