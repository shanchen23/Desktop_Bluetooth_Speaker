#include "stm32f10x.h"                  // Device header

void Time_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	TIM_InternalClockConfig(TIM2);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 65536 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	
	TIM_Cmd(TIM2,ENABLE);
} 

uint16_t Time_GetTimeus(void)
{
	return TIM_GetCounter(TIM2);
} 

void Time_Delayus(uint16_t us)
{
	uint16_t TIM_FlagTim = Time_GetTimeus();
	while(Time_GetTimeus() - TIM_FlagTim < us);
}
