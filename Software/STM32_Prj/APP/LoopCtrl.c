#include "LoopCtrl.h"
#include "DCDC.h"
#include "AnalogSignal.h"

uint8_t LoopCtrl_GoingFlag=0;

LoopCtrl_KpKiKdInit_Typedef LoopCtrl_KpKiKdStructure;
LoopCtrl_LimitInit_Typedef LoopCtrl_LimitStructure;

LoopCtrl_PID_Typedef LoopCtrl_PID_VcapHigh;
LoopCtrl_PID_Typedef LoopCtrl_PID_VcapLow;
LoopCtrl_PID_Typedef LoopCtrl_PID_Icap;
LoopCtrl_ffPID_Typedef LoopCtrl_ffPID_Psource;
LoopCtrl_PID_Typedef LoopCtrl_PID_Pchassis;

float LoopCtrl_ExpectPchassis,LoopCtrl_ExpectVcapH,LoopCtrl_ExpectVcapL;
float Expect_Icap_Last;

/**
  ************************************************************************** 
  ** @name          : LoopCtrl_Init
  ** @brief         : Initialise the parameters of each control loop.
  ** @param         : void
  ** @retval        : void
  ************************************************************************** 
**/
void LoopCtrl_StructInit(void)
{
	//电流环KpKiKd初始化
	LoopCtrl_KpKiKdStructure.IcapLoop_Kp = 0.0007f;//simulink result:0.0003
	LoopCtrl_KpKiKdStructure.IcapLoop_Ki = 0.00062f;//simulink result:0.001
	LoopCtrl_KpKiKdStructure.IcapLoop_Kd = 0.0f;//simulink result:0.0
	//Psource环KpKiKd初始化
	LoopCtrl_KpKiKdStructure.PsourceLoop_Kp = 0.0f;//simulink result:0.03
	LoopCtrl_KpKiKdStructure.PsourceLoop_Ki = 0.0f;//simulink result:0.01
	LoopCtrl_KpKiKdStructure.PsourceLoop_Kd = 0.0f;//simulink result:0.0
	//电池功率环KpKiKd初始化
	LoopCtrl_KpKiKdStructure.PchassisLoop_Kp = 0.05f;//simulink result:0.7
	LoopCtrl_KpKiKdStructure.PchassisLoop_Ki = 0.008f;//simulink result:0.09
	LoopCtrl_KpKiKdStructure.PchassisLoop_Kd = 0.0f;//simulink result:0.0
	//高限电压环KpKiKd初始化
	LoopCtrl_KpKiKdStructure.VcapHighLoop_Kp = 0.001f;//simulink result:0.001
	LoopCtrl_KpKiKdStructure.VcapHighLoop_Ki = 0.0025f;//simulink result:0.01
	LoopCtrl_KpKiKdStructure.VcapHighLoop_Kd = 0.0f;//simulink result:0.0
	//低限电压环KpKiKd初始化
	LoopCtrl_KpKiKdStructure.VcapLowLoop_Kp = 0.001f;//simulink result:0.001
	LoopCtrl_KpKiKdStructure.VcapLowLoop_Ki = 0.0025f;//simulink result:0.01
	LoopCtrl_KpKiKdStructure.VcapLowLoop_Kd = 0.0f;//simulink result:0.0
	
	//电流环限幅初始化(test only
	LoopCtrl_LimitStructure.Icap_IntegMax  = 28.0f/19.5f;
	LoopCtrl_LimitStructure.Icap_IntegMin  = 3.5f /27.5f;
	LoopCtrl_LimitStructure.Icap_OutputMax = 28.0f/19.5f;
	LoopCtrl_LimitStructure.Icap_OutputMin = 3.5f /27.5f;
	//Psource环限幅初始化(test only
	LoopCtrl_LimitStructure.Psource_PID_IntegMax  = 0.5f;
	LoopCtrl_LimitStructure.Psource_PID_IntegMin  = -0.5f;
	LoopCtrl_LimitStructure.Psource_PID_OutputMax = 0.5f;
	LoopCtrl_LimitStructure.Psource_PID_OutputMin = -0.5f;
	LoopCtrl_LimitStructure.Psource_ffPID_OutputMax = 10.0f;
	LoopCtrl_LimitStructure.Psource_ffPID_OutputMin = -10.0f;
	//Pchassis环限幅初始化(test only
	LoopCtrl_LimitStructure.Pchassis_IntegMin  = -205.0f;
	LoopCtrl_LimitStructure.Pchassis_OutputMin = -205.0f;
	//高限电压环限幅初始化(test only
	LoopCtrl_LimitStructure.VcapHigh_IntegMax  = 28.0f/19.5f;
	LoopCtrl_LimitStructure.VcapHigh_IntegMin  = 28.0f/27.5f;
	LoopCtrl_LimitStructure.VcapHigh_OutputMax = 28.0f/19.5f;
	LoopCtrl_LimitStructure.VcapHigh_OutputMin = 28.0f/27.5f;
	//低限电压环限幅初始化(test only
	LoopCtrl_LimitStructure.VcapLow_IntegMax  = 3.5f/19.5f;
	LoopCtrl_LimitStructure.VcapLow_IntegMin  = 3.5f/27.5f;
	LoopCtrl_LimitStructure.VcapLow_OutputMax = 3.5f/19.5f;
	LoopCtrl_LimitStructure.VcapLow_OutputMin = 3.5f/27.5f;
}

