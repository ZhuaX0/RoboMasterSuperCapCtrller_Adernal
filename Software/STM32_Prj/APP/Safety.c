#include "Safety.h"
#include "Mode.h"
#include "UsrMsg.h"
#include "fdcan.h"

Safety_DetectItemTypedef Safety_DetectItem;
Safety_LevelTypedef Safety_Level;

uint8_t Safety_ItemLevel[8] = {Safe};
uint8_t Safety_ItemLevel_ChangeTrigger=1U;

float Safety_Vcap_MaxLim=24.3f;
float Safety_Isource_AlertLim=10.5f,Safety_Isource_ForceLim=14.0f;
float Safety_Icap_AlertLim=10.5f,Safety_Icap_ForceLim=14.0f;

/**
  ************************************************************************** 
  ** @name          : Safety_Init
  ** @brief         : Initialization Safety management
  ** @param         : void
  ** @retval        : void
  ************************************************************************** 
**/
void Safety_Init(void)
{
	//检查不可逆损伤
	uint8_t i;
	for(i=0; i<8; i++)
	{
		if( *((uint64_t *)(0x0807F000+8*i)) == 0xAAAAAAAAAAAAAAAA )
		{
			Mode_EN = 0U;
			Safety_ItemLevelChange(i, Deadliness);
		}
	}
	
	//判断"固件错误"计数值
	Safety_Firmware_ItemCheck();
	
	//设定初始安全参数
	Safety_VcapParamSet(24.3f);
	Saftey_CurrentParamSet(10.5f, 14.0f, 10.5f, 14.0f);
}

/**
  ************************************************************************** 
  ** @name          : Safety_Firmware_ItemCheck
  ** @brief         : Specific implementation of the Firmware_Error security check.Called only once per run during the initial phase.
  ** @param         : void.
  ** @retval        : void.
  ************************************************************************** 
**/
void Safety_Firmware_ItemCheck(void)
{
	uint64_t FWEr_Counter = *((uint64_t *)(0x0807F800));
	
	if(FWEr_Counter == 0xFFFFFFFFFFFFFFFF)
	{
		Safety_FlashWrite64(0x0807F800, 0x0000000000000000);
	}
	else if(FWEr_Counter>0 && FWEr_Counter<3)
	{
		if(__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST) == 1U)
		{
			Safety_FlashWrite64(0x0807F800, FWEr_Counter-1);
		}
		Safety_ItemLevelChange(Firmware_Error, Warning);
	}
	else if(FWEr_Counter==3 && Safety_ItemLevel[Firmware_Error]!=Deadliness)
	{
		Mode_EN = 0U;
		Safety_ItemLevelChange(Firmware_Error, Deadliness);
		Safety_FlashErrorRecord(Firmware_Error);
	}
}

/**
  ************************************************************************** 
  ** @name          : Safety_bxCAN_ItemCheck
  ** @brief         : Called on bxCAN ErrorState interrupt.
  ** @param         : void.
  ** @retval        : void.
  ************************************************************************** 
**/
CCMRAM void Safety_bxCAN_ItemCheck(void)
{
	FDCAN_ProtocolStatusTypeDef FDCAN_ProtocolStatus;
	HAL_FDCAN_GetProtocolStatus(&hfdcan1, &FDCAN_ProtocolStatus);
	
	if(FDCAN_ProtocolStatus.ErrorPassive==0
	&& FDCAN_ProtocolStatus.Warning==0
	&& FDCAN_ProtocolStatus.BusOff==0)
	{
		Safety_ItemLevelChange(bxCAN_Error, Safe);
		Safety_TryUserCtrlMode();
	}
	else if(Safety_ItemLevel[bxCAN_Error] != Risk)
	{
		__disable_irq();
		Mode_Silent();
		Mode_EN = 0U;
		__enable_irq();
		Safety_ItemLevelChange(bxCAN_Error, Risk);
	}
	
	if(FDCAN_ProtocolStatus.BusOff==1)
	{
		HAL_FDCAN_Start(&hfdcan1);
	}
}

