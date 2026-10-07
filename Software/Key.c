#include "stm32f10x.h"                  // Device header
#include "Key.h"
#include "SYSTime.h"
	
Key_InitTypedef Key_Switch_Return = {RCC_APB2Periph_GPIOA, GPIOA, GPIO_Pin_0, 3000, 0, 0, 0};
Key_InitTypedef Key_Up     = {RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_8, 1000, 0, 0, 0};
Key_InitTypedef Key_Bown   = {RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_9, 1000, 0, 0, 0};
Key_InitTypedef Key_Left   = {RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_12, 1000, 0, 0, 0};
Key_InitTypedef Key_Right  = {RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_13, 1000, 0, 0, 0};
Key_InitTypedef Key_OK     = {RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_14, 1000, 0, 0, 0};

/*初始化按键的GPIO，EXTI，NVIC
  参数：按键引脚参数的结构体
*/
void Key_Init(Key_InitTypedef *Key)
{
	RCC_APB2PeriphClockCmd(Key->RCC_APB2Periph,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;
	GPIO_InitStructure.GPIO_Pin = Key->GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(Key->GPIOx,&GPIO_InitStructure);
	
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	uint8_t Key_PortSource;
	uint8_t Key_PinSource = 0;
	uint8_t IRQChannel;

	for(Key_PinSource = 0; Key_PinSource < 16; Key_PinSource++)
	{
		if(Key->GPIO_Pin & (1 << Key_PinSource)) break;
	}

	if(Key->GPIOx == GPIOA) Key_PortSource = GPIO_PortSourceGPIOA;
	else if(Key->GPIOx == GPIOB) Key_PortSource = GPIO_PortSourceGPIOB;
	
	GPIO_EXTILineConfig(Key_PortSource, Key_PinSource);

	EXTI_InitStructure.EXTI_Line = (uint32_t)Key->GPIO_Pin;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	if(Key_PinSource == 0) IRQChannel = EXTI0_IRQn;
	else if(Key_PinSource == 1) IRQChannel = EXTI1_IRQn;
	else if(Key_PinSource == 2) IRQChannel = EXTI2_IRQn;
	else if(Key_PinSource == 3) IRQChannel = EXTI3_IRQn;
	else if(Key_PinSource == 4) IRQChannel = EXTI4_IRQn;
	else if(Key_PinSource >= 5 && Key_PinSource <= 9) IRQChannel = EXTI9_5_IRQn;
	else IRQChannel = EXTI15_10_IRQn;

	NVIC_InitStructure.NVIC_IRQChannel = IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	Key->GetKey = 0;
	Key->Start_time = 0;
}

//初始化
void Key_ALLInit(void)
{
	Key_Init(&Key_Switch_Return);
	Key_Init(&Key_Up);
	Key_Init(&Key_Bown);
	Key_Init(&Key_Left);
	Key_Init(&Key_Right);
	Key_Init(&Key_OK);
}

/*在中断中处理按键状态
  参数1：按键引脚参数的结构体
	参数2：当前中断的线路
*/
void Key_IRQHandlerStatus(Key_InitTypedef *Key, uint32_t EXTI_Line)
{
	EXTI_ClearITPendingBit(EXTI_Line);
	if(SYSTime_Getms() - Key->LastTime < 20) return;

	Key->LastTime = SYSTime_Getms();

	if(GPIO_ReadInputDataBit(Key->GPIOx ,Key->GPIO_Pin) == 0)
	{
		Key->Start_time = SYSTime_Getms();
	}else
	{
		Key->GetKey = ((SYSTime_Getms() - Key->Start_time) > Key->LongTime) ? 2 : 1;
	}
}


/*按键长按没松手状态检测
  参数：按键引脚参数的结构体
*/
void Key_LongHoldPoll(Key_InitTypedef *Key)
{
	if(GPIO_ReadInputDataBit(Key->GPIOx, Key->GPIO_Pin) == Bit_RESET && (SYSTime_Getms() - Key->Start_time) > Key->LongTime)
	{
		Key->GetKey = 4;
	}else if(GPIO_ReadInputDataBit(Key->GPIOx, Key->GPIO_Pin) == Bit_RESET && (SYSTime_Getms() - Key->Start_time) < Key->LongTime)
	{
		Key->GetKey = 3;
	}
}



//推进所有按键长按不松手状态检测
void Key_ALLLongHoldPoll(void)
{
	Key_LongHoldPoll(&Key_Switch_Return);
	Key_LongHoldPoll(&Key_Up);
	Key_LongHoldPoll(&Key_Bown);
	Key_LongHoldPoll(&Key_Left);
	Key_LongHoldPoll(&Key_Right);
	Key_LongHoldPoll(&Key_OK);
}

/*返回按键的结果
  参数：按键引脚参数的结构体
	返回：0：没有按下  1：短按松手  2：长按松手  3：短按不松手  4：长按不松手
*/
uint8_t Key_GetStatus(Key_InitTypedef *Key)
{
	uint8_t ret;
	
	ret = Key->GetKey;
	Key->GetKey = 0;
	
	return ret;
}

//中断记录按下的时间和松手的时间
void EXTI0_IRQHandler(void)
{
	Key_IRQHandlerStatus(&Key_Switch_Return, EXTI_Line0);
}

void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetFlagStatus(EXTI_Line8))
	{
		Key_IRQHandlerStatus(&Key_Up, EXTI_Line8);
	}
	
	if(EXTI_GetFlagStatus(EXTI_Line9))
	{
		Key_IRQHandlerStatus(&Key_Bown, EXTI_Line9);
	}
}

void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetFlagStatus(EXTI_Line12))
	{
		Key_IRQHandlerStatus(&Key_Left, EXTI_Line12);
	}
	
	if(EXTI_GetFlagStatus(EXTI_Line13))
	{
		Key_IRQHandlerStatus(&Key_Right, EXTI_Line13);
	}
	
	if(EXTI_GetFlagStatus(EXTI_Line14))
	{
		Key_IRQHandlerStatus(&Key_OK, EXTI_Line14);
	}
}
