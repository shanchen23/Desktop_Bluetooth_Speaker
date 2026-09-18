#include "stm32f10x.h"                  // Device header
#include "Time.h"
#include "LCD.h"
#include "SYSTime.h"
#include "Key.h"
#include "DH11.h"
#include "W25Q64.h"
#include "BT301M.h"

void ShowTime(uint16_t x, uint16_t y, TimeDateTypedef *Time, uint16_t color, uint16_t BackgroundColor);
void ShowTempAndHum(uint16_t x, uint16_t y, uint16_t color, uint16_t BackgroundColor);

int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		
	
	
	TimeDateTypedef Time_structure;
	uint8_t MOD_Status = 0;
	uint32_t DHT11_ReadTime = 0;
	Key_InitTypedef Key_Switch = {RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0, 1000, 0, 0, 0};
	Key_InitTypedef Key_Up     = {RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_8, 1000, 0, 0, 0};
	Key_InitTypedef Key_Bown   = {RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_9, 1000, 0, 0, 0};
	Key_InitTypedef Key_Left   = {RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_12, 1000, 0, 0, 0};
	Key_InitTypedef Key_Right  = {RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_13, 1000, 0, 0, 0};
	Key_InitTypedef Key_OK     = {RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_14, 1000, 0, 0, 0};
	
	Time_Init();
	SYSTime_Init();
	MyRTC_Init();
	DH11_Init();
	LCD_Init();	
	W25Q64_Init();
	BT301M_Init();
													
	Key_Init(&Key_Switch);
	Key_Init(&Key_Up);
	Key_Init(&Key_Bown);
	Key_Init(&Key_Left);
	Key_Init(&Key_Right);
	Key_Init(&Key_OK);
	
	
	Time_structure.tm_year = 2026;
	Time_structure.tm_mon = 12;
	Time_structure.tm_mday = 31;
	Time_structure.tm_hour = 23;
	Time_structure.tm_min = 59;
	Time_structure.tm_sec = 55;
	
	W25Q64_GetCharNum();
	MyRTC_SetTime(&Time_structure);
	LCD_ALLClear(COLOR_BLACK);
	
	while(1)
	{
		switch (MOD_Status)
    {
    	case 0:
				Time_structure = MyRTC_ReadTime();
				
				ShowTime(56, 20, &Time_structure, COLOR_WHITE, COLOR_BLACK);
				if(DHT11_ReadTime != SYSTime_Getms())
				{
					DH11_ReadData();
					DHT11_ReadTime = SYSTime_Getms();
				}
				ShowTempAndHum(0, 0, COLOR_WHITE, COLOR_BLACK);
    		break;
    	case 1:
    		break;
    	default:break;
    }
	}
	
}






/*显示桌面的时间
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：含有时间信息的结构体
	参数4：像素颜色
*/
void ShowTime(uint16_t x, uint16_t y, TimeDateTypedef *Time, uint16_t color, uint16_t BackgroundColor)
{	
	uint8_t seat_mon;
	uint8_t seat_mday;
	static uint8_t Record_mon = 0;
	static uint8_t Record_mday = 0;
	
	LCD_ShowTimeNum(x + 16, y, Time->tm_hour/10, color ,BackgroundColor);
	LCD_ShowTimeNum(x + 16 + (1 * 32), y, Time->tm_hour%10, color ,BackgroundColor);
	LCD_ShowTimeNum(x + 16 + (2 * 32), y, 10, color ,BackgroundColor);
	LCD_ShowTimeNum(x + 16 + (3 * 32), y, Time->tm_min/10, color ,BackgroundColor);
	LCD_ShowTimeNum(x + 16 + (4 * 32), y, Time->tm_min%10, color ,BackgroundColor);
	LCD_ShowUnsigNum(x + 16 + (5 * 32), y + 32, Time->tm_sec / 10, color ,BackgroundColor);
	LCD_ShowUnsigNum(x + 16 + (5 * 32 + 16), y + 32, Time->tm_sec % 10, color ,BackgroundColor);
	
	LCD_ShowUnsigNum(x, y + 64, Time->tm_year, color ,BackgroundColor);
	LCD_ShowChar(x + (4 * 16), y + 64, "年", color ,BackgroundColor);
	seat_mon = (Time->tm_mon > 9) ? 16 : 0;
	LCD_ShowUnsigNum(x + (6 * 16), y + 64, Time->tm_mon, color ,BackgroundColor);
	LCD_ShowChar(x + (7 * 16) + seat_mon, y + 64, "月", color ,BackgroundColor);
	seat_mday = (Time->tm_mday > 9) ? 16 : 0;
	
	if((Record_mon != Time->tm_mon) || (Record_mday != Time->tm_mday))
	{
		LCD_WindowClear(x + 190, y + 64, x + 224, y + 96, 0x0000);
		Record_mon = Time->tm_mon;
		Record_mday = Time->tm_mday;
	}
	
	LCD_ShowUnsigNum(x + (9* 16) + seat_mon, y + 64, Time->tm_mday, color ,BackgroundColor);
	LCD_ShowChar(x + (10 * 16) + seat_mon + seat_mday, y + 64, "日", color ,BackgroundColor);
}

/*显示温湿度
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数4：像素颜色
*/
void ShowTempAndHum(uint16_t x, uint16_t y, uint16_t color, uint16_t BackgroundColor)
{	
	float Temperature = 0;
	float Humidity = 0;
	
	Temperature = DH11_GetTempData();
	Humidity = DH11_GetHumData();
	
	if(Temperature < 10)
	{
		LCD_WindowClear(x + 16, y + 32, x + 32, y + 64, BackgroundColor);
	}
	if(Humidity < 10)
	{
		LCD_WindowClear(x + 16, y, x + 32, y + 32, BackgroundColor);
	}
	
	
	if(Temperature >= 0)
	{
		LCD_ShowUnsigNum(x, y + 32, (uint8_t)Temperature, color, BackgroundColor);
		LCD_ShowPattern(x + 32, y + 32, x + 32 + 16, y + 64, 1, COLOR_ORANGRED, BackgroundColor);
	}else
	{
		Temperature *= -1;
		LCD_ShowChar(x, y + 32, "-", color, BackgroundColor);
		LCD_ShowUnsigNum(x, y + 32, (uint8_t)Temperature, color, BackgroundColor);
		LCD_ShowPattern(x + 32, y + 32, x + 32 + 16, y + 64, 1, COLOR_ORANGRED, BackgroundColor);
	}
	LCD_ShowUnsigNum(x, y, (uint8_t)Humidity, color, BackgroundColor);
	LCD_ShowPattern(x + 32, y, x + 32 + 16, y + 32, 0, COLOR_CYAN, BackgroundColor);
}
