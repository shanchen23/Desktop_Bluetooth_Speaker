#include "stm32f10x.h"                  // Device header
#include "stm32f10x_it.h"

//初始化系统时钟为1ms一次中断
void SYSTime_Init(void)
{
	//1ms中断一次
	SysTick_Config(72000);
	
}

//返回时间戳1ms的时间
uint32_t SYSTime_Getms(void)
{
	return SYS_Counter;
}

void SYSTime_Delayms(uint32_t ms)
{
	uint32_t SYSTime_FlagTim = SYSTime_Getms();
	while(SYSTime_Getms() - SYSTime_FlagTim < ms);
}