/**
  ************************************************************************** 
  ** @name          : LoopCtrl_Start
  ** @brief         : Perform the necessary parameter preprocessing to start the loop calculation with the output HRPWM.
  ** @param         : void
  ** @retval        : HAL_StatusTypeDef,if HAL_OK,loop start successful.if HAL_ERROR,Refuse due to security concerns.
  ************************************************************************** 
**/
HAL_StatusTypeDef LoopCtrl_Start(LoopCtrl_KpKiKdInit_Typedef *LoopCtrl_KpKiKdInit, LoopCtrl_LimitInit_Typedef *LoopCtrl_LimitInit,
																									float ExpectChassis, float ExpectVcapH, float ExpectVcapL)
{
	HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
	
	//电容电流环KpKiKd
	LoopCtrl_PID_Icap.ArmPID_Instance.Kp = LoopCtrl_KpKiKdInit->IcapLoop_Kp;
	LoopCtrl_PID_Icap.ArmPID_Instance.Ki = LoopCtrl_KpKiKdInit->IcapLoop_Ki;
	LoopCtrl_PID_Icap.ArmPID_Instance.Kd = LoopCtrl_KpKiKdInit->IcapLoop_Kd;
	//Psource环KpKiKd
	LoopCtrl_ffPID_Psource.PID_Instance.ArmPID_Instance.Kp = LoopCtrl_KpKiKdInit->PsourceLoop_Kp;
	LoopCtrl_ffPID_Psource.PID_Instance.ArmPID_Instance.Ki = LoopCtrl_KpKiKdInit->PsourceLoop_Ki;
	LoopCtrl_ffPID_Psource.PID_Instance.ArmPID_Instance.Kd = LoopCtrl_KpKiKdInit->PsourceLoop_Kd;
	//Pchassis环KpKiKd
	LoopCtrl_PID_Pchassis.ArmPID_Instance.Kp = LoopCtrl_KpKiKdInit->PchassisLoop_Kp;
	LoopCtrl_PID_Pchassis.ArmPID_Instance.Ki = LoopCtrl_KpKiKdInit->PchassisLoop_Ki;
	LoopCtrl_PID_Pchassis.ArmPID_Instance.Kd = LoopCtrl_KpKiKdInit->PchassisLoop_Kd;
	//高限电压环KpKiKd
	LoopCtrl_PID_VcapHigh.ArmPID_Instance.Kp = LoopCtrl_KpKiKdInit->VcapHighLoop_Kp;
	LoopCtrl_PID_VcapHigh.ArmPID_Instance.Ki = LoopCtrl_KpKiKdInit->VcapHighLoop_Ki;
	LoopCtrl_PID_VcapHigh.ArmPID_Instance.Kd = LoopCtrl_KpKiKdInit->VcapHighLoop_Kd;
	//低限制电压环KpKiKd
	LoopCtrl_PID_VcapLow.ArmPID_Instance.Kp = LoopCtrl_KpKiKdInit->VcapLowLoop_Kp;
	LoopCtrl_PID_VcapLow.ArmPID_Instance.Ki = LoopCtrl_KpKiKdInit->VcapLowLoop_Ki;
	LoopCtrl_PID_VcapLow.ArmPID_Instance.Kd = LoopCtrl_KpKiKdInit->VcapLowLoop_Kd;
	
	//电容电流环限幅
	LoopCtrl_PID_Icap.IntegMax  = LoopCtrl_LimitInit->Icap_IntegMax;
	LoopCtrl_PID_Icap.IntegMin  = LoopCtrl_LimitInit->Icap_IntegMin;
	LoopCtrl_PID_Icap.OutputMax = LoopCtrl_LimitInit->Icap_OutputMax;
	LoopCtrl_PID_Icap.OutputMin = LoopCtrl_LimitInit->Icap_OutputMin;
	//Psource环限幅
	LoopCtrl_ffPID_Psource.PID_Instance.IntegMax  = LoopCtrl_LimitInit->Psource_PID_IntegMax;
	LoopCtrl_ffPID_Psource.PID_Instance.IntegMin  = LoopCtrl_LimitInit->Psource_PID_IntegMin;
	LoopCtrl_ffPID_Psource.PID_Instance.OutputMax = LoopCtrl_LimitInit->Psource_PID_OutputMax;
	LoopCtrl_ffPID_Psource.PID_Instance.OutputMin = LoopCtrl_LimitInit->Psource_PID_OutputMin;
	LoopCtrl_ffPID_Psource.OutputMax = LoopCtrl_LimitInit->Psource_ffPID_OutputMax;
	LoopCtrl_ffPID_Psource.OutputMin = LoopCtrl_LimitInit->Psource_ffPID_OutputMin;
	//Pchassis环限幅
	LoopCtrl_ExpectPchassisSet(ExpectChassis);
	LoopCtrl_PID_Pchassis.IntegMin  = LoopCtrl_LimitInit->Pchassis_IntegMin;
	LoopCtrl_PID_Pchassis.OutputMin = LoopCtrl_LimitInit->Pchassis_OutputMin;
	//高限电压限幅
	LoopCtrl_PID_VcapHigh.IntegMax  = LoopCtrl_LimitInit->VcapHigh_IntegMax;
	LoopCtrl_PID_VcapHigh.IntegMin  = LoopCtrl_LimitInit->VcapHigh_IntegMin;
	LoopCtrl_PID_VcapHigh.OutputMax = LoopCtrl_LimitInit->VcapHigh_OutputMax;
	LoopCtrl_PID_VcapHigh.OutputMin = LoopCtrl_LimitInit->VcapHigh_OutputMin;
	//低限电压环限幅
	LoopCtrl_PID_VcapLow.IntegMax  = LoopCtrl_LimitInit->VcapLow_IntegMax;
	LoopCtrl_PID_VcapLow.IntegMin  = LoopCtrl_LimitInit->VcapLow_IntegMin;
	LoopCtrl_PID_VcapLow.OutputMax = LoopCtrl_LimitInit->VcapLow_OutputMax;
	LoopCtrl_PID_VcapLow.OutputMin = LoopCtrl_LimitInit->VcapLow_OutputMin;

	//设定高低限电容组电压
	LoopCtrl_ExpectVcapH = ExpectVcapH;
	LoopCtrl_ExpectVcapL = ExpectVcapL;

	float NowDutyRatio = 
	__ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vcap], Vcap) / __ANALOGSIGNAL_DTS_CNVT(AnalogSignal_FilteredData[Vbat], Vbat);
	
	//高限电压环初始化
	arm_pid_init_f32(&(LoopCtrl_PID_VcapHigh.ArmPID_Instance), 1); 
	LoopCtrl_PID_VcapHigh.ArmPID_Instance.state[2] = LoopCtrl_PID_VcapHigh.IntegMax;
	//低限电压环初始化
	arm_pid_init_f32(&(LoopCtrl_PID_VcapLow.ArmPID_Instance), 1);
	LoopCtrl_PID_VcapLow.ArmPID_Instance.state[2] = LoopCtrl_PID_VcapLow.IntegMin;
	//电流环初始化
	arm_pid_init_f32(&(LoopCtrl_PID_Icap.ArmPID_Instance), 1);
	LoopCtrl_PID_Icap.ArmPID_Instance.state[2] = NowDutyRatio;
	//Psource环初始化
	arm_pid_init_f32(&(LoopCtrl_ffPID_Psource.PID_Instance.ArmPID_Instance), 1);
	Expect_Icap_Last = 0.0f;
	//电池功率环初始化
	arm_pid_init_f32(&(LoopCtrl_PID_Pchassis.ArmPID_Instance), 1);
	
	DCDC_WorkStart(NowDutyRatio);
	
	LoopCtrl_GoingFlag = 1UL;
	
	return HAL_OK;
}