/**
  ************************************************************************** 
  ** @name          : Safety_Temp_ItemCheck
  ** @brief         : Specific implementation of the security check for OverTemp.Timed loop call(10ms).
  ** @param         : Temp_S,current temperature of PCBA.
  ** @retval        : void.
  ************************************************************************** 
**/
uint8_t Temp_AlertCnt=0;
uint8_t Temp_ForceCnt=0;
CCMRAM void Safety_Temp_ItemCheck(float Temp_S)
{
	if(Safety_ItemLevel[Temp_Error]!=Deadliness)
	{
		//更新Alert计数器
		if(Temp_S<70.0f && Temp_AlertCnt>0)
		{
			Temp_AlertCnt--;
		}
		else if(Temp_S>=70.0f && Temp_S<80.0f && Temp_AlertCnt<50)
		{
			Temp_AlertCnt++;
		}
		else if(Temp_S>=80.0f)
		{
			Temp_AlertCnt = 50;
		}
		
		//根据Alert计数值采取反应
		if(Temp_AlertCnt==0 && Safety_ItemLevel[Temp_Error]!=Safe)
		{
			Safety_ItemLevelChange(Temp_Error, Safe);
			Safety_TryUserCtrlMode();
		}
		else if(Temp_AlertCnt>0 && Temp_AlertCnt<50 && Safety_ItemLevel[Temp_Error]<Warning)
		{
			Safety_ItemLevelChange(Temp_Error, Warning);
		}
		else if(Temp_AlertCnt==50 && Safety_ItemLevel[Temp_Error]<Risk)
		{
			__disable_irq();
			Mode_Silent();
			Mode_EN = 0U;
			__enable_irq();
			Safety_ItemLevelChange(Temp_Error, Risk);
		}
		
		//更新Force计数器
		if(Safety_ItemLevel[Temp_Error]==Risk && Temp_ForceCnt<100)
		{
			Temp_ForceCnt++;
		}
		else if(Safety_ItemLevel[Temp_Error]<Risk && Temp_ForceCnt>0)
		{
			Temp_ForceCnt--;
		}
		
		//根据Force计数值采取反应
		if(Temp_ForceCnt==100)
		{
			Safety_ItemLevelChange(Temp_Error, Deadliness);
			Safety_FlashErrorRecord(Temp_Error);
		}
	}
}

/**
  ************************************************************************** 
  ** @name          : Safety_INA240cali_ItemCheck
  ** @brief         : Specific implementation of the security check for INA240 zero point calibration.Called only once per run during the initial phase.
  ** @param         : Source_0point,Isource Sense Port ADC Value as Zero Point.
  ** @param         : Cap_0point,Icap Sense Port ADC Value as Zero Point.
  ** @retval        : void.
  ************************************************************************** 
**/
void Safety_INA240cali_ItemCheck(float Source_0point, float Cap_0point)
{
	uint16_t Offset = 200;
	
	if((Source_0point>(2048+Offset) || Source_0point<(2048-Offset))
	|| (Cap_0point>(2048+Offset) || Cap_0point<(2048-Offset)))
	{
		Mode_EN = 0U;
		Safety_ItemLevelChange(INA240cali_Error, Danger);
	}
}

/**
  ************************************************************************** 
  ** @name          : Safety_VcapParamSet
  ** @brief         : Setting the maximum Vcap value for triggering safety measures.
  ** @param         : VcapMax,Maximum Vcap value for triggering safety measures.
  ** @retval        : void.
  ************************************************************************** 
**/
CCMRAM void Safety_VcapParamSet(float VcapMax)
{
	Safety_Vcap_MaxLim = VcapMax;
}

/**
  ************************************************************************** 
  ** @name          : Safety_Voltage_ItemCheck
  ** @brief         : Specific implementation of the security check for voltage of battery and SuperCap.Called immediately after obtaining the latest analogue value.
  ** @param         : Vbat_S,Current supply voltage (in international units).
  ** @param         : Vcap_S,Current capacitor bank voltage (in international units).
  ** @retval        : void.
  ************************************************************************** 
**/
uint16_t Vbat_Cnt=0;
uint8_t Vbat_Level=Safe;
uint16_t Vcap_Cnt=0;
uint8_t Vcap_Level=Safe;
CCMRAM void Safety_Voltage_ItemCheck(float Vbat_S, float Vcap_S)
{
	//更新Vbat_Cnt
	if(Vbat_S>27.5f || Vbat_S<19.5f)
	{
		Vbat_Cnt = 1000;
	}
	else if(Vbat_Cnt>0)
	{
		Vbat_Cnt--;
	}
	//分析Vbat_Cnt
	if(Vbat_Cnt==0 && Vbat_Level!=Safe)
	{
		Vbat_Level = Safe;
	}
	else if(Vbat_Cnt>0 && Vbat_Level!=Risk)
	{
		Vbat_Level = Risk;
	}
	
	//更新Vcap_Cnt
	if(Vcap_S>Safety_Vcap_MaxLim || Vcap_S<3.2f)
	{
		if(Vcap_Cnt<650)
		{
			Vcap_Cnt++;
		}
	}
	else if(Vcap_Cnt>0)
	{
		Vcap_Cnt--;
	}
	//分析Vcap_Cnt
	if(Vcap_Cnt==0 && Vcap_Level!=Safe)
	{
		Vcap_Level = Safe;
	}
	else if(Vcap_Cnt==650 && Vcap_Level != Risk)
	{
		Vcap_Level = Risk;
	}
	
	//融合
	uint8_t OutputLevel = (Vbat_Level>Vcap_Level ? Vbat_Level : Vcap_Level);
	if(OutputLevel==Safe && Safety_ItemLevel[Voltage_Error]!=Safe)
	{
		Safety_ItemLevelChange(Voltage_Error, Safe);
		Safety_TryUserCtrlMode();
	}
	else if(OutputLevel==Risk && Safety_ItemLevel[Voltage_Error]!=Risk)
	{
		__disable_irq();
		Mode_Silent();
		Mode_EN = 0U;
		__enable_irq();
		Safety_ItemLevelChange(Voltage_Error, Risk);
	}
}

