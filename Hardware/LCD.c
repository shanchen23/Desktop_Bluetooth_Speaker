#include "stm32f10x.h"                  // Device header
#include "MySPI.h"

//片选引脚
const  MySPI_SS_InitTypedef LCD_SS_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOA,
																						 .GPIOx = (GPIO_TypeDef *)GPIOA,
																						 .GPIO_Pin = GPIO_Pin_4};

//复位引脚
const  MySPI_SS_InitTypedef LCD_RST_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOB,
																						 .GPIOx = (GPIO_TypeDef *)GPIOB,
																						 .GPIO_Pin = GPIO_Pin_1};

//数据/命令选择引脚
const  MySPI_SS_InitTypedef LCD_DC_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOB,
																						 .GPIOx = (GPIO_TypeDef *)GPIOB,
																						 .GPIO_Pin = GPIO_Pin_0};

void LCD_WriteCom(uint8_t LCD_Com);
void LCD_WriteData(uint8_t LCD_Data);

//LCD屏初始化
void LCD_Init(void)
{
	MySPI_Init(&LCD_SS_Typedef);
	
	RCC_APB2PeriphClockCmd(LCD_RST_Typedef.RCC_APB2Periph,ENABLE);
	GPIO_InitTypeDef GPIO_RST_InitStructure;
	GPIO_RST_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_RST_InitStructure.GPIO_Pin = LCD_RST_Typedef.GPIO_Pin;
	GPIO_RST_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD_RST_Typedef.GPIOx,&GPIO_RST_InitStructure);
	
	RCC_APB2PeriphClockCmd(LCD_DC_Typedef.RCC_APB2Periph,ENABLE);
	GPIO_InitTypeDef GPIO_DC_InitStructure;
	GPIO_DC_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_DC_InitStructure.GPIO_Pin = LCD_DC_Typedef.GPIO_Pin;
	GPIO_DC_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD_DC_Typedef.GPIOx,&GPIO_DC_InitStructure);
	
	
	
}

/*写命令
	参数：8bit的命令
*/
void LCD_WriteCom(uint8_t LCD_Com)
{
	MySPI_SSStart(&LCD_SS_Typedef);
	
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx,LCD_DC_Typedef.GPIO_Pin,Bit_RESET);
	MySPI_SwapByte(LCD_Com);
	
	MySPI_SSStop(&LCD_SS_Typedef);
}

/*写数据
	参数：8bit的数据
*/
void LCD_WriteData(uint8_t LCD_Data)
{
	MySPI_SSStart(&LCD_SS_Typedef);
	
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx,LCD_DC_Typedef.GPIO_Pin,Bit_SET);
	MySPI_SwapByte(LCD_Data);
	
	MySPI_SSStop(&LCD_SS_Typedef);
}

