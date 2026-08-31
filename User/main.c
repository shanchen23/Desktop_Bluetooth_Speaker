#include "stm32f10x.h"                  // Device header
#include "LCD.h"
#include "SYSTime.h"
#include "Key.h"

void ShowTime(uint16_t x, uint16_t y, TimeDateTypedef *Time, uint16_t color);

int main()
{
	TimeDateTypedef Time_structure;
	uint8_t KeyStatus[6] = {0};
	Key_InitTypedef Key[] = {{RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_8, 1000, 0, 0, 0},
													{RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_9, 1000, 0, 0, 0},};
	
	SYSTime_Init();
	MyRTC_Init();
	LCD_Init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);											
	for(uint8_t i = 0;i < 2;i++)
	{
		Key_Init(&Key[i]);
	}
	
	
	Time_structure.tm_year = 2026;
	Time_structure.tm_mon = 12;
	Time_structure.tm_mday = 31;
	Time_structure.tm_hour = 23;
	Time_structure.tm_min = 59;
	Time_structure.tm_sec = 55;
	
	MyRTC_SetTime(&Time_structure);
	LCD_ALLClear(0x0000);
	while(1)
	{
		Time_structure = MyRTC_ReadTime();
		Key_StatusPoll(&Key[0]);
		KeyStatus[0] = Key_Get(&Key[0]);
		if(KeyStatus[0] == 1) LCD_ShowChinese(10,10,"菜",0xffff);
		if(KeyStatus[0] == 2) LCD_ShowChinese(10,42,"单",0xffff);
		
		ShowTime(56,20,&Time_structure,0xffff);
	}
	
}






/*显示桌面的时间
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：含有时间信息的结构体
	参数4：像素颜色
*/
void ShowTime(uint16_t x, uint16_t y, TimeDateTypedef *Time, uint16_t color)
{	
	uint8_t seat_mon;
	uint8_t seat_mday;
	static uint8_t Record_mon = 0;
	static uint8_t Record_mday = 0;
	
	LCD_ShowTimeNum(x + 16, y, Time->tm_hour/10, color);
	LCD_ShowTimeNum(x + 16 + (1 * 32), y, Time->tm_hour%10, color);
	LCD_ShowTimeNum(x + 16 + (2 * 32), y, 10, color);
	LCD_ShowTimeNum(x + 16 + (3 * 32), y, Time->tm_min/10, color);
	LCD_ShowTimeNum(x + 16 + (4 * 32), y, Time->tm_min%10, color);
	LCD_ShowUnsignNum(x + 16 + (5 * 32), y + 32, Time->tm_sec / 10, color);
	LCD_ShowUnsignNum(x + 16 + (5 * 32 + 16), y + 32, Time->tm_sec % 10, color);
	
	LCD_ShowUnsignNum(x, y + 64, Time->tm_year, color);
	LCD_ShowChinese(x + (4 * 16), y + 64, "年", color);
	seat_mon = (Time->tm_mon > 9) ? 16 : 0;
	LCD_ShowUnsignNum(x + (6 * 16), y + 64, Time->tm_mon, color);
	LCD_ShowChinese(x + (7 * 16) + seat_mon, y + 64, "月", color);
	seat_mday = (Time->tm_mday > 9) ? 16 : 0;
	
	if((Record_mon != Time->tm_mon) || (Record_mday != Time->tm_mday))
	{
		LCD_ClearWindow(x + 190, y + 64, x + 224, y + 96, 0x0000);
		Record_mon = Time->tm_mon;
		Record_mday = Time->tm_mday;
	}
	
	LCD_ShowUnsignNum(x + (9* 16) + seat_mon, y + 64, Time->tm_mday, color);
	LCD_ShowChinese(x + (10 * 16) + seat_mon + seat_mday, y + 64, "日", color);
}
