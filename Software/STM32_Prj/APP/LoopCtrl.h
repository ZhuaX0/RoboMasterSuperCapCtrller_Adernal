#ifndef __LOOPCTRL
#define __LOOPCTRL

#include "main.h"
#include "arm_math.h"

//0:Kp,1:Ki,2:Kd
typedef struct
{
	float VcapHighLoop_Kp,VcapHighLoop_Ki,VcapHighLoop_Kd;
	float VcapLowLoop_Kp,VcapLowLoop_Ki,VcapLowLoop_Kd;
	float IcapLoop_Kp,IcapLoop_Ki,IcapLoop_Kd;
	float PsourceLoop_Kp,PsourceLoop_Ki,PsourceLoop_Kd;
	float PchassisLoop_Kp,PchassisLoop_Ki,PchassisLoop_Kd;
}
LoopCtrl_KpKiKdInit_Typedef;

typedef struct
{
	float VcapHigh_IntegMax,VcapHigh_IntegMin,
				VcapHigh_OutputMax,VcapHigh_OutputMin;
	float VcapLow_IntegMax,VcapLow_IntegMin,
				VcapLow_OutputMax,VcapLow_OutputMin;
	float Icap_IntegMax,Icap_IntegMin,
				Icap_OutputMax,Icap_OutputMin;
	float Psource_PID_IntegMax,Psource_PID_IntegMin,
				Psource_PID_OutputMax,Psource_PID_OutputMin,
				Psource_ffPID_OutputMax,Psource_ffPID_OutputMin;
	float Pchassis_IntegMin,Pchassis_OutputMin;
}
LoopCtrl_LimitInit_Typedef;

typedef struct
{
	arm_pid_instance_f32 ArmPID_Instance;
	float IntegMax;
	float IntegMin;
	float OutputMax;
	float OutputMin;
}
LoopCtrl_PID_Typedef;

typedef struct
{
	LoopCtrl_PID_Typedef PID_Instance;
	float OutputMax;
	float OutputMin;
}
LoopCtrl_ffPID_Typedef;

extern LoopCtrl_KpKiKdInit_Typedef LoopCtrl_KpKiKdStructure;
extern LoopCtrl_LimitInit_Typedef LoopCtrl_LimitStructure;

void LoopCtrl_StructInit(void);
HAL_StatusTypeDef LoopCtrl_Start(LoopCtrl_KpKiKdInit_Typedef *LoopCtrl_KpKiKdInit, LoopCtrl_LimitInit_Typedef *LoopCtrl_LimitInit, float ExpectChassis, float ExpectVcapH, float ExpectVcapL);
CCMRAM void LoopCtrl_Stop(void);
CCMRAM void LoopCtrl_ExpectPchassisSet(float ExpectPchassis_In);
CCMRAM void LoopCtrl_Main(float Icap_S, float Ichassis_S, float Vcap_S, float Isource_S, float Vbat_S);
/**************************Arithmetic***************************/
CCMRAM float PID_Calculate(LoopCtrl_PID_Typedef *LoopCtrl_PID_Instance, float Expect, float Feedback);
CCMRAM float ffPID_Calculate(LoopCtrl_ffPID_Typedef *LoopCtrl_ffPID_Instance, float Expect, float Feedback, float Model);

#endif

