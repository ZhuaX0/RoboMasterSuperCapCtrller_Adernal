#ifndef __USRMSG
#define __USRMSG

#include "main.h"

typedef enum
{
	UsrMsgTx_SafeID = 0x001,
	UsrMsgRx_InitID,
	UsrMsgTx_FbID,
	UsrMsgRx_CtrlID,
	UsrMsgTx_StdByID
}
UsrMsg_CANID_Typedef;

typedef struct
{
	uint8_t UpdateTrigger;
	uint8_t CapTypeGet;//0x00~0x02
}
UsrMsg_InitData_Typedef;

typedef struct
{
	uint8_t Pchassis_UpdateTrigger;
	uint8_t PchassisGet_8;//Serial
	float PchassisGet_f;
	uint8_t Exceed_UpdateTrigger;
	uint8_t ExceedGet;//0x00 || 0xFF
	uint8_t Mode_UpdateTrigger;
	uint8_t ModeGet;//0x00~0x02
}
UsrMsg_CtrlData_Typedef;

extern UsrMsg_InitData_Typedef UsrMsg_InitData;
extern UsrMsg_CtrlData_Typedef UsrMsg_CtrlData;

void UsrMsg_Init(void);
CCMRAM void UsrMsg_InitFrame_Unpack(uint8_t *RxData);
CCMRAM void UsrMsg_CtrlFrame_Unpack(uint8_t *RxData);
CCMRAM void UsrMsg_StdByFrame_Transmit(FDCAN_HandleTypeDef *hfdcan, uint8_t Ready);
CCMRAM void UsrMsg_FbFrame_Transmit(FDCAN_HandleTypeDef *hfdcan, float Vcap_EsrFix, float WorkIntensity1, float WorkIntensity2, float Pchassis_S);
CCMRAM void UsrMsg_SafeFrame_Transmit(FDCAN_HandleTypeDef *hfdcan, uint8_t *SafeCode);

#endif

