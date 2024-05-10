#include "UsrMsg.h"

UsrMsg_CANID_Typedef UsrMsg_CANID;

UsrMsg_InitData_Typedef UsrMsg_InitData = {0};
UsrMsg_CtrlData_Typedef UsrMsg_CtrlData = {0};

void UsrMsg_Init(void)
{
	UsrMsg_InitData.CapTypeGet = 0x7F;
	UsrMsg_CtrlData.PchassisGet_8 = 0x00;
	UsrMsg_CtrlData.ExceedGet = 0x7F;
	UsrMsg_CtrlData.ModeGet = 0x7F;
}

CCMRAM void UsrMsg_InitFrame_Unpack(uint8_t *RxData)
{
	if(RxData[0] != UsrMsg_InitData.CapTypeGet
	&& RxData[0] <= 0x02)
	{
		UsrMsg_InitData.CapTypeGet = RxData[0];
		UsrMsg_InitData.UpdateTrigger = 1U;
	}
}

CCMRAM void UsrMsg_CtrlFrame_Unpack(uint8_t *RxData)
{
	if(RxData[0] != UsrMsg_CtrlData.PchassisGet_8
	&& RxData[0] <= 210U)
	{
		UsrMsg_CtrlData.PchassisGet_8 = RxData[0];
		UsrMsg_CtrlData.PchassisGet_f = (float)RxData[0];
		UsrMsg_CtrlData.Pchassis_UpdateTrigger = 1U;
	}
	if(RxData[1] != UsrMsg_CtrlData.ExceedGet
	&& RxData[1] <= 0x01)
	{
		UsrMsg_CtrlData.ExceedGet = RxData[1];
		UsrMsg_CtrlData.Exceed_UpdateTrigger = 1U;
	}
	if(RxData[2] != UsrMsg_CtrlData.ModeGet
	&& RxData[2] <= 0x02)
	{
		UsrMsg_CtrlData.ModeGet = RxData[2];
		UsrMsg_CtrlData.Mode_UpdateTrigger = 1U;
	}
}

CCMRAM void UsrMsg_StdByFrame_Transmit(FDCAN_HandleTypeDef *hfdcan, uint8_t Ready)
{
	FDCAN_TxHeaderTypeDef FDCAN_TxHeader;
	FDCAN_TxHeader.Identifier = (uint32_t)UsrMsgTx_StdByID;
	FDCAN_TxHeader.IdType = FDCAN_STANDARD_ID;
	FDCAN_TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	FDCAN_TxHeader.DataLength = FDCAN_DLC_BYTES_1;
	FDCAN_TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;//不确定
	FDCAN_TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	FDCAN_TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	FDCAN_TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	FDCAN_TxHeader.MessageMarker = (uint32_t)UsrMsgTx_StdByID;
	
	uint8_t TxData = Ready;
	
	HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &FDCAN_TxHeader, &TxData);
}

CCMRAM void UsrMsg_FbFrame_Transmit(FDCAN_HandleTypeDef *hfdcan, float Vcap_EsrFix, float WorkIntensity1, float WorkIntensity2, float Pchassis_S)
{
	FDCAN_TxHeaderTypeDef FDCAN_TxHeader;
	FDCAN_TxHeader.Identifier = (uint32_t)UsrMsgTx_FbID;
	FDCAN_TxHeader.IdType = FDCAN_STANDARD_ID;
	FDCAN_TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	FDCAN_TxHeader.DataLength = FDCAN_DLC_BYTES_6;
	FDCAN_TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;//不确定
	FDCAN_TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	FDCAN_TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	FDCAN_TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	FDCAN_TxHeader.MessageMarker = (uint32_t)UsrMsgTx_FbID;
	
	uint16_t Vcap_Tx = (uint16_t)(Vcap_EsrFix * 100);
	uint16_t Pchassis_Tx = (uint16_t)(Pchassis_S * 100);
	uint8_t TxData[6];
	
	TxData[0] = (Vcap_Tx>>8);
	TxData[1] = Vcap_Tx;
	TxData[2] = (uint8_t)(WorkIntensity1*100);
	TxData[3] = (uint8_t)(WorkIntensity2*100);
	TxData[4] = (Pchassis_Tx>>8);
	TxData[5] = Pchassis_Tx;
	
	HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &FDCAN_TxHeader, TxData);
}

CCMRAM void UsrMsg_SafeFrame_Transmit(FDCAN_HandleTypeDef *hfdcan, uint8_t *SafeCode)
{
	FDCAN_TxHeaderTypeDef FDCAN_TxHeader;
	FDCAN_TxHeader.Identifier = (uint32_t)UsrMsgTx_SafeID;
	FDCAN_TxHeader.IdType = FDCAN_STANDARD_ID;
	FDCAN_TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	FDCAN_TxHeader.DataLength = FDCAN_DLC_BYTES_8;
	FDCAN_TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;//不确定
	FDCAN_TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	FDCAN_TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	FDCAN_TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	FDCAN_TxHeader.MessageMarker = (uint32_t)UsrMsgTx_SafeID;
	
	HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &FDCAN_TxHeader, SafeCode);
}

