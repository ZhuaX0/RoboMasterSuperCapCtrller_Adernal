#ifndef __DCDC
#define __DCDC

#include "main.h"

HAL_StatusTypeDef DCDC_WorkStart(float DutyRatioInit);
CCMRAM HAL_StatusTypeDef DCDC_WorkStop(void);

CCMRAM void DCDC_PWM_DutyChange(float DutyRatio);

#endif