/**
  ************************************************************************** 
  ** @name          : LoopCtrl_Stop
  ** @brief         : Stops loop calculations and turns off HRPWM outputs.
  ** @param         : void
  ** @retval        : void
  ************************************************************************** 
**/
CCMRAM void LoopCtrl_Stop(void)
{
	LoopCtrl_GoingFlag = 0;
	DCDC_WorkStop();

	HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
}

CCMRAM void LoopCtrl_ExpectPchassisSet(float ExpectPchassis_In)
{
	float ExpectChassis;
	if(ExpectPchassis_In > 210.0f)//裁判规定机器人底盘功率最高不超过200W
	{
		ExpectChassis = 210.0f;
	}
	else if(ExpectPchassis_In < 35.0f)//裁判规定机器人底盘功率最低不超过45W
	{
		ExpectChassis = 35.0f;
	}
	else
	{
		ExpectChassis = ExpectPchassis_In;
	}
	
	LoopCtrl_PID_Pchassis.IntegMax  = ExpectChassis + 10.0f;//留10W裕量
	LoopCtrl_PID_Pchassis.OutputMax = ExpectChassis + 10.0f;//留10W裕量
	LoopCtrl_ExpectPchassis = ExpectChassis;
}

/**
  ************************************************************************** 
  ** @name          : LoopCtrl_Main
  ** @brief         : Loop control main program, which needs to be called repeatedly.
  ** @param         : Icap_S,Current capacitance current in amps.
  ** @param         : Ichassis_S,Current power supply output current in amps.
  ** @param         : Vcap_S,Current capacitor bank voltage in volts.
  ** @param         : Isource_S,Current DCDC input current in amps.
  ** @param         : Vbat_S,Current supply voltage in volts.
  ** @retval        : void
  ************************************************************************** 
**/
CCMRAM void LoopCtrl_Main(float Icap_S, float Ichassis_S, float Vcap_S, float Isource_S, float Vbat_S)
{
	if(LoopCtrl_GoingFlag == 1)
	{
	//MasterLoop
		//输入功率控制(PID)
		float Expect_Psource = PID_Calculate(&LoopCtrl_PID_Pchassis, LoopCtrl_ExpectPchassis, Vbat_S*Ichassis_S);
		//DCDC功率控制(FF)
		float SimpleModel = Expect_Psource/Vcap_S;
		if(SimpleModel > LoopCtrl_ffPID_Psource.OutputMax)
		{
			SimpleModel=LoopCtrl_ffPID_Psource.OutputMax;
		}
		else if(SimpleModel<LoopCtrl_ffPID_Psource.OutputMin)
		{
			SimpleModel=LoopCtrl_ffPID_Psource.OutputMin;
		}
		float EsrModel = Expect_Psource / (Vcap_S + __CAP_ESR*(SimpleModel-Expect_Icap_Last));
		float Expect_Icap = ffPID_Calculate(&LoopCtrl_ffPID_Psource, Expect_Psource, Vbat_S*Isource_S, EsrModel);
		Expect_Icap_Last = Expect_Icap;
		//电容组电流控制(PID)
		float Master_DutyRatio = PID_Calculate(&LoopCtrl_PID_Icap, Expect_Icap, Icap_S);
		
	//VcapLoop
		float VcapH_DutyRatio = PID_Calculate(&LoopCtrl_PID_VcapHigh, LoopCtrl_ExpectVcapH, Vcap_S);
		float VcapL_DutyRatio = PID_Calculate(&LoopCtrl_PID_VcapLow,  LoopCtrl_ExpectVcapL, Vcap_S);		
		
	//LoopCmpt
		float NowDutyRatio;
		if(Master_DutyRatio > VcapH_DutyRatio)
		{
			NowDutyRatio = VcapH_DutyRatio;
		}
		else if(Master_DutyRatio < VcapL_DutyRatio)
		{
			NowDutyRatio = VcapL_DutyRatio;
		}
		else
		{
			NowDutyRatio = Master_DutyRatio;
		}
		
		DCDC_PWM_DutyChange(NowDutyRatio);
	}
}

