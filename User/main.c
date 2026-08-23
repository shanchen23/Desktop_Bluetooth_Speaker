#include "stm32f10x.h"                  // Device header
#include "LCD.h"
#include "SYSTime.h"

int main()
{
	SYSTime_Init();
	LCD_Init();
	
	while(1)
	{
		LCD_ShowChar(10,10,'A',0xffff);
		LCD_ShowString(10,42,"ABCDEFG123",0xffff);
		LCD_ShowSignedNum(250,75,00000,0xffff);
		LCD_ShowChinese(10,107,"ียบฃาใ",0xffff);
		LCD_ShowString(10,139,"??",0xffff);
		
	}
	
}

