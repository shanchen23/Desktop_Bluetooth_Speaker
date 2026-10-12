#include "stm32f10x.h"                  // Device header
#include "MyRTC.h"
#include <Time.h>

TimeDateTypedef Time_structure;


//初始化RTC实时时钟
void MyRTC_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	
	if(BKP_ReadBackupRegister(BKP_DR1) != 0x5A5A)
	{
		RCC_LSEConfig(RCC_LSE_ON);
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET);
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
		
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();
		
		RTC_SetPrescaler(32768 - 1);
		RTC_WaitForLastTask();
		
		RTC_SetCounter(0);
		RTC_WaitForLastTask();
		
		BKP_WriteBackupRegister(BKP_DR1,0x5A5A);
	}else
	{
		Time_structure = MyRTC_ReadTime();
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();
	}
}

/*
	设置实时时钟的时间
	参数：时间参数的结构体
*/
void MyRTC_SetTime(TimeDateTypedef *SetTime)
{
	time_t Time_cnt;
	struct tm time_date = {0};
	
	time_date.tm_year = SetTime->tm_year - 1900;
	time_date.tm_mon =  SetTime->tm_mon - 1;
	time_date.tm_mday = SetTime->tm_mday;
	time_date.tm_hour = SetTime->tm_hour;
	time_date.tm_min =  SetTime->tm_min;
	time_date.tm_sec =  SetTime->tm_sec;
	
	Time_cnt = mktime(&time_date) - 8 * 60 * 60;
	RTC_SetCounter(Time_cnt);
}

/*
	读取RTC实时时钟的当前时间
	返回：时间参数的结构体
*/
TimeDateTypedef MyRTC_ReadTime(void)
{
	time_t time_cnt;
	struct tm time_date;
	TimeDateTypedef ReadTime;
	
	time_cnt = RTC_GetCounter() + 8 * 60 * 60;
	
	time_date = *localtime(&time_cnt);
	
	ReadTime.tm_year = time_date.tm_year + 1900;
	ReadTime.tm_mon = time_date.tm_mon + 1;
	ReadTime.tm_mday = time_date.tm_mday;
	ReadTime.tm_hour = time_date.tm_hour;
	ReadTime.tm_min = time_date.tm_min;
	ReadTime.tm_sec = time_date.tm_sec;
	
	return ReadTime;
}

/*
	读取RTC实时时钟的当前时间的时间戳
	返回：时间戳
*/
uint32_t MyRTC_GetCounter(void)
{
	return (uint32_t )(RTC_GetCounter() + 8 * 60 * 60);
}
