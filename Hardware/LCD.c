#include "stm32f10x.h"                  // Device header
#include "MySPI.h"
#include "SYSTime.h"
#include "LCD_Data.h"
#include "W25Q64.h"

//片选引脚配置
const  GPIO_Pin_InitTypedef LCD_SS_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOA,
																							 .GPIOx = (GPIO_TypeDef *)GPIOA,
																							 .GPIO_Pin = GPIO_Pin_4};

//复位引脚配置
const  GPIO_Pin_InitTypedef LCD_RST_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOB,
																							 .GPIOx = (GPIO_TypeDef *)GPIOB,
																							 .GPIO_Pin = GPIO_Pin_1};

//数据/命令选择引脚配置
const  GPIO_Pin_InitTypedef LCD_DC_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOB,
																							 .GPIOx = (GPIO_TypeDef *)GPIOB,
																							 .GPIO_Pin = GPIO_Pin_0};

//背光控制
const  GPIO_Pin_InitTypedef LCD_BL_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOA,
																							.GPIOx = (GPIO_TypeDef *)GPIOA,
																							.GPIO_Pin = GPIO_Pin_11};
																					 
uint8_t UTF8Char_length = 0;
uint16_t ColumnAddressSet = 320;
uint16_t RowAddressSet    = 240;
W25Q64_IndexTableTypedef IndexTable_Typedef;
																							 

/*写命令
	参数：8bit命令码
*/
void LCD_WriteCom(uint8_t LCD_Com)
{
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx,LCD_DC_Typedef.GPIO_Pin,Bit_RESET);
	
	MySPI_SSStart(&LCD_SS_Typedef);
	MySPI_SwapByte(LCD_Com);
	MySPI_SSStop(&LCD_SS_Typedef);
}

/*写数据
	参数：8bit数据
*/
void LCD_WriteData(uint8_t LCD_Data)
{
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx,LCD_DC_Typedef.GPIO_Pin,Bit_SET);
	
	MySPI_SSStart(&LCD_SS_Typedef);
	MySPI_SwapByte(LCD_Data);
	MySPI_SSStop(&LCD_SS_Typedef);
}

//LCD屏幕初始化
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
	
	RCC_APB2PeriphClockCmd(LCD_BL_Typedef.RCC_APB2Periph,ENABLE);
	GPIO_InitTypeDef GPIO_BL_InitStructure;
	GPIO_BL_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_BL_InitStructure.GPIO_Pin = LCD_BL_Typedef.GPIO_Pin;
	GPIO_BL_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LCD_BL_Typedef.GPIOx,&GPIO_BL_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 50;
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	TIM_Cmd(TIM1, ENABLE); 
	
	//硬件复位：先拉低再拉高
	GPIO_WriteBit(LCD_RST_Typedef.GPIOx,LCD_RST_Typedef.GPIO_Pin,Bit_RESET);
	SYSTime_Delayms(6);
	GPIO_WriteBit(LCD_RST_Typedef.GPIOx,LCD_RST_Typedef.GPIO_Pin,Bit_SET);
	SYSTime_Delayms(6);
	
	//软件复位命令
	LCD_WriteCom(0x01);
	SYSTime_Delayms(6);
	
	//退出睡眠模式
	LCD_WriteCom(0x11);
	SYSTime_Delayms(120);
	
	
	//--------------------------------ST7789S 帧速率控制----------------------------------//
	LCD_WriteCom(0x2A); //列地址设置命令(CASET)
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData((uint8_t )(ColumnAddressSet>>8));
	LCD_WriteData((uint8_t )ColumnAddressSet);

	LCD_WriteCom(0x2B); //行地址设置命令(RASET)
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData((uint8_t )(RowAddressSet>>8));
	LCD_WriteData((uint8_t )RowAddressSet);

	LCD_WriteCom(0xB2); //门控控制命令(PORCTRL)
	LCD_WriteData(0x0C); 
	LCD_WriteData(0x0C); 
	LCD_WriteData(0x00); 
	LCD_WriteData(0x33); 
	LCD_WriteData(0x33); 

	LCD_WriteCom(0x20);//关闭显示反显命令(INVOFF)

	LCD_WriteCom(0xB7);//门控控制命令(GCTRL)
	LCD_WriteData(0x56);

