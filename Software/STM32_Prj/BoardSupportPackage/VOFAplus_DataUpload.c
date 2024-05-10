#include "VOFAplus_DataUpload.h"
#include "usart.h"
#include "string.h"

float VOFA_UploadBuffer[32];

CCMRAM HAL_StatusTypeDef VOFA_DataJustFloat_Upload(float *ArrayUpload, uint8_t ArrayLength)
{
	if(ArrayLength > 32)
	{
		return HAL_ERROR;
	}
	
	memcpy(VOFA_UploadBuffer, ArrayUpload, sizeof(float)*ArrayLength);
	uint32_t EndFrame = 0x7F800000;
	
//	HAL_UART_Transmit(&huart1, (uint8_t *)VOFA_UploadBuffer, ArrayLength*sizeof(float), 1);
//	HAL_UART_Transmit(&huart1, (uint8_t *)&EndFrame, sizeof(uint32_t), 1);
	
	uint8_t i;
	for(i=0; i<ArrayLength*sizeof(float); i++)
	{
		while(LL_USART_IsActiveFlag_TXE(USART1)==0);
		LL_USART_TransmitData8(USART1, ((uint8_t *)VOFA_UploadBuffer)[i]);
	}
	
	for(i=0; i<sizeof(uint32_t); i++)
	{
		while(LL_USART_IsActiveFlag_TXE(USART1)==0);
		LL_USART_TransmitData8(USART1, ((uint8_t *)(&EndFrame))[i]);
	}

	return HAL_OK;
}

