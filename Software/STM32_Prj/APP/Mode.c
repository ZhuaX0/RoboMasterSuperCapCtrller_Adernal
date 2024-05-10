#include "Mode.h"
#include "LoopCtrl.h"
#include "Display.h"
#include "Safety.h"

//环路控制限幅原始数据
LoopCtrl_LimitInit_Typedef Mode_OriginLoopLim;
//电容组类型
Mode_CapType_Typedef Mode_CapType_Enum;
uint8_t Mode_CapType = CapType1_24V;
float Mode_CapMaxV = 24.0f;
float Mode_CapMinV = 3.5f;
//Exceed
uint8_t Mode_Exceed = 0U;
//Pchassis期望
float Mode_ExpectPchassis = 35.0f;
//模式切换
uint8_t Mode_NowMode = Silent;
//参数记录
float Mode_IcapLimit = 10.0f;
float Mode_PsourceLimit = 205.0f;
//模式变化使能
uint8_t Mode_EN = 1U;

/**
  ************************************************************************** 
  ** @name          : Mode_Init
  ** @brief         : Initialization mode management
  ** @param         : void
  ** @retval        : void
  ************************************************************************** 
**/
void Mode_Init(void)
{
	LoopCtrl_StructInit();
	
	Mode_OriginLoopLim = LoopCtrl_LimitStructure;
	Mode_OriginLoopLim.Icap_IntegMax  = 24.0f/19.5f;
	Mode_OriginLoopLim.Icap_OutputMax = 24.0f/19.5f;
	Mode_OriginLoopLim.VcapHigh_IntegMax  = 24.0f/19.5f;
	Mode_OriginLoopLim.VcapHigh_IntegMin  = 24.0f/27.5f;
	Mode_OriginLoopLim.VcapHigh_OutputMax = 24.0f/19.5f;
	Mode_OriginLoopLim.VcapHigh_OutputMin = 24.0f/27.5f;
}

/**
  ************************************************************************** 
  ** @name          : Mode_CapType_Set
  ** @brief         : Set the capacitor bank type to be called only once per power-up in the RoboMaster application.
  ** @param         : CapType,The passed parameter is of type Mode_CapType_Typedef.
  ** @retval        : void
  ************************************************************************** 
**/
void Mode_CapType_Set(uint8_t CapType)
{
	if(Mode_NowMode == Silent)
	{
		switch(CapType)
		{
			case CapType3_30V:
			{						
				Safety_VcapParamSet(30.3f);
				Mode_CapType = CapType3_30V;
				Mode_CapMaxV = 30.0f;
			}
			break;
			case CapType2_28V:
			{
				Safety_VcapParamSet(28.3f);
				Mode_CapType = CapType2_28V;
				Mode_CapMaxV = 28.0f;
			}
			break;
			case CapType1_24V:
			{
				Safety_VcapParamSet(24.3f);
				Mode_CapType = CapType1_24V;
				Mode_CapMaxV = 24.0f;
			}
			default:;
		}
		
		Mode_OriginLoopLim.Icap_IntegMax  = Mode_CapMaxV/19.5f;
		Mode_OriginLoopLim.Icap_OutputMax = Mode_CapMaxV/19.5f;
		Mode_OriginLoopLim.VcapHigh_IntegMax  = Mode_CapMaxV/19.5f;
		Mode_OriginLoopLim.VcapHigh_IntegMin  = Mode_CapMaxV/27.5f;
		Mode_OriginLoopLim.VcapHigh_OutputMax = Mode_CapMaxV/19.5f;
		Mode_OriginLoopLim.VcapHigh_OutputMin = Mode_CapMaxV/27.5f;
	}
}

/**
  ************************************************************************** 
  ** @name          : Mode_ExpectPchassis_Set
  ** @brief         : Setting the Pchassis desired power to be invoked during DCDC enable does not affect DCDC operation.
  ** @param         : ExpectPchassis,Pchassis expectations to be changed.
  ** @retval        : void
  ************************************************************************** 
**/
CCMRAM void Mode_ExpectPchassis_Set(float ExpectPchassis)
{
	Mode_ExpectPchassis = ExpectPchassis;
	
	if(Mode_NowMode != Silent)
	{
		LoopCtrl_ExpectPchassisSet(ExpectPchassis);
	}
}

/**
  ************************************************************************** 
  ** @name          : Mode_Exceed_Set
  ** @brief         : Enable or disable Exceed.Calling while the DCDC is active restarts the DCDC.
  ** @param         : ExceedEN,0x00 means disable,0x01 means enable.
  ** @retval        : void
  ************************************************************************** 
**/
CCMRAM void Mode_Exceed_Set(uint8_t ExceedEN)
{
	Mode_Exceed = ExceedEN;
	
	if(ExceedEN == 1)
	{
		Saftey_CurrentParamSet(14.0f, 17.5f, 14.0f, 17.5f);
	}
	else
	{
		Saftey_CurrentParamSet(10.5f, 14.0f, 10.5f, 14.0f);
	}
	
	switch(Mode_NowMode)
	{
		case Charge:
		{
			Mode_Charge();
		}
		break;
		case Work:
		{
			Mode_Work();
		}
		break;
		case Silent:
		default:;
	}
	
	Display_Exceed_DataUpdateInterface(ExceedEN);
}

