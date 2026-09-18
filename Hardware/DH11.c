#include "stm32f10x.h"                  // Device header
#include "SYSTime.h"
#include "Time.h"

//DH11引脚配置
const GPIO_Pin_InitTypedef DH11_IO_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOA,
																							.GPIOx = (GPIO_TypeDef *)GPIOA,
																							.GPIO_Pin = GPIO_Pin_1};

volatile uint16_t DH11_Status = 0;
uint8_t DH11_Data[5] = {0};
float TemperatureData = 0;
float HumidityData = 0;

//初始化
void DH11_Init(void)
{
	RCC_APB2PeriphClockCmd(DH11_IO_Typedef.RCC_APB2Periph,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = DH11_IO_Typedef.GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DH11_IO_Typedef.GPIOx,&GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource1);
	
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	GPIO_WriteBit(DH11_IO_Typedef.GPIOx,DH11_IO_Typedef.GPIO_Pin,Bit_SET);
	
	SYSTime_Delayms(1000);
}

//读取DH11数据
void DH11_ReadData(void)
{
	static uint32_t RecordTime_ms;
	
	switch(DH11_Status)
	{
		//主机拉低，开始
		case 0:
			for(uint8_t i = 0; i < 5; i++)
			{
				DH11_Data[i] = 0x00;
			}
			GPIO_WriteBit(DH11_IO_Typedef.GPIOx,DH11_IO_Typedef.GPIO_Pin,Bit_SET);
			Time_Delayus(2);
			GPIO_WriteBit(DH11_IO_Typedef.GPIOx,DH11_IO_Typedef.GPIO_Pin,Bit_RESET);
			RecordTime_ms = SYSTime_Getms();
			DH11_Status = 1;
			break;
		//到达时间主机拉高，释放
		case 1:
			if(SYSTime_Getms() - RecordTime_ms >= 18)
			{
				GPIO_WriteBit(DH11_IO_Typedef.GPIOx,DH11_IO_Typedef.GPIO_Pin,Bit_SET);
				DH11_Status = 2;
			}
			break;
		//处理数据
		case 7:
			if(DH11_Data[0] + DH11_Data[1] + DH11_Data[2] + DH11_Data[3] == DH11_Data[4])
			{
				HumidityData = DH11_Data[0] * 1.0 + DH11_Data[1] / 10.0;
				TemperatureData = DH11_Data[2] * 1.0 + DH11_Data[3] / 10.0;
				DH11_Status = 0;
			}else
				{
					DH11_Status = 0;
				}
			break;
		default:break;
	}
}

float DH11_GetTempData(void)
{
	return TemperatureData;
}

float DH11_GetHumData(void)
{
	return HumidityData;
}

void EXTI1_IRQHandler(void)
{
	uint8_t EXTI1_Flag;// 0:下降沿 1:上升沿
	static uint16_t RecordTime_us;
	static uint8_t Data_BitNum = 0;
	
	if(GPIO_ReadInputDataBit(DH11_IO_Typedef.GPIOx,DH11_IO_Typedef.GPIO_Pin) == Bit_RESET)
	{
		EXTI1_Flag = 0;
	}else
	{
		EXTI1_Flag = 1;
	}
	
	switch(DH11_Status)
	{
		//从机回应低
		case 2:
			if(EXTI1_Flag == 0)
			{
				RecordTime_us = Time_GetTimeus();
			}else if((Time_GetTimeus() - RecordTime_us >= 78) && (Time_GetTimeus() - RecordTime_us <= 88))
				{
					RecordTime_us = Time_GetTimeus();
					DH11_Status = 3;
				}else
					{
						DH11_Status = 0;
					}
			break;
		//从机回应高
		case 3:
			if(EXTI1_Flag == 0)
			{
				if((Time_GetTimeus() - RecordTime_us >= 80) && (Time_GetTimeus() - RecordTime_us <= 92))
				{
					RecordTime_us = Time_GetTimeus();
					DH11_Status = 4;
				}else
					{
						DH11_Status = 0;
					}
			}
			break;
		//从机回应1Bit数据开始
		case 4:
			if(EXTI1_Flag == 1)
			{
				if((Time_GetTimeus() - RecordTime_us >= 50) && (Time_GetTimeus() - RecordTime_us <= 58))
				{
					RecordTime_us = Time_GetTimeus();
					DH11_Status = 5;
				}else
					{
						Data_BitNum = 0;
						DH11_Status = 0;
					}
			}
			break;
		//1Bit数据
		case 5:
			if(EXTI1_Flag == 0)
			{
				if(Time_GetTimeus() - RecordTime_us <= 40)
				{
					Data_BitNum++;
					RecordTime_us = Time_GetTimeus();
					DH11_Status = 4;
				}else if(Time_GetTimeus() - RecordTime_us <= 80)
					{
						DH11_Data[Data_BitNum / 8] |= (0x80 >> (Data_BitNum % 8));
						Data_BitNum++;
						RecordTime_us = Time_GetTimeus();
						DH11_Status = 4;
					}else
					{
						Data_BitNum = 0;
						DH11_Status = 0;
					}
			}
			if(Data_BitNum >= 40)
			{
				Data_BitNum = 0;
				RecordTime_us = Time_GetTimeus();
				DH11_Status = 6;
			}
			break;
		//从机结束释放
		case 6:
			if(EXTI1_Flag == 1)
			{
				if((Time_GetTimeus() - RecordTime_us >= 52) && (Time_GetTimeus() - RecordTime_us <= 56))
				{
					DH11_Status = 7;
				}else
				{
					DH11_Status = 0;
				}
			}
			break;
		default:break;
	}
	
	EXTI_ClearITPendingBit(EXTI_Line1);
}