/**************************Arithmetic***************************/
/**
  ************************************************************************** 
  ** @name          : PID_Calculate
  ** @brief         : Calculation of the control volume based on the desired and feedback quantities by means of a PID algorithm.
  ** @param         : LoopCtrl_PID_Instance,Parameters and Activity Data for PID Calculation.
  ** @param         : Expect,Expectations to be met by the control system.
  ** @param         : Feedback,Actual amount of current feedback from the control system.
  ** @retval        : float,The control volume calculated by the PID algorithm.
  ************************************************************************** 
**/
CCMRAM float PID_Calculate(LoopCtrl_PID_Typedef *LoopCtrl_PID_Instance, float Expect, float Feedback)
{
	//环路计算
	float Error = Expect - Feedback;
	float Output = arm_pid_f32(&(LoopCtrl_PID_Instance->ArmPID_Instance), Error);
	
	//积分限幅
	if(LoopCtrl_PID_Instance->ArmPID_Instance.state[2] > LoopCtrl_PID_Instance->IntegMax)
	{
		LoopCtrl_PID_Instance->ArmPID_Instance.state[2] = LoopCtrl_PID_Instance->IntegMax;
	}
	else if(LoopCtrl_PID_Instance->ArmPID_Instance.state[2] < LoopCtrl_PID_Instance->IntegMin)
	{
		LoopCtrl_PID_Instance->ArmPID_Instance.state[2] = LoopCtrl_PID_Instance->IntegMin;
	}
	
	//输出限幅
	if(Output > LoopCtrl_PID_Instance->OutputMax)
	{
		return LoopCtrl_PID_Instance->OutputMax;
	}
	else if(Output < LoopCtrl_PID_Instance->OutputMin)
	{
		return LoopCtrl_PID_Instance->OutputMin;
	}
	else
	{
		return Output;
	}
}

