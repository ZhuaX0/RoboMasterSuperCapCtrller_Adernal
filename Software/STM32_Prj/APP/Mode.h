#ifndef __MODE
#define __MODE

#include "main.h"

typedef enum
{
	CapType1_24V,
	CapType2_28V,
	CapType3_30V
}
Mode_CapType_Typedef;

typedef enum
{
	Silent,
	Work,
	Charge
}
Mode_ModeTypedef;

extern Mode_CapType_Typedef Mode_CapType_Enum;
extern Mode_ModeTypedef Mode_ModeType_Enum;
extern float Mode_IcapLimit;
extern float Mode_PsourceLimit;
extern uint8_t Mode_EN;

void Mode_Init(void);
void Mode_CapType_Set(uint8_t CapType);
CCMRAM void Mode_ExpectPchassis_Set(float ExpectPchassis);
CCMRAM void Mode_Exceed_Set(uint8_t ExceedEN);
CCMRAM void Mode_Silent(void);
CCMRAM void Mode_Work(void);
CCMRAM void Mode_Charge(void);

#endif

