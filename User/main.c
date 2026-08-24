#include "stm32f10x.h"                  // Device header
#include "LCD.h"
#include "SYSTime.h"
#include "Key.h"

int main()
{
	uint8_t KeyStatus[6] = {0};
	Key_InitTypedef Key[] = {{RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_8, 1000, 0, 0, 0},
													{RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_9, 1000, 0, 0, 0},};
	
	SYSTime_Init();
	LCD_Init();
	
	for(uint8_t i = 0;i < 2;i++)
	{
		Key_Init(&Key[i]);
	}
	
	while(1)
	{
		Key_StatusPoll(&Key[0]);
		KeyStatus[0] = Key_Get(&Key[0]);
		if(KeyStatus[0] == 1) LCD_ShowChinese(10,10,"²Ë",0xffff);
		if(KeyStatus[0] == 2) LCD_ShowChinese(10,42,"µ¥",0xffff);
		if(KeyStatus[0] == 3) LCD_ShowChinese(74,10,"ÕÂ",0xffff);
	}
	
}