/**
  ************************************************************************** 
  ** @name          : ffPID_Calculate
  ** @brief         : On the basis of the PID algorithm, the addition of feed-forward.
  ** @param         : LoopCtrl_ffPID_Instance,Parameters and activity data for PID calculations with control system output limits.
  ** @param         : Expect,Expectations to be met by the control system.
  ** @param         : Feedback,Actual amount of current feedback from the control system.
  ** @param         : Model,A feedforward quantity that is summed with the PID output.
  ** @retval        : float,The PID algorithm together with the feedforward calculates the amount of control.
  ************************************************************************** 
**/
CCMRAM float ffPID_Calculate(LoopCtrl_ffPID_Typedef *LoopCtrl_ffPID_Instance,
														 float Expect, float Feedback, float Model)
{
	//环路计算
	float Output = 
		PID_Calculate(&(LoopCtrl_ffPID_Instance->PID_Instance), Expect, Feedback)
		+ Model;
	
	//输出限幅
	if(Output > LoopCtrl_ffPID_Instance->OutputMax)
	{
		return LoopCtrl_ffPID_Instance->OutputMax;
	}
	else if(Output < LoopCtrl_ffPID_Instance->OutputMin)
	{
		return LoopCtrl_ffPID_Instance->OutputMin;
	}
	else
	{
		return Output;
	}
}

