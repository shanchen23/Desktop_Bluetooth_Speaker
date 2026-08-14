#include "stm32f10x.h"                  // Device header
#include "MyRTC.h"
#include <Time.h>

//初始化RTC实时时钟
void MyRTC_Init(void)
{
	//设置寄存器RCC_APB1ENR的PWREN和BKPEN位，使能电源和后备接口时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	//设置寄存器PWR_CR的DBP位，使能对后备寄存器和RTC的访问。
	PWR_BackupAccessCmd(ENABLE);
	
	if(BKP_ReadBackupRegister(BKP_DR1) == 0x5A5A)
	{
		//设置在备份域控制寄存器(RCC_BDCR)里的LSEBYP和LSEON位来选择是否旁路模式
		RCC_LSEConfig(RCC_LSE_ON);
		//备份域控制寄存器(RCC_BDCR)里的LSERDY指示LSE晶体振荡是否稳定
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET);
		//选择RTCCLK来源为LSE
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		//RTCCLK使能
		RCC_RTCCLKCmd(ENABLE);
		
		
		//等待RTC_CNT、RTC_ALR或RTC_PRL同步
		RTC_WaitForSynchro();
		//等待上一个操作完成
		RTC_WaitForLastTask();
		
		RTC_SetPrescaler(32768 - 1);
		RTC_WaitForLastTask();
		
		//这里是初始时间
		RTC_SetCounter(0);
		RTC_WaitForLastTask();
		
		BKP_WriteBackupRegister(BKP_DR1,0x5A5A);
	}else
	{
		//等待RTC_CNT、RTC_ALR或RTC_PRL同步
		RTC_WaitForSynchro();
		//RTC_CR寄存器中的RTOFF状态位是’1’时，才可以写入RTC寄存器。
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
	struct tm time_date;
	
	time_date.tm_year = SetTime->tm_year - 1900;
	time_date.tm_mon =  SetTime->tm_mon - 1;
	time_date.tm_mday = SetTime->tm_mday;
	time_date.tm_hour = SetTime->tm_hour;
	time_date.tm_min =  SetTime->tm_min;
	time_date.tm_sec =  SetTime->tm_sec;
	
	Time_cnt = mktime(&time_date);
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
	//读取RTC的CNT，获取当前的秒计数器   + 8 * 60 * 60为东八区的时区调整
	time_cnt = RTC_GetCounter() + 8 * 60 * 60;
	
	//使用localtime函数，将秒计数器转换为日期时间格式
	time_date = *localtime(&time_cnt);
	
	ReadTime.tm_year = time_date.tm_year + 1900;
	ReadTime.tm_mon = time_date.tm_mon + 1;
	ReadTime.tm_mday = time_date.tm_mday;
	ReadTime.tm_hour = time_date.tm_hour;
	ReadTime.tm_min = time_date.tm_min;
	ReadTime.tm_sec = time_date.tm_sec;
	
	return ReadTime;
}
