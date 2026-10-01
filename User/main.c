#include "stm32f10x.h"                  // Device header
#include "Time.h"
#include "LCD.h"
#include "LCD_Data.h"
#include "SYSTime.h"
#include "Key.h"
#include "DH11.h"
#include "W25Q64.h"
#include "BT301M.h"

void DesktopMode(void);
void DSetMode(void);
void ShowTime(uint16_t x, uint16_t y, TimeDateTypedef *Time, uint16_t color, uint16_t BackgroundColor);
void ShowTempAndHum(uint16_t x, uint16_t y, uint16_t color, uint16_t BackgroundColor);
void Showplayer(uint16_t x, uint16_t y, uint16_t color, uint16_t BackgroundColor);

#define Desktop_Mode   ((uint8_t )0x01)
#define Set_Mode       ((uint8_t )0x02)

#define BluetoothSet_Status   ((uint8_t )0x01)
#define gradienter_Status     ((uint8_t )0x02)
#define TimeSet_Status        ((uint8_t )0x03)
#define DisplaySet_Status     ((uint8_t )0x04)

TimeDateTypedef Time_structure = {1999, 12, 31, 23, 59, 55};
uint8_t Mode = 0;
//uint8_t Status = 0;
uint8_t Key_Status = 0;
uint8_t Play_Flag = 1;

uint32_t DHT11_ReadTimeFlag = 0;

int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		
	
	Time_Init();
	SYSTime_Init();
	MyRTC_Init();
	DH11_Init();
	LCD_Init();
	W25Q64_Init();
	BT301M_Init();
	Key_ALLInit();
	
	MyRTC_SetTime(&Time_structure);
	LCD_ALLClear(COLOR_EGGSHELL);
	
	Mode = Desktop_Mode;
	
	while(1)
	{
		Key_ALLStatusPoll();
		
		BT301M_SendATCmd();
		DistributeRXData();
		
		switch (Mode)
    {
    	case Desktop_Mode:
				DesktopMode();
    		break;
			
    	case Set_Mode:
				DSetMode();
    		break;
    	default:break;
    }
		
		
		
		//调试信息
		LCD_ShowString(100, 150, ErrorInformation, COLOR_RED ,COLOR_EGGSHELL);
	}
}




/*显示桌面的时间
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：含有时间信息的结构体
	参数4：像素颜色
	参数5：背景像素颜色
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
		LCD_WindowClear(x + 190, y + 64, x + 224, y + 96, BackgroundColor);
		Record_mon = Time->tm_mon;
		Record_mday = Time->tm_mday;
	}
	
	LCD_ShowUnsigNum(x + (9* 16) + seat_mon, y + 64, Time->tm_mday, color ,BackgroundColor);
	LCD_ShowChar(x + (10 * 16) + seat_mon + seat_mday, y + 64, "日", color ,BackgroundColor);
}


/*显示温湿度
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：像素颜色
	参数4：背景像素颜色
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
		LCD_ShowPattern(x + 32, y + 32, x + 32 + 16, y + 64, LCD_Pattern16x32[1], COLOR_DARK_ORANGE, BackgroundColor);
	}else
	{
		Temperature *= -1;
		LCD_ShowChar(x, y + 32, "-", color, BackgroundColor);
		LCD_ShowUnsigNum(x, y + 32, (uint8_t)Temperature, color, BackgroundColor);
		LCD_ShowPattern(x + 32, y + 32, x + 32 + 16, y + 64, LCD_Pattern16x32[1], COLOR_DARK_ORANGE, BackgroundColor);
	}
	LCD_ShowUnsigNum(x, y, (uint8_t)Humidity, color, BackgroundColor);
	LCD_ShowPattern(x + 32, y, x + 32 + 16, y + 32, LCD_Pattern16x32[0], COLOR_CYAN, BackgroundColor);
}


/*显示桌面播放界面
	参数1：横坐标起始坐标
	参数2：纵坐标起始坐标
	参数3：像素颜色
	参数4：背景像素颜色
*/
void Showplayer(uint16_t x, uint16_t y, uint16_t color, uint16_t BackgroundColor)
{	
	static uint8_t Pattern_Num = 0;
	
	LCD_ShowPattern(x, y, x + 64, y + 64, LCD_Pattern64x64[Pattern_Num], COLOR_BLACK, COLOR_EGGSHELL);
	if(BT301M_BTINFO_Data.BTINFO_Status == 1)
	{
		Pattern_Num = (Pattern_Num < 3) ? (Pattern_Num + 1): 0;
	}
	
	LCD_ShowPattern(x + 80, y + 32 , x + 80 + 208, y + 32 + 8, LCD_Pattern8x208[0], COLOR_BLACK, COLOR_EGGSHELL);
	
	
	
	//LCD_ShowUnsigNum(x + 100, y, BT301M_BTINFO_Data.BTINFO_Status, color, BackgroundColor);
}