/**
  ************************************************************************** 
  ** @name          : Saftey_CurrentParamSet
  ** @brief         : Setting the Current parameter for triggering safety measures.
  ** @param         : Isource_AlertLim,Isource's alert value(in international units).
  ** @param         : Isource_ForceLim,Isource's force value(in international units).
  ** @param         : Icap_AlertLim,Icap's alert value(in international units).
  ** @param         : Icap_ForceLim,Icap's force value(in international units).
  ** @retval        : void.
  ************************************************************************** 
**/
CCMRAM void Saftey_CurrentParamSet(float Isource_AlertLim, float Isource_ForceLim,
																	 float Icap_AlertLim,    float Icap_ForceLim)
{
	Safety_Isource_AlertLim = Isource_AlertLim;
	Safety_Isource_ForceLim = Isource_ForceLim;
	Safety_Icap_AlertLim = Icap_AlertLim;
	Safety_Icap_ForceLim = Icap_ForceLim;
}

/**
  ************************************************************************** 
  ** @name          : Safety_Current_ItemCheck
  ** @brief         : Specific implementation of the security check for current of source and cap net.Called immediately after obtaining the latest analogue value.
  ** @param         : Isource_S,Current source current(in international units).
  ** @param         : Icap_S,Current capacitor bank current(in international units).
  ** @retval        : void.
  ************************************************************************** 
**/
uint8_t Isource_Level = Safe;
uint16_t Isource_AlertCnt=0,Isource_ForceCnt=0;
uint8_t Icap_Level = Safe;
uint16_t Icap_AlertCnt=0,Icap_ForceCnt=0;
CCMRAM void Safety_Current_ItemCheck(float Isource_S, float Icap_S)
{
	if(Safety_ItemLevel[Current_Error] != Deadliness)
	{
		float GetIsource = (Isource_S>=0.0f ? Isource_S : ((-1.0f)*Isource_S));
		float GetIcap    = (Icap_S>=0.0f    ? Icap_S    : ((-1.0f)*Icap_S));
		
		//更新Isource_AlertCnt
		if(GetIsource<Safety_Isource_AlertLim && Isource_AlertCnt>0)
		{
			Isource_AlertCnt--;
		}
		else if(GetIsource>=Safety_Isource_AlertLim && GetIsource<Safety_Isource_ForceLim && Isource_AlertCnt<100)
		{
			Isource_AlertCnt++;
		}
		else if(GetIsource>=Safety_Isource_ForceLim)
		{
			Isource_AlertCnt=100;
		}
		
		//分析Isource_AlertCnt
		if(Isource_AlertCnt==0 && Isource_Level!=Safe)
		{
			Isource_Level = Safe;
		}
		else if(Isource_AlertCnt>0 && Isource_AlertCnt<100 && Isource_Level<Warning)
		{
			Isource_Level = Warning;
		}
		else if(Isource_AlertCnt==100 && Isource_Level<Risk)
		{
			Isource_Level = Risk;
		}
		
		//更新Isource_ForceCnt
		if(Isource_Level==Risk && Isource_ForceCnt<150)
		{
			Isource_ForceCnt++;
		}
		else if(Isource_Level<Risk && Isource_ForceCnt>0)
		{
			Isource_ForceCnt--;
		}
		
		//分析Isource_ForceCnt
		if(Isource_ForceCnt==150)
		{
			Isource_Level = Deadliness;
		}
		
		//更新Icap_AlertCnt
		if(GetIcap<Safety_Icap_AlertLim && Icap_AlertCnt>0)
		{
			Icap_AlertCnt--;
		}
		else if(GetIcap>=Safety_Icap_AlertLim && GetIcap<Safety_Icap_ForceLim && Icap_AlertCnt<100)
		{
			Icap_AlertCnt++;
		}
		else if(GetIcap>=Safety_Icap_ForceLim)
		{
			Icap_AlertCnt=100;
		}
		
		//分析Icap_AlertCnt
		if(Icap_AlertCnt==0 && Icap_Level!=Safe)
		{
			Icap_Level = Safe;
		}
		else if(Icap_AlertCnt>0 && Icap_AlertCnt<100 && Icap_Level<Warning)
		{
			Icap_Level = Warning;
		}
		else if(Icap_AlertCnt==100 && Icap_Level<Risk)
		{
			Icap_Level = Risk;
		}
		
		//更新Icap_ForceCnt
		if(Icap_Level==Risk && Icap_ForceCnt<150)
		{
			Icap_ForceCnt++;
		}
		else if(Icap_Level<Risk && Icap_ForceCnt>0)
		{
			Icap_ForceCnt--;
		}
		
		//分析Icap_ForceCnt
		if(Icap_ForceCnt==150)
		{
			Icap_Level = Deadliness;
		}
		
		//融合
		uint8_t OutputLevel = (Isource_Level>=Icap_Level ? Isource_Level : Icap_Level);
		if(Safety_ItemLevel[Current_Error] != OutputLevel)
		{
			switch(Safety_ItemLevel[Current_Error])
			{
				case Safe:
				case Warning:
				{
					switch(OutputLevel)
					{
						case Safe:
						case Warning:
						break;
						case Risk:
						{
							__disable_irq();
							Mode_Silent();
							Mode_EN = 0U;
							__enable_irq();
						}
						break;
						case Deadliness:
						{
							__disable_irq();
							Mode_Silent();
							Mode_EN = 0U;
							__enable_irq();
							Safety_FlashErrorRecord(Current_Error);
						}
						break;
						default:;
					}
				}
				break;
				case Risk:
				{
					switch(OutputLevel)
					{
						case Safe:
						case Warning:
						{
							Safety_TryUserCtrlMode();
						}
						break;
						case Deadliness:
						{
							Safety_FlashErrorRecord(Current_Error);
						}
						break;
						default:;
					}
				}
				break;
				default:;
			}
			
			Safety_ItemLevelChange(Current_Error, OutputLevel);
		}
	}
}

