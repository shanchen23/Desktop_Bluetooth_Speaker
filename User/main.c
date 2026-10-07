#include "stm32f10x.h"                  // Device header
#include "stm32f10x_pwr.h"   // 电源控制库（工程里已经编译过 stm32f10x_pwr.o，直接用）
#include <stdio.h>
#include "Time.h"
#include "LCD.h"
#include "LCD_Data.h"
#include "SYSTime.h"
#include "Key.h"
#include "DH11.h"
#include "W25Q64.h"
#include "BT301M.h"

void DesktopMode(void);
void SetMode(void);
void SetMode_Bluetooth(void);
void ShowTime(uint16_t x, uint16_t y, TimeDateTypedef *Time, uint16_t color, uint16_t BackgroundColor);
void ShowTempAndHum(uint16_t x, uint16_t y, uint16_t color, uint16_t BackgroundColor);
void Showplayer(uint16_t x, uint16_t y, uint16_t color, uint16_t BackgroundColor);
//void Power_Init(void);
//void EnterPowerOff(void);
	
#define Desktop_Mode   		((uint8_t )0x01)
#define Set_Mode       		((uint8_t )0x02)

#define BluetoothSet_Status   ((uint8_t )0x01)
#define gradienter_Status     ((uint8_t )0x02)
#define TimeSet_Status        ((uint8_t )0x03)
#define DisplaySet_Status     ((uint8_t )0x04)

typedef struct 
{
	char *Name;
	uint16_t SerialNum;
	
}Menu_Tab;

TimeDateTypedef Time_structure = {1999, 12, 31, 23, 59, 55};
uint8_t Mode = 0;
//uint8_t Status = 0;
uint8_t Key_Status = 0;
uint8_t Key_Singlelong_Flag = 0; // 0:还没长按 1:长按过了
uint8_t Key_SwitchReturn_Flag = 0;//防止多个地方取用Key_Switch_Return得不到状态
uint32_t DHT11_ReadTimeFlag = 0;

int main()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		
	
	Time_Init();
	SYSTime_Init();
	MyRTC_Init();
	DH11_Init();
	W25Q64_Init();
	BT301M_Init();
	Key_ALLInit();
	LCD_Init();
//	Power_Init(); 
	
	MyRTC_SetTime(&Time_structure);
	LCD_ALLClear(COLOR_EGGSHELL);
	
	Mode = Desktop_Mode;
	
	LCD_ShowPattern(288, 0, 320, 16, LCD_Pattern16x32[2], COLOR_BLACK, COLOR_EGGSHELL);
	while(1)
	{
		BT301M_SendATCmd();
		DistributeRXData();
		
		Key_SwitchReturn_Flag = Key_GetStatus(&Key_Switch_Return);
		if(Key_SwitchReturn_Flag == 4)   // 长按电源键松手 → 关机
		{
			Key_SwitchReturn_Flag = 0;
//			EnterPowerOff();
		}
		
		//Key_ALLLongHoldPoll();
		switch (Mode)
    {
    	case Desktop_Mode:
				DesktopMode();
    		break;
			
    	case Set_Mode:
				SetMode();
    		break;
			
    	default:break;
    }
		
		
		
		//调试信息
		//LCD_ShowString(100, 150, ErrorInformation, COLOR_RED ,COLOR_EGGSHELL);
	}
}

//void Power_Init(void)
//{
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); // 只开时钟
//}