//桌面模式
void DesktopMode(void)
{
	static uint32_t SPKVOLT_Time;
	
	uint16_t SPKVOLT_IntervalTime = 250;
	
	Time_structure = MyRTC_ReadTime();
	ShowTime(56, 32, &Time_structure, COLOR_BLACK, COLOR_EGGSHELL);
	
	DH11_ReadData();
	if(DHT11_ReadTimeFlag != MyRTC_GetCounter())
	{
		ShowTempAndHum(0, 0, COLOR_BLACK, COLOR_EGGSHELL);
		DHT11_ReadTimeFlag = MyRTC_GetCounter();
	}

	if(Play_Flag == 1)
	{
		Showplayer(10, 144, COLOR_BLACK, COLOR_EGGSHELL);
	}
	
	Key_Status = Key_GetStatus(&Key_OK);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
				//播放标志为1
				if(Play_Flag == 1)
				{
					//LCD画面同步
					//BT301M暂停和开始播放
					BT301M_TXEnQueue("AT+PLAYPAUSE");
				}
    		break;
    	case 2:
				LCD_ALLClear(COLOR_BLACK);
				Mode = Set_Mode;
    		break;
			case 4:
				LCD_ALLClear(COLOR_BLACK);
				Mode = Set_Mode;
    		break;
    	default:break;
    }
	}
	
	Key_Status = Key_GetStatus(&Key_Up);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
				//播放标志为1
				if(Play_Flag == 1)
				{
					//LCD画面同步
					//增加音量1
					BT301M_TXEnQueue("AT+SPKVOL=+");
				}
    		break;
			case 4:
				//播放标志为1
				if(Play_Flag == 1)
				{
					//LCD画面同步
					//一直增加音量直加满
					if((SYSTime_Getms() - SPKVOLT_Time) >= SPKVOLT_IntervalTime)
					{
						BT301M_TXEnQueue("AT+SPKVOL=+");
						SPKVOLT_Time = SYSTime_Getms();
					}
				}
    		break;
    	default:break;
    }
	}
	
	Key_Status = Key_GetStatus(&Key_Bown);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
				//播放标志为1
				if(Play_Flag == 1)
				{
					//LCD画面同步
					//减音量1
					BT301M_TXEnQueue("AT+SPKVOL=-");
				}
    		break;
			case 4:
				//播放标志为1
				if(Play_Flag == 1)
				{
					//LCD画面同步
					//一直减音量直到为0
					if((SYSTime_Getms() - SPKVOLT_Time) >= SPKVOLT_IntervalTime)
					{
						BT301M_TXEnQueue("AT+SPKVOL=-");
						SPKVOLT_Time = SYSTime_Getms();
					}
				}
    		break;
    	default:break;
    }
	}
	
	Key_Status = Key_GetStatus(&Key_Left);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
				//播放标志为1
				if(Play_Flag == 1)
				{
					//LCD画面同步
					//上一首音乐
					BT301M_TXEnQueue("AT+BACKWARD");
				}
    		break;
    	default:break;
    }
	}
	
	Key_Status = Key_GetStatus(&Key_Right);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
				//播放标志为1
				if(Play_Flag == 1)
				{
					//LCD画面同步
					//下一首音乐
					BT301M_TXEnQueue("AT+FORWARD");
				}
    		break;
    	default:break;
    }
	}
}




//桌设置模式
void DSetMode(void)
{
	
	
	Key_Status = Key_GetStatus(&Key_OK);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
    		break;
    	default:break;
    }
	}
	
	Key_Status = Key_GetStatus(&Key_Up);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
    		break;
    	default:break;
    }
	}
	
	Key_Status = Key_GetStatus(&Key_Bown);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
    		break;
    	default:break;
    }
	}
	
	Key_Status = Key_GetStatus(&Key_Left);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
				LCD_ALLClear(COLOR_EGGSHELL);
				Mode = Desktop_Mode;
    		break;
    	default:break;
    }
	}
	
	Key_Status = Key_GetStatus(&Key_Right);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
    		break;
    	default:break;
    }
	}
}
