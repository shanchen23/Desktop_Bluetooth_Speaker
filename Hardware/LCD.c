#include "stm32f10x.h"                  // Device header
#include "MySPI.h"
#include "SYSTime.h"
#include "LCD.h"
#include "LCD_Data.h"

//片选引脚配置
const  MySPI_SS_InitTypedef LCD_SS_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOA,
																							 .GPIOx = (GPIO_TypeDef *)GPIOA,
																							 .GPIO_Pin = GPIO_Pin_4};

//复位引脚配置
const  MySPI_SS_InitTypedef LCD_RST_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOB,
																							 .GPIOx = (GPIO_TypeDef *)GPIOB,
																							 .GPIO_Pin = GPIO_Pin_1};

//数据/命令选择引脚配置
const  MySPI_SS_InitTypedef LCD_DC_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOB,
																							 .GPIOx = (GPIO_TypeDef *)GPIOB,
																							 .GPIO_Pin = GPIO_Pin_0};

uint16_t ColumnAddressSet = 320;
uint16_t RowAddressSet    = 240;
																							 
void LCD_WriteCom(uint8_t LCD_Com);
void LCD_WriteData(uint8_t LCD_Data);

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

/*写命令
	参数：8bit命令码
*/
void LCD_WriteCom(uint8_t LCD_Com)
{
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx,LCD_DC_Typedef.GPIO_Pin,Bit_RESET);
	
	MySPI_SSStart(&LCD_SS_Typedef);
	MySPI_SendByte(LCD_Com);
	MySPI_SSStop(&LCD_SS_Typedef);
}

/*写数据
	参数：8bit数据
*/
void LCD_WriteData(uint8_t LCD_Data)
{
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx,LCD_DC_Typedef.GPIO_Pin,Bit_SET);
	
	MySPI_SSStart(&LCD_SS_Typedef);
	MySPI_SendByte(LCD_Data);
	MySPI_SSStop(&LCD_SS_Typedef);
}

/*写颜色数据
	参数：8bit数据
*/
void LCD_WriteColorData(uint16_t color)
{
		MySPI_SendByte((uint8_t )(color>>8));
		MySPI_SendByte((uint8_t )color);
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
void LCD_ClearWindow(uint16_t x, uint16_t y, uint16_t Stop_x, uint16_t Stop_y, uint16_t color)
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
	参数3：显示的字符,根据ASCII码
	参数4：像素颜色
*/
void LCD_ShowChar(uint16_t x, uint16_t y, char Char, uint16_t color)
{
	uint16_t Row_Pixel;
	
	LCD_SetWindow(x, y, x + 16, y + 32);
	GPIO_WriteBit(LCD_DC_Typedef.GPIOx,LCD_DC_Typedef.GPIO_Pin,Bit_SET);
	MySPI_SSStart(&LCD_SS_Typedef);
	
	for(uint8_t row=0;row<32;row++)
	{
		Row_Pixel = (uint16_t )(LCD_Font16x32[Char - ' '][2 * row] << 8)
							| (uint16_t )LCD_Font16x32[Char - ' '][2 * row + 1];
		for (uint8_t col=0;col<16;col++)
		{
			if (Row_Pixel & (0x8000>>col))
			{
				LCD_WriteColorData(color);
			}else
			{
				LCD_WriteColorData(0x0000);
			}
		}
	}
	MySPI_SSStop(&LCD_SS_Typedef);
}

/*显示字符串
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：显示的字符串,根据ASCII码
	参数4：像素颜色
*/
void LCD_ShowString(uint16_t x, uint16_t y, char *Char, uint16_t color)
{
	for(uint8_t i = 0;Char[i] != '\0';i++)
	{
		LCD_ShowChar(x + (i * 16), y, Char[i],color);
	}
}

/*显示无符号数字
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：显示的数字,根据ASCII码
	参数4：像素颜色
*/
void LCD_ShowUnsignNum(uint16_t x, uint16_t y, uint32_t Num, uint16_t color)
{
	uint8_t Num_length;
	uint32_t Places;
	uint32_t num;
	
	if(Num == 0)
	{
		LCD_ShowChar(x,y,'0',color);
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
			LCD_ShowChar(x + (i * 16), y, ((Num / Places) % 10) + '0',color);
		}
	}
}

/*显示汉字
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：显示的汉字,用数组存储
	参数4：像素颜色
*/
void LCD_ShowChinese(uint16_t x, uint16_t y, uint8_t ZW[], uint16_t color)
{
	uint32_t Row_Pixel;
	uint16_t Indexes_num;
	uint8_t Fleg_Indexes;
	uint8_t ZW_CharLength;
	uint8_t ZW_Num;
	
	for(ZW_CharLength = 0;ZW[ZW_CharLength] != '\0';ZW_CharLength++);
	
	for(ZW_Num = 0;ZW_Num < (ZW_CharLength / 2); ZW_Num++)
	{
		Fleg_Indexes = 0;
		
		for(Indexes_num = 0;LCD_ZWFont32x32[Indexes_num].Indexes[0] != 0x00;Indexes_num++)
		{
			if((LCD_ZWFont32x32[Indexes_num].Indexes[0] == ZW[(ZW_Num * 2)]) && (LCD_ZWFont32x32[Indexes_num].Indexes[1] == ZW[(ZW_Num * 2) + 1]))
			{
				Fleg_Indexes = 1;
				break;
			}
		}
		
		LCD_SetWindow(x, y, x + 32, y + 32);
		GPIO_WriteBit(LCD_DC_Typedef.GPIOx,LCD_DC_Typedef.GPIO_Pin,Bit_SET);
		MySPI_SSStart(&LCD_SS_Typedef);
		
		if(Fleg_Indexes == 0)
		{
			LCD_ShowString(x + (ZW_Num * 32), y, "??",0xffff);
		}else
		{
			for(uint8_t row=0;row<32;row++)
			{
				Row_Pixel = (uint32_t )(LCD_ZWFont32x32[Indexes_num].Data[4 * row] << 24)
									| (uint32_t )(LCD_ZWFont32x32[Indexes_num].Data[4 * row + 1] << 16)
									| (uint32_t )(LCD_ZWFont32x32[Indexes_num].Data[4 * row + 2] << 8)
									| (uint32_t ) LCD_ZWFont32x32[Indexes_num].Data[4 * row + 3];
				
				for (uint8_t col=0;col<32;col++)
				{
					if (Row_Pixel & (0x80000000>>col))
					{
						LCD_WriteColorData(color);
					}else
					{
						LCD_WriteColorData(0x0000);
					}
				}
			}
		}
		MySPI_SSStop(&LCD_SS_Typedef);
	}
}

/*显示32*64的时钟数字
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：显示的数字
	参数4：像素颜色
*/
void LCD_ShowTimeNum(uint16_t x, uint16_t y, uint8_t Num, uint16_t color)
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
				LCD_WriteColorData(0x0000);
			}
		}
	}
	MySPI_SSStop(&LCD_SS_Typedef);
}

