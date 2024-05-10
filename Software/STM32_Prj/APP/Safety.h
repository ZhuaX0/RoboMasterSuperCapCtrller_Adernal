#ifndef __SAFETY
#define __SAFETY

#include "main.h"

typedef enum
{
	Firmware_Error,
	bxCAN_Error,
	Temp_Error,
	INA240cali_Error,
	Voltage_Error,
	Current_Error,
	deltaP_Error,
	DMAden_Error
}
Safety_DetectItemTypedef;

typedef enum
{
	Safe,
	Warning,
	Risk,
	Danger,
	Deadliness
}
Safety_LevelTypedef;

extern Safety_DetectItemTypedef Safety_DetectItem;
extern Safety_LevelTypedef Safety_Level;
extern uint8_t Safety_ItemLevel[8];
extern uint8_t Safety_ItemLevel_ChangeTrigger;

void Safety_Init(void);
void Safety_Firmware_ItemCheck(void);
CCMRAM void Safety_bxCAN_ItemCheck(void);
CCMRAM void Safety_Temp_ItemCheck(float Temp_S);
void Safety_INA240cali_ItemCheck(float Source_0point, float Cap_0point);
CCMRAM void Safety_VcapParamSet(float VcapMax);
CCMRAM void Safety_Voltage_ItemCheck(float Vbat_S, float Vcap_S);
CCMRAM void Saftey_CurrentParamSet(float Isource_AlertLim, float Isource_ForceLim, float Icap_AlertLim, float Icap_ForceLim);
CCMRAM void Safety_Current_ItemCheck(float Isource_S, float Icap_S);
CCMRAM void Safety_deltaP_ItemCheck(float Vbat_S, float Isource_S, float Vcap_S, float Icap_S);
CCMRAM void Safety_DMAden_ItemCheck(void);
void Safety_TryUserCtrlMode(void);
CCMRAM void Safety_ItemLevelChange(uint8_t Item, uint8_t Level);
HAL_StatusTypeDef Safety_FlashErrorRecord(uint8_t Item);
HAL_StatusTypeDef Safety_FlashWrite64(uint32_t Address, uint64_t Data);

#endif

