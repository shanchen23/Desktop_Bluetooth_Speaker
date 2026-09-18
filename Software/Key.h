#ifndef __KEY_H
#define __KEY_H

/*选择按键参数
  参数1：按键外设时钟初始化
	参数2：按键引脚的GPIOx
	参数3：按键引脚的Pin
	参数4：判断是否是长按的时间ms
	参数5：按键当前状态
	参数6：按键的结果	0：没有按下	1：短按	2：长按	3：长按不松手
	参数7：记录按下的时间
*/
typedef struct 
{
	uint32_t RCC_APB2Periph;
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_Pin;
	uint16_t LongTime;
	uint8_t KeyStatus;
	uint8_t GetKey;
	uint32_t RecordTim;
	
}Key_InitTypedef;

void Key_Init(Key_InitTypedef *Key);
void Key_StatusPoll(Key_InitTypedef *Key);
uint8_t Key_Get(Key_InitTypedef *Key);

#endif