//---------------------------------ST7789S 电源控制--------------------------------------//
	LCD_WriteCom(0xBB); //VCOM设置命令(VCOMS)
	LCD_WriteData(0x18); 

	LCD_WriteCom(0xc0); 		//LCM控制命令(LCMCTRL)
	LCD_WriteData(0x2C); 

	LCD_WriteCom(0xC2); 		//VDV和VRH命令使能(VDVVRHEN)
	LCD_WriteData(0x01); 

	LCD_WriteCom(0xC3); //VRH设置命令(VRHS)
	LCD_WriteData(0x1F); 

	LCD_WriteCom(0xC4); 			//VDV设置命令(VDVS)
	LCD_WriteData(0x20); 

	LCD_WriteCom(0xC6); 			//帧率控制2命令(FRCTRL2)
	LCD_WriteData(0x0F);
	
	LCD_WriteCom(0xD0);  //电源控制1命令(PWCTRL1)
	LCD_WriteData(0xA6); //AVDD/GVDD电压参数
	LCD_WriteData(0xA1); //电源控制参数
//--------------------------------ST7789S Gamma伽马校正---------------------------------------//

	LCD_WriteCom(0xe0); //正电压Gamma校正命令(PVGAMCTRL)
	LCD_WriteData(0xD0); 
	LCD_WriteData(0x0D); 
	LCD_WriteData(0x14); 
	LCD_WriteData(0x0B); 
	LCD_WriteData(0x0B); 
	LCD_WriteData(0x07); 
	LCD_WriteData(0x3A); 
	LCD_WriteData(0x44); 
	LCD_WriteData(0x50); 
	LCD_WriteData(0x08); 
	LCD_WriteData(0x13); 
	LCD_WriteData(0x13); 
	LCD_WriteData(0x2d); 
	LCD_WriteData(0x32); 

	LCD_WriteCom(0xE1); //负电压Gamma校正命令(NVGAMCTRL)
	LCD_WriteData(0xD0); 
	LCD_WriteData(0x0D); 
	LCD_WriteData(0x14); 
	LCD_WriteData(0x0B); 
	LCD_WriteData(0x0B); 
	LCD_WriteData(0x07); 
	LCD_WriteData(0x3A); 
	LCD_WriteData(0x44); 
	LCD_WriteData(0x50); 
	LCD_WriteData(0x08); 
	LCD_WriteData(0x13); 
	LCD_WriteData(0x13); 
	LCD_WriteData(0x2d); 
	LCD_WriteData(0x32); 

	LCD_WriteCom(0x36); 			//内存数据访问控制命令(MADCTL)
	LCD_WriteData(0xA0); 

	LCD_WriteCom(0x3A); 			//接口像素格式命令(COLMOD)
	LCD_WriteData(0x55);

	LCD_WriteCom(0xE7); 			//SPI2使能命令（2数据通道模式）
	LCD_WriteData(0x00); 


	LCD_WriteCom(0x21);			//显示反显开启命令(INVON)
	LCD_WriteCom(0x29); 			//显示开启命令(DISPON)
	LCD_WriteCom(0x2C);			//内存写入命令(RAMWR)
}

/*设置背光亮度，留有%10的余量
	参数：0-99亮度值
*/
void LCD_SetBLBrightnes(uint16_t Brightnes)
{
	Brightnes = (Brightnes * 90 / 100 + 10);
	TIM_SetCompare4(TIM1, Brightnes);
}

/*写颜色数据
	参数：8bit数据
*/
void LCD_WriteColorData(uint16_t color)
{
	MySPI_SwapByte((uint8_t )(color>>8));
	MySPI_SwapByte((uint8_t )color);
}

/*UTF-8解码
	参数：UTF8编码
	返回：UTF8码点
*/
uint32_t LCD_UTF8Decode(char *UTF8)
{
	uint32_t CodePoint = 0;
	UTF8Char_length = 0;
	
	while((UTF8Char_length < 4) && (*UTF8 & (0x80 >> UTF8Char_length)))
	{
		UTF8Char_length++;
	}
	
	if(UTF8Char_length == 0)
	{
		CodePoint = *UTF8;
	}else
	{
		CodePoint = *UTF8 & (0x7F >> UTF8Char_length);
		for(int8_t i = 1; i < UTF8Char_length; i++)
		{
			CodePoint = (CodePoint << 6) | (*(UTF8 + i) & 0x3F);
		}
	}
	
	return CodePoint;
}

