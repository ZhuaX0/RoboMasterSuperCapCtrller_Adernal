#include "LowOptZone.h"

CCMRAM void LowOpt_BlockusDelay(uint8_t us)
{
	uint16_t i;
	for(i=0; i<us*10; i++)
	{
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
	}
}