/**
  ************************************************************************** 
  ** @name          : Safety_deltaP_ItemCheck
  ** @brief         : Specific implementation of the security check for Power difference between the two ends of DC-DC converter.Called immediately after obtaining the latest analogue value.
  ** @param         : Vbat_S,Current supply voltage (in international units).
  ** @param         : Isource_S,Current source current(in international units).
  ** @param         : Vcap_S,Current capacitor bank voltage (in international units).
  ** @param         : Icap_S,Current capacitor bank current(in international units).
  ** @retval        : void.
  ************************************************************************** 
**/
uint16_t deltaP_AlertCnt = 0;
uint16_t deltaP_ForceCnt = 0;
CCMRAM void Safety_deltaP_ItemCheck(float Vbat_S, float Isource_S,
																		float Vcap_S, float Icap_S)
{
	if(Safety_ItemLevel[deltaP_Error] != Deadliness)
	{
		float deltaP = Vbat_S*Isource_S - Vcap_S*Icap_S;
		deltaP = (deltaP>=0.0f ? deltaP : ((-1.0f)*deltaP));
		
		//更新deltaP_AlertCnt
		if(deltaP<25.0f && deltaP_AlertCnt>0)
		{
			deltaP_AlertCnt--;
		}
		else if(deltaP>=25.0f && deltaP_AlertCnt<350)
		{
			deltaP_AlertCnt++;
		}
		
		//分析deltaP_AlertCnt
		if(deltaP_AlertCnt==0 && Safety_ItemLevel[deltaP_Error]!=Safe)
		{
			Safety_ItemLevelChange(deltaP_Error, Safe);
			Safety_TryUserCtrlMode();
		}
		else if(deltaP_AlertCnt>0 && deltaP_AlertCnt<350 && Safety_ItemLevel[deltaP_Error]<Warning)
		{
			Safety_ItemLevelChange(deltaP_Error, Warning);
		}
		else if(deltaP_AlertCnt==350 && Safety_ItemLevel[deltaP_Error]<Risk)
		{
			__disable_irq();
			Mode_Silent();
			Mode_EN = 0U;
			__enable_irq();
			Safety_ItemLevelChange(deltaP_Error, Risk);
		}
		
		//更新deltaP_ForceCnt
		if(Safety_ItemLevel[deltaP_Error]==Risk && deltaP_ForceCnt<450)
		{
			deltaP_ForceCnt++;
		}
		else if(Safety_ItemLevel[deltaP_Error]<Risk && deltaP_ForceCnt>0)
		{
			deltaP_ForceCnt--;
		}
		
		//分析deltaP_ForceCnt
		if(deltaP_ForceCnt == 450)
		{
			Safety_ItemLevelChange(deltaP_Error, Deadliness);
			Safety_FlashErrorRecord(deltaP_Error);
		}
	}
}