/*画像素点
	参数1：横坐标 --->
	参数2：纵坐标 |
							  V
	参数3：像素颜色
*/
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  LCD_WriteCom(0x2A); 
  LCD_WriteData((uint8_t )(x>>8));
	LCD_WriteData((uint8_t )(x));
  LCD_WriteData((uint8_t )(x>>8));
	LCD_WriteData((uint8_t )(x));
    
  LCD_WriteCom(0x2B); 
  LCD_WriteData((uint8_t )(y >> 8));
	LCD_WriteData((uint8_t )(y));
  LCD_WriteData((uint8_t )(y >> 8));
	LCD_WriteData((uint8_t )(y));
    
  LCD_WriteCom(0x2C); // 写显存
  LCD_WriteData((uint8_t )(color >> 8)); 
	LCD_WriteData((uint8_t )(color));
}

/*设置窗口
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：横坐标截至坐标
	参数4：纵坐标截至坐标
*/
void LCD_SetWindow(uint16_t x, uint16_t y, uint16_t Stop_x, uint16_t Stop_y)
{
	Stop_x--;
	Stop_y--;
	
  LCD_WriteCom(0x2A); 
	LCD_WriteData((uint8_t )(x>>8));
	LCD_WriteData((uint8_t )x);
	LCD_WriteData((uint8_t )(Stop_x>>8));
	LCD_WriteData((uint8_t )Stop_x);

	LCD_WriteCom(0x2B);
	LCD_WriteData((uint8_t )(y>>8));
	LCD_WriteData((uint8_t )y);
	LCD_WriteData((uint8_t )(Stop_y>>8));
	LCD_WriteData((uint8_t )Stop_y);
	
	LCD_WriteCom(0x2C);
}

/*全部清屏
	参数：清屏颜色
*/
void LCD_ALLClear(uint16_t color)
{
	uint32_t ALLPixelNum = ColumnAddressSet * RowAddressSet;
	
	LCD_SetWindow(0x0000, 0x0000, ColumnAddressSet, RowAddressSet);
	
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx,LCD_DC_Typedef.GPIO_Pin,Bit_SET);
	MySPI_SSStart(&LCD_SS_Typedef);
	for(uint32_t i=0;i<ALLPixelNum;i++)
	{
		LCD_WriteColorData(color);
	}
	MySPI_SSStop(&LCD_SS_Typedef);
}

/*窗口清屏
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：横坐标截至坐标
	参数4：纵坐标截至坐标
	参数5：显示颜色
*/
void LCD_WindowClear(uint16_t x, uint16_t y, uint16_t Stop_x, uint16_t Stop_y, uint16_t color)
{
	uint32_t ALLPixelNum = (Stop_x - x) * (Stop_y - y);
	
  LCD_SetWindow(x, y, Stop_x, Stop_y);
	
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx,LCD_DC_Typedef.GPIO_Pin,Bit_SET);
	MySPI_SSStart(&LCD_SS_Typedef);
	for(uint32_t i=0;i<ALLPixelNum;i++)
	{
		LCD_WriteColorData(color);
	}
	MySPI_SSStop(&LCD_SS_Typedef);
}

/*显示字符
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：显示的字符（只支持UTF-8，如果是字符串就只显示第一个）
	参数4：像素颜色
	参数5：背景颜色
*/
void LCD_ShowChar(uint16_t x, uint16_t y, char *utf8, uint16_t color, uint16_t BackgroundColor)
{
	uint16_t PixelNum;
	uint8_t DataBuf[256] = {0};
	
	
	IndexTable_Typedef.CodePoint = LCD_UTF8Decode(utf8);
	W25Q64_GetIndexTable(&IndexTable_Typedef);
	
	PixelNum = IndexTable_Typedef.Wide * IndexTable_Typedef.Tall;
	W25Q64_ReadData(DATA_START_ADDRESS  + IndexTable_Typedef.DataOffset, DataBuf, PixelNum / 8);
	
	LCD_SetWindow(x, y, x + IndexTable_Typedef.Wide, y + IndexTable_Typedef.Tall);
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx, LCD_DC_Typedef.GPIO_Pin,Bit_SET);
	
	MySPI_SSStart(&LCD_SS_Typedef);
	for(uint8_t PixelData = 0; PixelData < (PixelNum / 8); PixelData++)
	{
		for(uint8_t i = 0; i < 8; i++)
		{
			if (DataBuf[PixelData] & (0x80>>i))
			{
				LCD_WriteColorData(color);
			}else
			{
				LCD_WriteColorData(BackgroundColor);
			}
		}
	}
	MySPI_SSStop(&LCD_SS_Typedef);
}

