#include "AlarmSystem.h"
#include "tim.h"

/***************************USER Change Start*********************************/
//闹钟系统时基对应定时器外设,需开启溢出中断
TIM_HandleTypeDef *AlarmSystem_TimeBase = &htim6;
/***************************USER Change Stop**********************************/

AlarmSystem_ChannelStateTypedef AlarmSystem_ChannelState;

/***************************功能与状态配置*********************************/
volatile uint32_t AlarmSystem_EnableFigure;//通道使能表
volatile uint16_t AlarmSystem_OverLoadValue[32];//各通道重载值
volatile uint16_t AlarmSystem_CountValue[32];//各通道当前计数值
volatile uint32_t AlarmSystem_CycleModeFigure;//各通道循环模式设置
volatile uint32_t AlarmSystem_RingFigure;//各通道是否响铃
volatile uint8_t  AlarmSystem_EnableChannelNbr;//使能通道数
volatile uint8_t  AlarmSystem_PeakChannelNbr;//非空闲通道数

/**
  ************************************************************************** 
  ** @name          : AlarmSystem_Init
  ** @brief         : Initialising the alarm clock system.
  ** @param         : void
  ** @retval        : void
  ************************************************************************** 
**/
void AlarmSystem_Init(void)
{
	HAL_TIM_Base_Start_IT(AlarmSystem_TimeBase);
	
	AlarmSystem_EnableFigure=0;
	AlarmSystem_RingFigure=0;
	AlarmSystem_EnableChannelNbr=0;
	AlarmSystem_PeakChannelNbr=0;
}

/**
  ************************************************************************** 
  ** @name          : AlarmSystem_ChannelActive
  ** @brief         : Match free channels to start timing tasks.
  ** @param         : OverLoadValue,timekeeping.
  ** @param         : CycleMode,Whether to configure for round robin timing,0 or 1 only.
  ** @retval        : uint8_t,Usually the value of the matched channel.If 0xFF,Failure to match a free channel.
  ************************************************************************** 
**/
uint8_t AlarmSystem_ChannelActive(uint16_t OverLoadValue, uint8_t CycleMode)
{
	//监测是否存在空闲通道
	if(AlarmSystem_PeakChannelNbr == 32)
	{
		return 0xFF;
	}
	
	__disable_irq();//临界区始端
	
	//搜索空闲通道序号
	uint8_t FreeChannel;
	for(FreeChannel=0; 
			 __ALARMSYSTEM_BIT_Read(AlarmSystem_EnableFigure, FreeChannel)==1 
		|| __ALARMSYSTEM_BIT_Read(AlarmSystem_RingFigure, FreeChannel)==1;
			FreeChannel++);
	
	//空闲通道配置与初始化
	AlarmSystem_OverLoadValue[FreeChannel] = OverLoadValue;
	switch(CycleMode)
	{
		case 0U:__ALARMSYSTEM_BIT_WRITE0(AlarmSystem_CycleModeFigure, FreeChannel);
		break;
		case 1U:__ALARMSYSTEM_BIT_WRITE1(AlarmSystem_CycleModeFigure, FreeChannel);
		break;
		default:;
	}
	AlarmSystem_CountValue[FreeChannel] = 0;
	__ALARMSYSTEM_BIT_WRITE1(AlarmSystem_EnableFigure, FreeChannel);
	AlarmSystem_EnableChannelNbr++;
	AlarmSystem_PeakChannelNbr++;
	
	__enable_irq();//临界区末端
	
	return FreeChannel;
}

/**
  ************************************************************************** 
  ** @name          : AlarmSystem_ChannelInactive
  ** @brief         : Cancellation of activities on specific channels.
  ** @param         : Channel,Serial number of the particular channel.
  ** @retval        : void
  ************************************************************************** 
**/
void AlarmSystem_ChannelInactive(uint8_t *Channel)
{
	__disable_irq();//临界区始端
	
	if(__ALARMSYSTEM_BIT_Read(AlarmSystem_EnableFigure, *Channel) == 1U)
	{
		__ALARMSYSTEM_BIT_WRITE0(AlarmSystem_EnableFigure, *Channel);
		AlarmSystem_EnableChannelNbr--;
	}
	__ALARMSYSTEM_BIT_WRITE0(AlarmSystem_RingFigure,   *Channel);

	AlarmSystem_PeakChannelNbr--;
	
	*Channel = 0xFF;
	__enable_irq();//临界区末端
}

/**
  ************************************************************************** 
  ** @name          : AlarmSystem_GetBellRing
  ** @brief         : Check if a specific channel is ringing.
  ** @brief         : Passing into this function with variables that are not initialised or have not been assigned values by AlarmSystem_ChannelActive is dangerous behaviour. It is recommended that the alarm channel variable be assigned the value 0xff when it is initialised.
  ** @param         : Channel,Serial number of the particular channel.
	** @retval        : AlarmSystem_ChannelStateTypedef.Clocking point to Timing mission in progress.Ring point to Timing mission is ending.
  ************************************************************************** 
**/
AlarmSystem_ChannelStateTypedef AlarmSystem_GetBellRing(uint8_t *Channel)
{
	if(*Channel > 31)
	{
		return UnInit;
	}
	
	if(__ALARMSYSTEM_BIT_Read(AlarmSystem_RingFigure, *Channel) == 1)
	{
		__disable_irq();//临界区始端
		
		__ALARMSYSTEM_BIT_WRITE0(AlarmSystem_RingFigure, *Channel);
		
		if(__ALARMSYSTEM_BIT_Read(AlarmSystem_CycleModeFigure, *Channel) == 0)
		{
			AlarmSystem_PeakChannelNbr--;
			*Channel = 0xFF;
		}
		
		__enable_irq();//临界区末端
		
		return Ring;
	}
	else
	{
		return Clocking;
	}
}

//该中断需最高抢占优先级
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == AlarmSystem_TimeBase)
	{
		uint8_t i,EnableCN_Counter;
		for(i=0,EnableCN_Counter=0; i<32 && EnableCN_Counter<AlarmSystem_EnableChannelNbr; i++)
		{
			if(__ALARMSYSTEM_BIT_Read(AlarmSystem_EnableFigure, i) == 1)//判定该通道是否使能
			{
				//计数值不足时自增，否则判定是否为循环模式。若是，清空计数值，若不是，失能该通道。
				if(AlarmSystem_CountValue[i] < AlarmSystem_OverLoadValue[i])
				{
					AlarmSystem_CountValue[i]++;
				}
				else
				{
					__ALARMSYSTEM_BIT_WRITE1(AlarmSystem_RingFigure, i);
					
					if(__ALARMSYSTEM_BIT_Read(AlarmSystem_CycleModeFigure, i) == 1)
					{
						AlarmSystem_CountValue[i] = 0;
					}
					else
					{
						__ALARMSYSTEM_BIT_WRITE0(AlarmSystem_EnableFigure, i);
						AlarmSystem_EnableChannelNbr--;
					}
				}
				
				EnableCN_Counter++;
			}
		}
	}
}

