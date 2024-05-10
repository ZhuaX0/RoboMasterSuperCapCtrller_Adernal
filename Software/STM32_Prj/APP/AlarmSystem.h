/*
闹钟系统支持库,依托HAL库使用
该库依托定时器外设，提供32个闹钟通道。
每个闹钟通道提供独立的，非阻塞性的非精确性的ms单位倒计时服务
每个闹钟通道同时可以配置为循环模式，即周期性的经过预设的的时间后触发响铃
使用闹钟系统的循环模式可以构建一个粗略的实时操作系统。
*/
#ifndef __ALARMSYSTEM
#define __ALARMSYSTEM

#include "main.h"

#define __ALARMSYSTEM_BIT_WRITE0(WrittenVaria,Offset) ((WrittenVaria)&=~(1U<<(Offset)))
#define __ALARMSYSTEM_BIT_WRITE1(WrittenVaria,Offset) ((WrittenVaria)|=(1U<<(Offset)))
#define __ALARMSYSTEM_BIT_Read(ReadVaria,Offset) (((ReadVaria)&(1U<<(Offset)))!=0?1:0)

typedef enum
{
	Clocking,
	Ring,
	UnInit
}
AlarmSystem_ChannelStateTypedef;

extern AlarmSystem_ChannelStateTypedef AlarmSystem_ChannelState;

void AlarmSystem_Init(void);
uint8_t AlarmSystem_ChannelActive(uint16_t OverLoadValue, uint8_t CycleMode);
void AlarmSystem_ChannelInactive(uint8_t *Channel);
AlarmSystem_ChannelStateTypedef AlarmSystem_GetBellRing(uint8_t *Channel);


#endif