/*显示字符串
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：显示的字符串
	参数4：像素颜色
	参数5：背景颜色
*/
void LCD_ShowString(uint16_t x, uint16_t y, char *utf8, uint16_t color, uint16_t BackgroundColor)
{
	uint16_t X_Offset = 0;
	
	for(uint8_t i = 0;utf8[i] != '\0';i++)
	{
		LCD_ShowChar(x + X_Offset, y, &utf8[i], color, BackgroundColor);
		if(UTF8Char_length != 0)
		{
			i += UTF8Char_length - 1;
		}
		X_Offset += IndexTable_Typedef.Wide;
	}
}

/*显示无符号数字
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：显示的数字,根据ASCII码
	参数4：像素颜色
	参数5：背景颜色
*/
void LCD_ShowUnsigNum(uint16_t x, uint16_t y, uint32_t Num, uint16_t color,  uint16_t BackgroundColor)
{
	uint8_t Num_length;
	uint32_t Places;
	uint32_t num;
	char Digit[2];
	
	
	Digit[1] = '\0';
	if(Num == 0)
	{
		Digit[0] = '0';
		LCD_ShowChar(x, y, Digit, color, BackgroundColor);
	}else
	{
		num = Num;
		for(Num_length = 0;num != 0;Num_length++)
		{
			num /= 10;
		}
		
		for(uint8_t i = 0;i < Num_length;i++)
		{
			Places = 1;
			for(uint8_t n = 0;n < Num_length - i - 1;n++)
			{
				Places *= 10;
			}
			Digit[0] = ((Num / Places) % 10) + '0';
			LCD_ShowChar(x + (i * 16), y, Digit, color, BackgroundColor);
		}
	}
}

/*显示32*64的时钟数字
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：显示的数字
	参数4：像素颜色
	参数5：背景颜色
*/
void LCD_ShowTimeNum(uint16_t x, uint16_t y, uint8_t Num, uint16_t color,  uint16_t BackgroundColor)
{
	uint32_t Row_Pixel;
	
	LCD_SetWindow(x, y, x + 32, y + 64);
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx,LCD_DC_Typedef.GPIO_Pin,Bit_SET);
	MySPI_SSStart(&LCD_SS_Typedef);
	
	for(uint8_t row=0;row<64;row++)
	{
		Row_Pixel = (uint32_t )(LCD_FontNum32x64[Num][4 * row] << 24)
							| (uint32_t )(LCD_FontNum32x64[Num][4 * row + 1] << 16)
							| (uint32_t )(LCD_FontNum32x64[Num][4 * row + 2] << 8)
							| (uint32_t )(LCD_FontNum32x64[Num][4 * row + 3]);
		for (uint8_t col=0;col<32;col++)
		{
			if (Row_Pixel & (0x80000000>>col))
			{
				LCD_WriteColorData(color);
			}else
			{
				LCD_WriteColorData(BackgroundColor);
			}
		}
	}
	MySPI_SSStop(&LCD_SS_Typedef);
}

/*显示图案
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：横坐标截至坐标
	参数4：纵坐标截至坐标
	参数5：图案数组
	参数6：像素颜色
	参数5：背景颜色
*/
void LCD_ShowPattern(uint16_t x, uint16_t y,  uint16_t Stop_x, uint16_t Stop_y,const uint8_t *Pattern, uint16_t color,  uint16_t BackgroundColor)
{
	LCD_SetWindow(x, y, Stop_x, Stop_y);
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx,LCD_DC_Typedef.GPIO_Pin,Bit_SET);
	MySPI_SSStart(&LCD_SS_Typedef);
	
	for(uint16_t DataSub = 0; DataSub < ((Stop_x - x) * (Stop_y - y)) / 8; DataSub++)
	{
		for(uint8_t i = 0; i < 8 ; i++)
		{
			if (Pattern[DataSub] & (0x80>>i))
			{
				LCD_WriteColorData(color);
			}else
			{
				LCD_WriteColorData(BackgroundColor);
			}
		}
	}
	MySPI_SSStop(&LCD_SS_Typedef);
}

/*返回字符串X轴的像素宽度
	参数：被测字符串
	返回：X轴的像素宽度
*/
uint16_t LCD_GetString_X_Offset(char *Data)
{
	uint16_t X_Offset = 0;
	
	for(uint16_t i = 0; Data[i] != '\0'; i++)
	{
		IndexTable_Typedef.CodePoint = LCD_UTF8Decode(&Data[i]);
		W25Q64_GetIndexTable(&IndexTable_Typedef);
		
		X_Offset += IndexTable_Typedef.Wide;
		
		if(UTF8Char_length != 0)
		{
				i += UTF8Char_length - 1;
		}
	}
	return X_Offset;
}


