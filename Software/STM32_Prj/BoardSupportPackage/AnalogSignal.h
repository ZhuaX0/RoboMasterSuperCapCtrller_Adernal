#ifndef __ANALOG_SIGNAL
#define __ANALOG_SIGNAL

#include "main.h"

//ADC单位(Digital)与国际单位(Standard)互相转换，TYP为转换至类型，对应AnalogDataTypology_t
#define __ANALOGSIGNAL_STD_CNVT(INPUT_S, TYP) (uint16_t)(AnalogUnitCnvtParm[(TYP)].Gain*(INPUT_S)+AnalogUnitCnvtParm[(TYP)].Offset)
#define __ANALOGSIGNAL_DTS_CNVT(INPUT_D, TYP) (((float)(INPUT_D)-AnalogUnitCnvtParm[(TYP)].Offset)/AnalogUnitCnvtParm[(TYP)].Gain)

typedef enum
{
	Icap,Ichassis,Vcap,//ADC1
	Isource,Vbat//ADC2
}
AnalogDataTypology_t;

typedef struct
{
	float Gain;
	float Offset;
}
AnalogUnitCnvt_HandleTypedef;

typedef struct
{
	uint16_t FilterWindow[64];
	uint8_t  Length;
	uint8_t  Probe;
	uint32_t Sum;
}
WindowFilter_HandleTypeDef;

extern AnalogDataTypology_t  AnalogDataTypology;
extern uint16_t AnalogSignal_FilteredData[5];
extern uint16_t AnalogSignal_RealtimeData[5];
extern uint16_t AnalogSignal_CapData_RxBuffer[2][16][3];
extern uint16_t AnalogSignal_SRCData_RxBuffer[2][16][2];
extern AnalogUnitCnvt_HandleTypedef AnalogUnitCnvtParm[5];
extern uint8_t AnalogSignal_CaliReady;

CCMRAM void AnalogSignal_ADCDMA_OVRStop(ADC_HandleTypeDef *hadc);
CCMRAM void AnalogSignal_ADCDMA_OVRRecovery(ADC_HandleTypeDef *hadc);
void AnalogSignal_AnalogData_Init(uint8_t Icap_WinLength, uint8_t Icha_WinLength, uint8_t Vcap_WinLength, uint8_t Isrc_WinLength, uint8_t Vbat_WinLength);
CCMRAM void AnalogSignal_AnalogData_Update(void);
void AnalogSignal_AnalogData_Calibration(void);
CCMRAM float AnalogSignal_GetTemp(void);

/*******************************************底层函数，用户禁止修改****************************************************/
void AnalogSignal_WindowFilter_Init(WindowFilter_HandleTypeDef *hWindowFilter, uint8_t SetLength);
CCMRAM uint16_t AnalogSignal_WindowFilter_DataUpdate(WindowFilter_HandleTypeDef *hWindowFilter, uint16_t InputData);

#endif

