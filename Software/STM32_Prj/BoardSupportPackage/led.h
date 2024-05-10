//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//中景园电子
//店铺地址：http://shop73023976.taobao.com/?spm=2013.1.0.0.M4PqC2
//
//  文 件 名   : main.c
//  版 本 号   : v2.0
//  作    者   : HuangKai
//  生成日期   : 2018-03-29
//  最近修改   : 
//  功能描述   : OLED 4接口演示例程(STM32F0系列)
//              说明: 
//              ----------------------------------------------------------------
//              GND    电源地
//              VCC  接3.3v电源
//              D0   接PA9（CLK）
//              D1   接PA7（MOSI）
//              RES  接PA6
//              DC   接PA5
//              CS   接PA4               
//              ----------------------------------------------------------------
// 修改历史   :
// 日    期   : 
// 作    者   : HuangKai
// 修改内容   : 创建文件
//版权所有，盗版必究。
//Copyright(C) 中景园电子2014/3/16
//All rights reserved
//******************************************************************************/
//The x-direction limit value is 121
//The y-direction limit value is 6
//The minimum width of the character x direction is 7(recommand 8
//The minimum width of the character y direction is 2

#ifndef __LED_H
#define __LED_H

#include "main.h"
#define LED_GPIO_CLKA   RCC_AHBPeriph_GPIOA 
#define LED_GPIO_CLKB   RCC_AHBPeriph_GPIOB
#define LED_PORT   	   GPIOB
#define LED_PIN        GPIO_Pin_1
#define Max_Column	128
#define Max_Row		64
#define SIZE 16
void LED_Init(void);
void LED_ON(void);
void LED_OFF(void);

#define OLED_RST_Clr() OLED_RES_GPIO_Port->BRR=OLED_RES_Pin//RES
#define OLED_RST_Set() OLED_RES_GPIO_Port->BSRR=OLED_RES_Pin

#define OLED_DC_Clr() OLED_DC_GPIO_Port->BRR=OLED_DC_Pin//DC
#define OLED_DC_Set() OLED_DC_GPIO_Port->BSRR=OLED_DC_Pin
 		     
#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据


//OLED控制用函数
void OLED_WR_Byte(uint8_t dat,uint8_t cmd);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);	   							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t);
void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot);
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);
void OLED_ShowString(uint8_t x,uint8_t y, uint8_t *p);	 
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t no);
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);
#endif  