/*显示字符的一部分，用于左右滚动显示
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：显示的字符（只支持UTF-8）
	参数4：从字符第几列开始显示
	参数5：显示多少列
	参数6：像素颜色
	参数7：背景颜色
*/
void LCD_ShowPortionChar(uint16_t x, uint16_t y, char *utf8, uint16_t StartColumn, uint16_t ShowWide, uint16_t color, uint16_t BackgroundColor)
{
	uint16_t PixelNum;
	uint8_t DataBuf[256] = {0};
	
	IndexTable_Typedef.CodePoint = LCD_UTF8Decode(utf8);
	W25Q64_GetIndexTable(&IndexTable_Typedef);
	
	PixelNum = IndexTable_Typedef.Wide * IndexTable_Typedef.Tall;
	W25Q64_ReadData(DATA_START_ADDRESS + IndexTable_Typedef.DataOffset, DataBuf, PixelNum / 8);
	
	
	LCD_SetWindow(x, y, x + ShowWide, y + IndexTable_Typedef.Tall);
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx, LCD_DC_Typedef.GPIO_Pin, Bit_SET);
	MySPI_SSStart(&LCD_SS_Typedef);
	for(uint16_t row = 0; row < IndexTable_Typedef.Tall; row++)
	{
		for(uint16_t Col = StartColumn; Col < StartColumn + ShowWide; Col++)
		{
			uint8_t byte = DataBuf[row * (IndexTable_Typedef.Wide / 8) + Col / 8];
			if(byte & (0x80 >> (Col % 8)))
			{
				LCD_WriteColorData(color);
			}
			else
			{
				LCD_WriteColorData(BackgroundColor);
			}
		}
	}
	MySPI_SSStop(&LCD_SS_Typedef);
}

/*滚动显示字符串，调用一次刷新一帧
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：滚动像素显示的宽度
	参数4：字符串
	参数5：滚动偏移量（给一个变量，每次执行就++并回绕）
	参数6：像素颜色
	参数7：背景颜色
*/
void LCD_ShowStringRoll(uint16_t x, uint16_t y, uint16_t ShowWide, char *Data, uint16_t *offset, uint16_t color, uint16_t BackgroundColor)
{
	int16_t OriginalOffset = 0;
	int16_t Left;
	int16_t Right;
	uint16_t char_X_Offset;
	uint16_t char_StartColumn;
	uint16_t char_ShowWide;
	
	
	for(uint16_t i = 0;Data[i] != '\0';i++)
	{
		IndexTable_Typedef.CodePoint = LCD_UTF8Decode(&Data[i]);
		W25Q64_GetIndexTable(&IndexTable_Typedef);
		
		Left  = OriginalOffset - *offset;
		Right = OriginalOffset + IndexTable_Typedef.Wide - *offset;

		if(Right > 0 && Left < ShowWide)
		{
			char_X_Offset    = (Left < 0) ? 0 : Left;
			char_StartColumn = (Left < 0) ? (-Left) : 0;
			char_ShowWide    = ((Right > ShowWide) ? ShowWide : Right) - char_X_Offset;

			LCD_ShowPortionChar(x + char_X_Offset, y, &Data[i], char_StartColumn, char_ShowWide, color, BackgroundColor);
		}
		
		if(UTF8Char_length != 0)
		{
			i += UTF8Char_length - 1;
		}
		OriginalOffset += IndexTable_Typedef.Wide;
	}
	
	(*offset)++;
	if(*offset >= OriginalOffset)
	{
		*offset = 0;
	}
}

//void LCD_SetBrightness(uint8_t Brightness)
//{
//	LCD_WriteCom(0x53);
//	LCD_WriteData(0x2C);
//	
//	LCD_WriteCom(0x51);
//	LCD_WriteData((uint8_t )Brightness);
//}

//uint16_t LCD_GetBrightness(void)
//{
//	uint16_t Brightness;
//	
//	LCD_WriteCom(0x53);
//	LCD_WriteData(0x2C);
//	
//	GPIO_WriteBit(LCD_DC_Typedef.GPIOx, LCD_DC_Typedef.GPIO_Pin, Bit_RESET);
//	MySPI_SSStart(&LCD_SS_Typedef);
//	MySPI_SwapByte(0x52);

//	GPIO_WriteBit(LCD_DC_Typedef.GPIOx, LCD_DC_Typedef.GPIO_Pin, Bit_SET);
//	MySPI_SwapByte(0xFF);
//	Brightness = MySPI_SwapByte(0xFF);
//	MySPI_SSStop(&LCD_SS_Typedef);
//	
//	return Brightness;
//}