/**
  ************************************************************************** 
  ** @name          : Mode_Silent
  ** @brief         : Enter Silent working mode.
  ** @param         : void
  ** @retval        : void
  ************************************************************************** 
**/
CCMRAM void Mode_Silent(void)
{
	if(Mode_EN == 1U)
	{
		if(Mode_NowMode != Silent)
		{
			LoopCtrl_Stop();
			
			Mode_NowMode = Silent;
			Display_Mode_DataUpdateInterface(Silent);
		}
	}
}

/**
  ************************************************************************** 
  ** @name          : Mode_Work
  ** @brief         : Enter Work working mode.Calling while the DCDC is active causes the DCDC to restart.
  ** @param         : void
  ** @retval        : void
  ************************************************************************** 
**/
CCMRAM void Mode_Work(void)
{
	if(Mode_EN == 1U)
	{
		//暂停环路与DCDC工作
		if(Mode_NowMode != Silent)
		{
			LoopCtrl_Stop();
		}
		
		LoopCtrl_LimitInit_Typedef WorkMode_LoopLimit = Mode_OriginLoopLim;
		
		//考虑Exceed对积分限幅带来的变化
		switch(Mode_Exceed)
		{
			case 0x01://待调试
			{
				WorkMode_LoopLimit.Psource_ffPID_OutputMax  = 13.5f;
				WorkMode_LoopLimit.Psource_ffPID_OutputMin  = -13.5f;
				WorkMode_LoopLimit.Pchassis_IntegMin  = -205.0f;
				WorkMode_LoopLimit.Pchassis_OutputMin = -205.0f;
			}
			break;
			case 0x00:
			default:;
		}
		Mode_PsourceLimit = (WorkMode_LoopLimit.Pchassis_OutputMin*(-1.0f));
		Mode_IcapLimit = (WorkMode_LoopLimit.Psource_ffPID_OutputMax > (WorkMode_LoopLimit.Psource_ffPID_OutputMin*(-1.0f)) ?
											WorkMode_LoopLimit.Psource_ffPID_OutputMax : (WorkMode_LoopLimit.Psource_ffPID_OutputMin*(-1.0f)));
		
		//重启环路
		LoopCtrl_Start(&LoopCtrl_KpKiKdStructure, &WorkMode_LoopLimit,
									 Mode_ExpectPchassis, Mode_CapMaxV, Mode_CapMinV);
		Mode_NowMode = Work;
		Display_Mode_DataUpdateInterface(Work);
	}
}

/**
  ************************************************************************** 
  ** @name          : Mode_Charge
  ** @brief         : Enter Charge working mode.Calling while the DCDC is active causes the DCDC to restart.
  ** @param         : void
  ** @retval        : void
  ************************************************************************** 
**/
CCMRAM void Mode_Charge(void)
{
	if(Mode_EN == 1U)
	{
		//暂停环路与DCDC工作
		if(Mode_NowMode != Silent)
		{
			LoopCtrl_Stop();
		}
		
		LoopCtrl_LimitInit_Typedef ChargeMode_LoopLimit = Mode_OriginLoopLim;
		
		//考虑Exceed对积分限幅带来的变化
		switch(Mode_Exceed)
		{
			case 0x01://待调试
			{
				ChargeMode_LoopLimit.Psource_ffPID_OutputMax  = 13.5f;
				ChargeMode_LoopLimit.Psource_ffPID_OutputMin  = 0.0f;
				ChargeMode_LoopLimit.Pchassis_IntegMin  = 0.0f;
				ChargeMode_LoopLimit.Pchassis_OutputMin = 0.0f;
			}
			case 0x00:
			default:
			{
				ChargeMode_LoopLimit.Psource_ffPID_OutputMax  = 10.0f;
				ChargeMode_LoopLimit.Psource_ffPID_OutputMin  = 0.0f;
				ChargeMode_LoopLimit.Pchassis_IntegMin  = 0.0f;
				ChargeMode_LoopLimit.Pchassis_OutputMin = 0.0f;
			}
		}
		Mode_PsourceLimit = 200.0f;
		Mode_IcapLimit = ChargeMode_LoopLimit.Psource_ffPID_OutputMax;
		
		//重启环路
		LoopCtrl_Start(&LoopCtrl_KpKiKdStructure, &ChargeMode_LoopLimit,
									 Mode_ExpectPchassis, Mode_CapMaxV, Mode_CapMinV);
		Mode_NowMode = Charge;
		Display_Mode_DataUpdateInterface(Charge);	
	}
}

