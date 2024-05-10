#include "LED.h"
#include "oledfont.h"  
#include "spi.h"
#include "LowOptZone.h"

SPI_TypeDef *OLED_SPI = SPI1;

void LED_Init(void)
{
	//ç¦ç”¨LLåº“æ—¶æ³¨é‡ŠæŽ‰ä¸‹è¡Œä»£ç 
	LL_SPI_Enable(OLED_SPI);
	
	OLED_RST_Set();
	HAL_Delay(100);
	OLED_RST_Clr();
	HAL_Delay(200);
	OLED_RST_Set(); 
					  
	OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
	OLED_WR_Byte(0x02,OLED_CMD);//---set low column address
	OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
	OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
	OLED_WR_Byte(0xff,OLED_CMD); // Set SEG Output Current Brightness
	OLED_WR_Byte(0xA0,OLED_CMD);//--Set SEG/Column Mapping     0xa0×óÓÒ·´ÖÃ 0xa1Õý³£
	OLED_WR_Byte(0xC0,OLED_CMD);//Set COM/Row Scan Direction   0xc0ÉÏÏÂ·´ÖÃ 0xc8Õý³£
	OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
	OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
	OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OLED_WR_Byte(0x00,OLED_CMD);//-not offset
	OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
	OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
	OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
	OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
	OLED_WR_Byte(0x12,OLED_CMD);
	OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
	OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
	OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
	OLED_WR_Byte(0x02,OLED_CMD);//
	OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
	OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
	OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
	OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7) 
	OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel
	
	OLED_WR_Byte(0xAF,OLED_CMD); /*display ON*/ 
	OLED_Clear();
	OLED_Set_Pos(0,0); 	
}

//ÏòSSD1106Ð´ÈëÒ»¸ö×Ö½Ú¡£
//dat:ÒªÐ´ÈëµÄÊý¾Ý/ÃüÁî
//cmd:Êý¾Ý/ÃüÁî±êÖ¾ 0,±íÊ¾ÃüÁî;1,±íÊ¾Êý¾Ý;
void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{	
	if(cmd)
	{
	  OLED_DC_Set();
	}
	else
	{		
	  OLED_DC_Clr();		  
	}

	while(LL_SPI_IsActiveFlag_BSY(OLED_SPI)==1);
	LL_SPI_TransmitData8(OLED_SPI, dat);
	LowOpt_BlockusDelay(4);
	
	//åœ¨-Ofastä¸‹ä½¿ç”¨DMAä¼šå¯¼è‡´ä¼ è¾“æ•°æ®ä¸å®Œæ•´
	//ä½¿ç”¨LLåº“ä¼šå¯¼è‡´å±å¹•æ— æ³•æ˜¾ç¤ºæœ‰æ•ˆä¿¡æ¯
	//uint8_t DataTx = dat;
	//HAL_SPI_Transmit(&hspi1, &DataTx, 1, 1);
	
	OLED_DC_Set();   	  
}

void OLED_Set_Pos(unsigned char x, unsigned char y) 
{ 
	OLED_WR_Byte(0xb0+y,OLED_CMD);
	OLED_WR_Byte((((x+2)&0xf0)>>4)|0x10,OLED_CMD);
	OLED_WR_Byte(((x+2)&0x0f),OLED_CMD); 
}   	  
//¿ªÆôOLEDÏÔÊ¾    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDCÃüÁî
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//¹Ø±ÕOLEDÏÔÊ¾     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDCÃüÁî
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}		   			 
//ÇåÆÁº¯Êý,ÇåÍêÆÁ,Õû¸öÆÁÄ»ÊÇºÚÉ«µÄ!ºÍÃ»µãÁÁÒ»Ñù!!!	  
void OLED_Clear(void)  
{  
	uint8_t i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //ÉèÖÃÒ³µØÖ·£¨0~7£©
		OLED_WR_Byte (0x02,OLED_CMD);      //ÉèÖÃÏÔÊ¾Î»ÖÃ¡ªÁÐµÍµØÖ·
		OLED_WR_Byte (0x10,OLED_CMD);      //ÉèÖÃÏÔÊ¾Î»ÖÃ¡ªÁÐ¸ßµØÖ·   
		for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA); 
	} //¸üÐÂÏÔÊ¾
}


//ÔÚÖ¸¶¨Î»ÖÃÏÔÊ¾Ò»¸ö×Ö·û,°üÀ¨²¿·Ö×Ö·û
//x:0~127
//y:0~63
//mode:0,·´°×ÏÔÊ¾;1,Õý³£ÏÔÊ¾				 
//size:Ñ¡Ôñ×ÖÌå 16/12 
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr)
{      	
	unsigned char c=0,i=0;	
		c=chr-' ';//µÃµ½Æ«ÒÆºóµÄÖµ			
		if(x>Max_Column-1){x=0;y=y+2;}
		if(SIZE ==16)
			{
			OLED_Set_Pos(x,y);	
			for(i=0;i<8;i++)
			OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
			OLED_Set_Pos(x,y+1);
			for(i=0;i<8;i++)
			OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
			}
			else {	
				OLED_Set_Pos(x,y+1);
				for(i=0;i<6;i++)
				OLED_WR_Byte(F6x8[c][i],OLED_DATA);
				
			}
}
//m^nº¯Êý
uint32_t oled_pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}				  
//ÏÔÊ¾2¸öÊý×Ö
//x,y :Æðµã×ø±ê	 
//len :Êý×ÖµÄÎ»Êý
//size:×ÖÌå´óÐ¡
//mode:Ä£Ê½	0,Ìî³äÄ£Ê½;1,µþ¼ÓÄ£Ê½
//num:ÊýÖµ(0~4294967295);	 		  
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size/2)*t,y,' ');
				continue;
			}else enshow=1; 
		 	 
		}
	 	OLED_ShowChar(x+(size/2)*t,y,temp+'0'); 
	}
} 
//ÏÔÊ¾Ò»¸ö×Ö·ûºÅ´®
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{		OLED_ShowChar(x,y,chr[j]);
			x+=8;
		if(x>120){x=0;y+=2;}
			j++;
	}
}
//ÏÔÊ¾ºº×Ö
//void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t no)
//{      			    
//	uint8_t t,adder=0;
//	OLED_Set_Pos(x,y);	
//    for(t=0;t<16;t++)
//		{
//				OLED_WR_Byte(Hzk[2*no][t],OLED_DATA);
//				adder+=1;
//     }	
//		OLED_Set_Pos(x,y+1);	
//    for(t=0;t<16;t++)
//			{	
//				OLED_WR_Byte(Hzk[2*no+1][t],OLED_DATA);
//				adder+=1;
//      }					
//}
/***********¹¦ÄÜÃèÊö£ºÏÔÊ¾ÏÔÊ¾BMPÍ¼Æ¬128¡Á64ÆðÊ¼µã×ø±ê(x,y),xµÄ·¶Î§0¡«127£¬yÎªÒ³µÄ·¶Î§0¡«7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{ 	
 unsigned int j=0;
 unsigned char x,y;
  
  if(y1%8==0) y=y1/8;      
  else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
    for(x=x0;x<x1;x++)
	    {      
	    	OLED_WR_Byte(BMP[j++],OLED_DATA);	    	
	    }
	}
} 