#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"                  // Device header

/*选择按键参数
  参数1：按键外设时钟初始化
	参数2：按键引脚的GPIOx
	参数3：按键引脚的Pin
	参数4：判断是否是长按的时间ms
	参数5：按键的结果	0：没有按下  1：短按松手  2：长按松手  3：短按不松手  4：长按不松手
	参数6：记录开始时间
	参数7：记录最后时间
*/
typedef struct 
{
	uint32_t RCC_APB2Periph;
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_Pin;
	uint16_t LongTime;
	volatile uint8_t GetKey;
	uint32_t Start_time;
	uint32_t LastTime;
	
}Key_InitTypedef;

extern Key_InitTypedef Key_Switch_Return;
extern Key_InitTypedef Key_Up;
extern Key_InitTypedef Key_Bown;
extern Key_InitTypedef Key_Left;
extern Key_InitTypedef Key_Right;
extern Key_InitTypedef Key_OK;
extern uint32_t Key_UpdateTim;

void Key_Init(Key_InitTypedef *Key);
void Key_ALLInit(void);
void Key_IRQHandlerStatus(Key_InitTypedef *Key, uint32_t EXTI_Line);
void Key_LongPoll(Key_InitTypedef *Key);
void Key_ALLLongHoldPoll(void);
uint8_t Key_GetStatus(Key_InitTypedef *Key);
void PowerOff(void);

#endif
