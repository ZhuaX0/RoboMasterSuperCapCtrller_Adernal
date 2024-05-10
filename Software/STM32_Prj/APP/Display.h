#ifndef __DISPLAY
#define __DISPLAY

#include "main.h"

void Display_BootUpInterface(void);
void Display_LogoInterface(void);
void Display_PrepareInterface(void);
CCMRAM void Display_Vcap_DataUpdateInterface(float Vcap);
CCMRAM void Display_Mode_DataUpdateInterface(uint8_t Mode);
CCMRAM void Display_Exceed_DataUpdateInterface(uint8_t ExceedGet);
CCMRAM void Display_SafetyCode_DataUpdateInterface(uint8_t *SafetyCode);

#endif