//// 彻底关机：黑屏 + 进待机
//void EnterPowerOff(void)
//{
//	LCD_ALLClear(COLOR_BLACK);
//	SYSTime_Delayms(200);
//	__disable_irq();
//	PWR_WakeUpPinCmd(ENABLE);   // 关机前一刻才把 PA0 设为唤醒脚
//	PWR_EnterSTANDBYMode();
//}


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
	static uint8_t PlayProgress = 0; // 0 - 199
	static char Name[256] = {0};
	static char Libretto[256] = {0};
	
	
	static uint16_t NameRoll = 0;
	static uint16_t LibrettoRoll = 0;
	
	
	
	
	LCD_ShowPattern(x, y, x + 64, y + 64, LCD_Pattern64x64[Pattern_Num], COLOR_BLACK, COLOR_EGGSHELL);
	if(BT301M_BTINFO_Data.BTINFO_Status == 1)
	{
		Pattern_Num = (Pattern_Num < 3) ? (Pattern_Num + 1): 0;
		
		//显示总时长
		if((BT301M_BTINFO_Data.BTINFO_ALLTime / 1000 / 60) <= 60)
		{
			if((BT301M_BTINFO_Data.BTINFO_ALLTime / 1000 / 60) >= 10 )
			{
				LCD_ShowUnsigNum(x + 64, y + 40, BT301M_BTINFO_Data.BTINFO_ALLTime / 1000 / 60, COLOR_BLACK, COLOR_EGGSHELL);
			}else
			{
				LCD_WindowClear(x + 64, y + 40, x + 80, y + 72, COLOR_EGGSHELL);
				LCD_ShowUnsigNum(x + 80, y + 40, BT301M_BTINFO_Data.BTINFO_ALLTime / 1000 / 60, COLOR_BLACK, COLOR_EGGSHELL);
			}
			
			LCD_ShowChar(x + 98, y + 40, ":", COLOR_BLACK, COLOR_EGGSHELL);
			
			if((BT301M_BTINFO_Data.BTINFO_ALLTime / 1000 % 60) >= 10 )
			{
				LCD_ShowUnsigNum(x + 114, y + 40, BT301M_BTINFO_Data.BTINFO_ALLTime / 1000 % 60, COLOR_BLACK, COLOR_EGGSHELL);
			}else
			{
				LCD_WindowClear(x + 130, y + 40, x + 146, y + 72, COLOR_EGGSHELL);
				LCD_ShowUnsigNum(x + 114, y + 40, BT301M_BTINFO_Data.BTINFO_ALLTime / 1000 % 60, COLOR_BLACK, COLOR_EGGSHELL);
			}
		}else
		{
			LCD_WindowClear(x + 64 + 16 * 3, y + 40, x + 64 + 16 * 5, y + 72, COLOR_EGGSHELL);
			LCD_ShowString(x + 64, y + 40, ">1h", COLOR_BLACK, COLOR_EGGSHELL);
		}
		
		
		//显示当前时长
		if((BT301M_BTINFO_Data.BTINFO_PlayTime / 1000 / 60) <= 60)
		{
			if((BT301M_BTINFO_Data.BTINFO_PlayTime / 1000 / 60) >= 10 )
			{
				LCD_ShowUnsigNum(x + 64 + 144, y + 40, BT301M_BTINFO_Data.BTINFO_PlayTime / 1000 / 60, COLOR_BLACK, COLOR_EGGSHELL);
			}else
			{
				LCD_WindowClear(x + 64 + 144, y + 40, x + 80 + 144, y + 72, COLOR_EGGSHELL);
				LCD_ShowUnsigNum(x + 80 + 144, y + 40, BT301M_BTINFO_Data.BTINFO_PlayTime / 1000 / 60, COLOR_BLACK, COLOR_EGGSHELL);
			}
			
			LCD_ShowChar(x + 98 + 144, y + 40, ":", COLOR_BLACK, COLOR_EGGSHELL);
			
			if((BT301M_BTINFO_Data.BTINFO_PlayTime / 1000 % 60) >= 10 )
			{
				LCD_ShowUnsigNum(x + 114 + 144, y + 40, BT301M_BTINFO_Data.BTINFO_PlayTime / 1000 % 60, COLOR_BLACK, COLOR_EGGSHELL);
			}else
			{
				LCD_WindowClear(x + 130 + 144, y + 40, x + 146 + 144, y + 72, COLOR_EGGSHELL);
				LCD_ShowUnsigNum(x + 114 + 144, y + 40, BT301M_BTINFO_Data.BTINFO_PlayTime / 1000 % 60, COLOR_BLACK, COLOR_EGGSHELL);
			}
		}else
		{
			LCD_WindowClear(x + 64 + 16 * 3 + 144, y + 40, x + 64 + 16 * 5 + 144, y + 72, COLOR_EGGSHELL);
			LCD_ShowString(x + 64 + 144, y + 40, ">1h", COLOR_BLACK, COLOR_EGGSHELL);
		}
		
		//显示进度
		if(PlayProgress > (uint8_t )(BT301M_BTINFO_Data.BTINFO_PlayTime / (BT301M_BTINFO_Data.BTINFO_ALLTime / 200)) )
		{
			LCD_ShowPattern(x + 80, y + 32 , x + 80 + 208, y + 32 + 8, LCD_Pattern8x208[0], COLOR_BLACK, COLOR_EGGSHELL);
			LCD_WindowClear(x + 84, y + 34, x + 84 + 200, y + 38, COLOR_EGGSHELL);
		}
		PlayProgress = (uint8_t )(BT301M_BTINFO_Data.BTINFO_PlayTime / (BT301M_BTINFO_Data.BTINFO_ALLTime / 200));
		LCD_WindowClear(x + 84, y + 34, x + 84 + PlayProgress + 1, y + 38, COLOR_RED);
	}
	
	//显示歌词歌名
	if(strcoll(BT301M_LYRIC_Data.LYRIC_Libretto, Libretto) != 0)
	{
			strcpy(Libretto, BT301M_LYRIC_Data.LYRIC_Libretto);
			LCD_WindowClear(0, y - 32, 320, y, COLOR_EGGSHELL);
			LibrettoRoll = 0;
	}

	if(LCD_GetString_X_Offset(Libretto) > 320)
	{
			// 滚动显示
			LCD_ShowStringRoll(0, y - 32, 320, Libretto, &LibrettoRoll, COLOR_DEEP_SKY_BLUE, COLOR_EGGSHELL);
	}else
	{
			LCD_ShowString(0, y - 32, Libretto, COLOR_DEEP_SKY_BLUE, COLOR_EGGSHELL);
	}

	if(strcoll(BT301M_LYRIC_Data.LYRIC_Name, Name) != 0)
	{
			strcpy(Name, BT301M_LYRIC_Data.LYRIC_Name);
			LCD_WindowClear(x + 80, y, 320, y + 32, COLOR_EGGSHELL);
			NameRoll = 0;
	}

	if(LCD_GetString_X_Offset(Name) > 320 - (x + 80))
	{
			// 滚动显示
			LCD_ShowStringRoll(x + 80, y, 320 - (x + 80), Name, &NameRoll, COLOR_BLACK, COLOR_EGGSHELL);
	}else
	{
			LCD_ShowString(x + 80, y, Name, COLOR_BLACK, COLOR_EGGSHELL);
	}
}