/**
  ************************************************************************** 
  ** @name          : Safety_DMAden_ItemCheck
  ** @brief         : Specific implementation of the security check for DMA transfer error.Called in the interrupt callback function entered after a dma error.
  ** @param         : void.
  ** @retval        : void.
  ************************************************************************** 
**/
uint8_t DMAden_Cnt=0;
CCMRAM void Safety_DMAden_ItemCheck(void)
{
	if(DMAden_Cnt<3)
	{
		DMAden_Cnt++;
		
		if(Safety_ItemLevel[DMAden_Error] != Warning)
		{
			Safety_ItemLevelChange(DMAden_Error, Warning);
		}
	}
	else
	{
		if(Safety_ItemLevel[DMAden_Error] != Danger)
		{
			__disable_irq();
			Mode_EN = 0U;
			Mode_Silent();
			__enable_irq();
			Safety_ItemLevelChange(DMAden_Error, Danger);
		}
	}
}

/**
  ************************************************************************** 
  ** @name          : Safety_TryUserCtrlMode
  ** @brief         : Submitting a proposal to security management to restore user control may be rejected by a single vote on any of the tests.
  ** @param         : void.
  ** @retval        : void.
  ************************************************************************** 
**/
void Safety_TryUserCtrlMode(void)
{
	if(Mode_EN == 0U)
	{
		uint8_t i;
		for(i=0; i<8 && Safety_ItemLevel[i]<=Warning; i++);
		
		if(i == 8)
		{
			Mode_EN = 1U;
			if(UsrMsg_CtrlData.Mode_UpdateTrigger==1U)
			{
				UsrMsg_CtrlData.Mode_UpdateTrigger = 0U;
			}
			
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
	}
}

/**
  ************************************************************************** 
  ** @name          : Safety_ItemLevelChange
  ** @brief         : Change the security level of a specific test item.
  ** @param         : Item,Test items that require a change in security level.
  ** @param         : Level,The security level at which the test item will be changed.
  ** @retval        : void
  ************************************************************************** 
**/
CCMRAM void Safety_ItemLevelChange(uint8_t Item, uint8_t Level)
{
	if((Safety_ItemLevel[Item]<=Risk && Safety_ItemLevel[Item]!=Level)
	|| (Safety_ItemLevel[Item]==Danger && Level==Deadliness))
	{
		Safety_ItemLevel[Item] = Level;
		Safety_ItemLevel_ChangeTrigger = 1U;
	}
}

/**
  ************************************************************************** 
  ** @name          : Safety_FlashErrorRecord
  ** @brief         : The safety level corresponding to "Deadliness" for any of the test items is recorded in the flash memory.
  ** @param         : Item,Safety item to be recorded.
  ** @retval        : void
  ************************************************************************** 
**/
HAL_StatusTypeDef Safety_FlashErrorRecord(uint8_t Item)
{
	return Safety_FlashWrite64(0x0807F000 + 8*Item, 0xAAAAAAAAAAAAAAAA);
}

/**
  ************************************************************************** 
  ** @name          : Safety_FlashWrite64
  ** @brief         : Modify the 64-bit memory information of any address in flash.
  ** @param         : Address,Need to change the address of the stored information.
  ** @param         : Data,Target Storage Information.
	** @retval        : HAL_StatusTypeDef.
  ************************************************************************** 
**/
HAL_StatusTypeDef Safety_FlashWrite64(uint32_t Address, uint64_t Data)
{
	__disable_irq();//临界区始端
	HAL_FLASH_Unlock();
	HAL_StatusTypeDef Output = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
																							 Address,
																							 Data);
	HAL_FLASH_Lock();
	__enable_irq();//临界区末端
	
	return Output;
}

