#include "stm32f10x.h"                  // Device header
#include "Key.h"
#include "SYSTime.h"

/*初始化
  参数：按键引脚参数的结构体
*/
void Key_Init(Key_InitTypedef *Key)
{
	RCC_APB2PeriphClockCmd(Key->RCC_APB2Periph,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;
	GPIO_InitStructure.GPIO_Pin = Key->GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(Key->GPIOx,&GPIO_InitStructure);
	
	Key->KeyStatus = 0;
	Key->GetKey = 0;
}

/*按键状态检测
  参数：按键引脚参数的结构体
*/
void Key_StatusPoll(Key_InitTypedef *Key)
{
	switch(Key->KeyStatus)
	{
		case 0:
		{
			if(GPIO_ReadInputDataBit(Key->GPIOx, Key->GPIO_Pin) == Bit_RESET)
			{
				Key->RecordTim = SYSTime_Getms();
				Key->KeyStatus = 1;
			}
			break;
		}
		
		case 1:
		{
			if((SYSTime_Getms() - Key->RecordTim) > 10)
			{
				if(GPIO_ReadInputDataBit(Key->GPIOx, Key->GPIO_Pin) == Bit_RESET)
				{
					Key->KeyStatus = 2;
				}else
				{
					Key->KeyStatus = 0;
				}
			}
			break;
		}
		
		case 2:
		{
			if(GPIO_ReadInputDataBit(Key->GPIOx, Key->GPIO_Pin) == Bit_SET)
			{
				Key->GetKey = 1;
				Key->KeyStatus = 0;
			}else
			{
				if((SYSTime_Getms() - Key->RecordTim) > Key->LongTime)
				{
					Key->GetKey = 2;
					Key->KeyStatus = 3;
				}
			}
			break;
		}
		
		case 3:
		{
			if(GPIO_ReadInputDataBit(Key->GPIOx, Key->GPIO_Pin) == Bit_SET)
			{
				Key->GetKey = 3;
				Key->KeyStatus = 0;
			}
			break;
		}
	}
}

/*返回按键的结果
  参数：按键引脚参数的结构体
	返回：ret = 0：没有按下
				ret = 1：短按
				ret = 2：长按不松手
				ret = 3：长按松手
*/
uint8_t Key_Get(Key_InitTypedef *Key)
{
	uint8_t ret;
	
	ret = Key->GetKey;
	Key->GetKey = 0;
	
	return ret;
}