//更新桌面
void Update_DesktopMode(void)
{
	LCD_ALLClear(COLOR_EGGSHELL);
	LCD_ShowPattern(288, 0, 320, 16, LCD_Pattern16x32[2], COLOR_BLACK, COLOR_EGGSHELL);
	if(BT301M_MODE_Data == 0)
	{
		LCD_ShowPattern(90, 192, 90 + 208, 192 + 8, LCD_Pattern8x208[0], COLOR_BLACK, COLOR_EGGSHELL);
	}
}


//桌面模式
void DesktopMode(void)
{
	static uint32_t SPKVOLT_Time;
	static uint32_t MODE_State = 0;
	
	uint16_t SPKVOLT_IntervalTime = 250;
	
	Time_structure = MyRTC_ReadTime();
	
	//更新不同模式屏幕
	if(BT301M_MODE_Data != MODE_State)
	{
		MODE_State = BT301M_MODE_Data;
		Update_DesktopMode();
	}
	
	//显示时间
	if(BT301M_MODE_Data == 0)
	{
		ShowTime(56, 16, &Time_structure, COLOR_BLACK, COLOR_EGGSHELL);
	}else
	{
		ShowTime(56, 64, &Time_structure, COLOR_BLACK, COLOR_EGGSHELL);
	}
	
	//显示温湿度
//	if(MyRTC_GetCounter() - DHT11_ReadTimeFlag >= 5)
//	{
			DH11_ReadData();
			ShowTempAndHum(0, 0, COLOR_BLACK, COLOR_EGGSHELL);
			DHT11_ReadTimeFlag = MyRTC_GetCounter();
//	}

	//显示播放动画
	if(BT301M_MODE_Data == 0)
	{
		Showplayer(10, 160, COLOR_BLACK, COLOR_EGGSHELL);
	}
	
	//按键判断
	Key_Status = Key_GetStatus(&Key_OK);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
				//蓝牙模式
				if(BT301M_MODE_Data == 0)
				{
					//LCD画面同步
					//BT301M暂停和开始播放
					BT301M_TXEnQueue("AT+PLAYPAUSE");
				}
    		break;
    	case 2:
				Key_Singlelong_Flag = 0;
    		break;
			case 4:
				if(Key_Singlelong_Flag == 0)
				{
					LCD_ALLClear(COLOR_BLACK);
					Mode = Set_Mode;
					Key_Singlelong_Flag = 1;
				}
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
				//蓝牙模式
				if(BT301M_MODE_Data == 0)
				{
					//LCD画面同步
					//增加音量1
					BT301M_TXEnQueue("AT+SPKVOL=+");
				}
    		break;
			case 4:
				//蓝牙模式
				if(BT301M_MODE_Data == 0)
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
				//蓝牙模式
				if(BT301M_MODE_Data == 0)
				{
					//LCD画面同步
					//减音量1
					BT301M_TXEnQueue("AT+SPKVOL=-");
				}
    		break;
			case 4:
				//蓝牙模式
				if(BT301M_MODE_Data == 0)
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
				//蓝牙模式
				if(BT301M_MODE_Data == 0)
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
				//蓝牙模式
				if(BT301M_MODE_Data == 0)
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




//设置模式
void SetMode(void)
{
	static int8_t Checked = 0;
	static uint16_t Checked_SerialNum = 0X0000;
	static uint8_t Key_OKFeedback_Flag = 0;
	static uint8_t Key_LeftFeedback_Flag = 0;
	static uint8_t Key_RightFeedback_Flag = 0;
	static uint8_t LANGID_Status = 0;	
	static TimeDateTypedef SetTime = {0};
	static TimeDateTypedef UPSetTime_Flag = {0};
	
	uint8_t Checked_Scope;
	
	const uint8_t MainMenu_Num = 4;
	const uint8_t BluetoothMenu_Num = 4;
	const uint8_t TimeMenu_Num = 7;
	
	Menu_Tab Menu[] = {{"蓝牙设置->", 0X0000},
										{"时间设置->", 0X0001},
										{"显示设置->", 0X0002},
										{"水平仪模式->", 0X0003},
	
										{"蓝牙连接", 0XF000},
										{"连接提示音", 0XF001},
										{"提示音语言", 0XF002},
										{"连接自动播放音乐", 0XF003},
										
										{"年", 0XF100},
										{"月", 0XF101},
										{"日", 0XF102},
										{"时", 0XF103},
										{"分", 0XF104},
										{"秒", 0XF105},
										{"应用", 0XF106},
	
										{"亮度", 0XF200},
										{"显示温湿度", 0XF201}};
	
																		
	Key_Status = Key_GetStatus(&Key_Up);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
				Checked--;
    		break;
			case 4:
				Checked--;
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
				Checked++;
    		break;
			case 4:
				Checked++;
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
				if((Checked_SerialNum & 0XF000) == 0)
				{
					Mode = Desktop_Mode;
					Update_DesktopMode();
					Checked_SerialNum = 0;
					return;
				}else
				{
					switch(Checked_SerialNum & 0xFF00)
					{
						case 0xF100:
							switch(Checked_SerialNum)
							{
								case 0XF100:
									SetTime.tm_year--;
									break;
								case 0XF101:
									SetTime.tm_mon--;
									break;
								case 0XF102:
									SetTime.tm_mday--;
									break;
								case 0XF103:
									SetTime.tm_hour--;
									break;
								case 0XF104:
									SetTime.tm_min--;
									break;
								case 0XF105:
									SetTime.tm_sec--;
									break;
								case 0XF106:
									Checked_SerialNum = 0x0000;
									LCD_ALLClear(COLOR_BLACK);
									break;
								default:break;
							}
							Key_LeftFeedback_Flag = 10;
							break;
						default:
							Checked_SerialNum = 0x0000;
							LCD_ALLClear(COLOR_BLACK);
							break;
					}
				}
    		break;
				case 4:
					switch(Checked_SerialNum & 0xFF00)
					{
						case 0xF100:
							switch(Checked_SerialNum)
							{
								case 0XF100:
									SetTime.tm_year--;
									break;
								case 0XF101:
									SetTime.tm_mon--;
									break;
								case 0XF102:
									SetTime.tm_mday--;
									break;
								case 0XF103:
									SetTime.tm_hour--;
									break;
								case 0XF104:
									SetTime.tm_min--;
									break;
								case 0XF105:
									SetTime.tm_sec--;
									break;
								case 0XF106:
									Checked_SerialNum = 0x0000;
									LCD_ALLClear(COLOR_BLACK);
									break;
								default:break;
							}
							Key_LeftFeedback_Flag = 10;
							break;
						default:
							Checked_SerialNum = 0x0000;
							LCD_ALLClear(COLOR_BLACK);
							break;
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
				if((Checked_SerialNum & 0XF000) == 0)
				{
					Checked_SerialNum |= 0xF000;
					Checked_SerialNum = ((Checked_SerialNum & 0x000F) << 8) | (Checked_SerialNum & 0xF0F0);
					Checked = 0;
					LCD_ALLClear(COLOR_BLACK);
				}else
				{
					switch(Checked_SerialNum & 0xFF00)
					{
						case 0xF000:
							BT301M_TXEnQueue("AT+MODE");//查询模式
							BT301M_TXEnQueue("AT+WARNING");//查询提示音
							BT301M_TXEnQueue("AT+AUTOPLAY");//查询连接自动播放
							DistributeRXData();
							break;
						case 0xF100:
							switch(Checked_SerialNum)
							{
								case 0XF100:
									SetTime.tm_year++;
									break;
								case 0XF101:
									SetTime.tm_mon++;
									break;
								case 0XF102:
									SetTime.tm_mday++;
									break;
								case 0XF103:
									SetTime.tm_hour++;
									break;
								case 0XF104:
									SetTime.tm_min++;
									break;
								case 0XF105:
									SetTime.tm_sec++;
									break;
								default:break;
							}
							Key_RightFeedback_Flag = 10;
							break;
						default:break;
					}
				}
    		break;
			case 4:
				switch(Checked_SerialNum & 0xFF00)
				{
					case 0xF000:
						BT301M_TXEnQueue("AT+MODE");//查询模式
						BT301M_TXEnQueue("AT+WARNING");//查询提示音
						BT301M_TXEnQueue("AT+AUTOPLAY");//查询连接自动播放
						DistributeRXData();
						break;
					case 0xF100:
						switch(Checked_SerialNum)
						{
							case 0XF100:
								SetTime.tm_year++;
								break;
							case 0XF101:
								SetTime.tm_mon++;
								break;
							case 0XF102:
								SetTime.tm_mday++;
								break;
							case 0XF103:
								SetTime.tm_hour++;
								break;
							case 0XF104:
								SetTime.tm_min++;
								break;
							case 0XF105:
								SetTime.tm_sec++;
								break;
							default:break;
						}
						Key_RightFeedback_Flag = 10;
						break;
					default:break;
				}
				break;
    	default:break;
    }
	}

	Key_Status = Key_GetStatus(&Key_OK);
	if(Key_Status != 0)
	{
		switch(Key_Status)
    {
    	case 1:
				if((Checked_SerialNum & 0xF000) == 0)
				{
					Checked_SerialNum |= 0xF000;
					Checked_SerialNum = ((Checked_SerialNum & 0x000F) << 8) | (Checked_SerialNum & 0xF0F0);
					Checked = 0;
					SetTime = MyRTC_ReadTime();
					LCD_ALLClear(COLOR_BLACK);
				}else
				{
					switch((Checked_SerialNum & 0x0F00) >> 8)
					{
						case 0: 
						switch(Checked_SerialNum)
						{
							case 0XF000:
								if(BT301M_MODE_Data == 0)
								{
									BT301M_TXEnQueue("AT+MODE=IDLE");
								}else
								{
									BT301M_TXEnQueue("AT+MODE=BT");
								}
								break;
							case 0XF001:
								if(BT301M_WARNING_Data == 0)
								{
									BT301M_TXEnQueue("AT+WARNING=1");
								}else
								{
									BT301M_TXEnQueue("AT+WARNING=0");
								}
								break;
							case 0XF002:
								if(BT301M_LANGID_Data == 0)
								{
									BT301M_TXEnQueue("AT+LANGID=1");
								}else
								{
									BT301M_TXEnQueue("AT+LANGID=0");
								}
								break;
							case 0XF003:
								if(BT301M_AUTOPLAY_Data == 0)
								{
									BT301M_TXEnQueue("AT+AUTOPLAY=1");
								}else
								{
									BT301M_TXEnQueue("AT+AUTOPLAY=0");
								}
								break;
							default:break;
							}
						case 1:
							if(Checked_SerialNum == 0XF106)
							{
								MyRTC_SetTime(&SetTime);
								Checked_SerialNum = 0x0000;
								LCD_ALLClear(COLOR_BLACK);
							}
							break;
						default:break;
					}
				}
				
				if((Checked_SerialNum & 0xFF00) == 0XF000)
				{
					BT301M_TXEnQueue("AT+MODE");//查询模式
					BT301M_TXEnQueue("AT+WARNING");//查询提示音
					BT301M_TXEnQueue("AT+LANGID");//查询提示音语言
					BT301M_TXEnQueue("AT+AUTOPLAY");//查询连接自动播放
					DistributeRXData();
				}
				Key_OKFeedback_Flag = 1;
    		break;
    	case 2:
				Key_Singlelong_Flag = 0;
    		break;
    	case 4:
				//返回桌面
				if(Key_Singlelong_Flag == 0)
				{
					Mode = Desktop_Mode;
					Update_DesktopMode();
					Checked_SerialNum = 0;
					Key_Singlelong_Flag = 1;
					return;
				}
    	default:break;
    }
	}
	
	if(Key_SwitchReturn_Flag != 0)
	{
		switch(Key_SwitchReturn_Flag)
    {
    	case 1:
				Key_SwitchReturn_Flag = 0;
				if((Checked_SerialNum & 0XF000) == 0)
				{
					Mode = Desktop_Mode;
					Update_DesktopMode();
					Checked_SerialNum = 0;
					return;
				}else
				{
					Checked_SerialNum = 0x0000;
					LCD_ALLClear(COLOR_BLACK);
				}
    		break;
    	default:break;
    }
	}
	
	if((Checked_SerialNum & 0XF000) == 0)
	{
		Checked_Scope = MainMenu_Num;
	}else
	{
		switch((Checked_SerialNum & 0X0F00) >> 8)
		{
			case 0:
				Checked_Scope = BluetoothMenu_Num;
				break;
			case 1:
				Checked_Scope = TimeMenu_Num;
				break;
			default:break;
		}
	}
	
	if(Checked < 0) Checked = Checked_Scope - 1;
	if(Checked > Checked_Scope - 1) Checked = 0;
	Checked_SerialNum = (Checked_SerialNum & 0XFFF0) | ( (uint16_t )(Checked & 0X0F) );
	
	
	for(uint8_t i = 0; i < MainMenu_Num + BluetoothMenu_Num + TimeMenu_Num; i++)
	{
		if( (Checked_SerialNum & 0XFFF0) == (Menu[i].SerialNum & 0XFFF0) )
		{
			if(Checked_SerialNum == Menu[i].SerialNum)
			{
				if(Key_OKFeedback_Flag == 1)
				{
					LCD_ShowString(0 , (Menu[i].SerialNum & 0x000F) * 32, Menu[i].Name, COLOR_BLACK, COLOR_EGGSHELL);
					Key_OKFeedback_Flag = 0;
				}else
				{
					LCD_ShowString(0 , (Menu[i].SerialNum & 0x000F) * 32, Menu[i].Name, COLOR_BLACK, COLOR_SKY_BLUE);
				}
			}else
			{
				LCD_ShowString(0 , (Menu[i].SerialNum & 0x000F) * 32, Menu[i].Name, COLOR_EGGSHELL, COLOR_BLACK);
			}
		}
	}
	
	if((Checked_SerialNum & 0xFF00) == 0XF000)
	{
		if(BT301M_MODE_Data == 0)
		{
			LCD_ShowPattern(320 - 32, 0, 320, 32, LCD_Pattern32x32[1], COLOR_GREEN, COLOR_BLACK);
		}else
		{
			LCD_ShowPattern(320 - 32, 0, 320, 32, LCD_Pattern32x32[0], COLOR_RED, COLOR_BLACK);
		}
		
		if(BT301M_WARNING_Data == 0)
		{
			LCD_ShowPattern(320 - 32, 32, 320, 64, LCD_Pattern32x32[0], COLOR_RED, COLOR_BLACK);
		}else
		{
			LCD_ShowPattern(320 - 32, 32, 320, 64, LCD_Pattern32x32[1], COLOR_GREEN, COLOR_BLACK);
		}
		
		if(LANGID_Status != BT301M_LANGID_Data)
		{
			LANGID_Status = BT301M_LANGID_Data;
			LCD_WindowClear(192, 64, 320, 96, COLOR_BLACK);
		}
		if(BT301M_LANGID_Data == 0)
		{
			LCD_ShowString(192, 64, "English", COLOR_EGGSHELL, COLOR_BLACK);
		}else
		{
			LCD_ShowString(224, 64, "中文", COLOR_EGGSHELL, COLOR_BLACK);
		}
		
		if(BT301M_AUTOPLAY_Data == 0)
		{
			LCD_ShowPattern(320 - 32, 96, 320, 128, LCD_Pattern32x32[0], COLOR_RED, COLOR_BLACK);
		}else
		{
			LCD_ShowPattern(320 - 32, 96, 320, 128, LCD_Pattern32x32[1], COLOR_GREEN, COLOR_BLACK);
		}
	}
	
	if((Checked_SerialNum & 0xFF00) == 0XF100)
	{
		for(uint8_t i = 0; i < TimeMenu_Num - 1; i++)
		{
			if(Key_LeftFeedback_Flag != 0)
			{
				Key_LeftFeedback_Flag--;
				LCD_ShowPattern(64, 8 + 32 * (Checked_SerialNum & 0x000F), 80, 24 + 32 * (Checked_SerialNum & 0x000F), LCD_Pattern16x16[0], COLOR_EGGSHELL, COLOR_BLACK);
			}else
			{
				LCD_ShowPattern(64, 8 + 32 * i, 80, 24 + 32 * i, LCD_Pattern16x16[0], COLOR_STEEL_BLUE, COLOR_BLACK);
			}
		}
		for(uint8_t i = 0; i < TimeMenu_Num - 1; i++)
		{
			if(Key_RightFeedback_Flag != 0)
			{
				Key_RightFeedback_Flag --;
				LCD_ShowPattern(144, 8 + 32 * (Checked_SerialNum & 0x000F), 160, 24 + 32 * (Checked_SerialNum & 0x000F), LCD_Pattern16x16[1], COLOR_EGGSHELL, COLOR_BLACK);
			}else
			{
				LCD_ShowPattern(144, 8 + 32 * i, 160, 24 + 32 * i, LCD_Pattern16x16[1], COLOR_STEEL_BLUE, COLOR_BLACK);
			}
		}
		
		//年
		if(UPSetTime_Flag.tm_year != SetTime.tm_year)
		{
			if(SetTime.tm_year > 60000) SetTime.tm_year = 9999;
			if(SetTime.tm_year > 9999) SetTime.tm_year = 0;
			UPSetTime_Flag.tm_year = SetTime.tm_year;
			LCD_WindowClear(80, 0, 80 + 64, 32, COLOR_BLACK);
		}
		LCD_ShowUnsigNum(80, 0, SetTime.tm_year, COLOR_EGGSHELL,COLOR_BLACK);
		
		//月
		if(UPSetTime_Flag.tm_mon != SetTime.tm_mon)
		{
			if(SetTime.tm_mon > 250 || SetTime.tm_mon == 0) SetTime.tm_mon = 12;
			if(SetTime.tm_mon > 12) SetTime.tm_mon = 1;
			UPSetTime_Flag.tm_mon = SetTime.tm_mon;
			LCD_WindowClear(80, 32, 80 + 64, 32 + 32, COLOR_BLACK);
		}
		if(SetTime.tm_mon < 10)
		{
			LCD_ShowUnsigNum(104, 32, SetTime.tm_mon, COLOR_EGGSHELL,COLOR_BLACK);
		}else
		{
			LCD_ShowUnsigNum(96, 32, SetTime.tm_mon, COLOR_EGGSHELL,COLOR_BLACK);
		}
		
		//日
		if(UPSetTime_Flag.tm_mon == 2)
		{
			if((SetTime.tm_year % 4 == 0 && SetTime.tm_year % 100 != 0) || SetTime.tm_year % 400 == 0)
			{
				if(SetTime.tm_mday > 250 || SetTime.tm_mday == 0) SetTime.tm_mday = 29;
				if(SetTime.tm_mday > 29) SetTime.tm_mday = 1;
			}else
			{
				if(SetTime.tm_mday > 250 || SetTime.tm_mday == 0) SetTime.tm_mday = 28;
				if(SetTime.tm_mday > 28) SetTime.tm_mday = 1;
			}
		}else
		{
			if(UPSetTime_Flag.tm_mon == 1
				|| UPSetTime_Flag.tm_mon == 3
				|| UPSetTime_Flag.tm_mon == 5
				|| UPSetTime_Flag.tm_mon == 7
				|| UPSetTime_Flag.tm_mon == 8
				|| UPSetTime_Flag.tm_mon == 10
				|| UPSetTime_Flag.tm_mon == 12)
			{
				if(SetTime.tm_mday > 250 || SetTime.tm_mday == 0) SetTime.tm_mday = 31;
				if(SetTime.tm_mday > 31) SetTime.tm_mday = 1;
			}else
			{
				if(SetTime.tm_mday > 250 || SetTime.tm_mday == 0) SetTime.tm_mday = 30;
				if(SetTime.tm_mday > 30) SetTime.tm_mday = 1;
			}
		}
		if(UPSetTime_Flag.tm_mday != SetTime.tm_mday)
		{
			UPSetTime_Flag.tm_mday = SetTime.tm_mday;
			LCD_WindowClear(80, 32 * 2, 80 + 64, 32 * 2 + 32, COLOR_BLACK);
		}
		if(SetTime.tm_mday < 10)
		{
			LCD_ShowUnsigNum(104, 32 * 2, SetTime.tm_mday, COLOR_EGGSHELL,COLOR_BLACK);
		}else
		{
			LCD_ShowUnsigNum(96, 32 * 2, SetTime.tm_mday, COLOR_EGGSHELL,COLOR_BLACK);
		}
		
		//时
		if(UPSetTime_Flag.tm_hour != SetTime.tm_hour)
		{
			if(SetTime.tm_hour > 250) SetTime.tm_hour = 23;
			if(SetTime.tm_hour > 23) SetTime.tm_hour = 0;
			UPSetTime_Flag.tm_hour = SetTime.tm_hour;
			LCD_WindowClear(80, 32 * 3, 80 + 64, 32 * 3 + 32, COLOR_BLACK);
		}
		if(SetTime.tm_hour < 10)
		{
			LCD_ShowUnsigNum(104, 32 * 3, SetTime.tm_hour, COLOR_EGGSHELL,COLOR_BLACK);
		}else
		{
			LCD_ShowUnsigNum(96, 32 * 3, SetTime.tm_hour, COLOR_EGGSHELL,COLOR_BLACK);
		}
		
		//分
		if(UPSetTime_Flag.tm_min != SetTime.tm_min)
		{
			if(SetTime.tm_min > 250) SetTime.tm_min = 59;
			if(SetTime.tm_min > 59) SetTime.tm_min = 0;
			UPSetTime_Flag.tm_min = SetTime.tm_min;
			LCD_WindowClear(80, 32 * 4, 80 + 64, 32 * 4 + 32, COLOR_BLACK);
		}
		if(SetTime.tm_min < 10)
		{
			LCD_ShowUnsigNum(104, 32 * 4, SetTime.tm_min, COLOR_EGGSHELL,COLOR_BLACK);
		}else	
		{
			LCD_ShowUnsigNum(96, 32 * 4, SetTime.tm_min, COLOR_EGGSHELL,COLOR_BLACK);
		}
		
		//秒
		if(UPSetTime_Flag.tm_sec != SetTime.tm_sec)
		{
			if(SetTime.tm_sec > 250) SetTime.tm_sec = 59;
			if(SetTime.tm_sec > 59) SetTime.tm_sec = 0;
			UPSetTime_Flag.tm_sec = SetTime.tm_sec;
			LCD_WindowClear(80, 32 * 5, 80 + 64, 32 * 5 + 32, COLOR_BLACK);
		}
		if(SetTime.tm_sec < 10)
		{
			LCD_ShowUnsigNum(104, 32 * 5, SetTime.tm_sec, COLOR_EGGSHELL,COLOR_BLACK);
		}else
		{
			LCD_ShowUnsigNum(96, 32 * 5, SetTime.tm_sec, COLOR_EGGSHELL,COLOR_BLACK);
		}
	}
}

