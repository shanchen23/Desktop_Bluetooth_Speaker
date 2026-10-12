#include "stm32f10x.h"                  // Device header
#include "SYSTime.h"
#include "Time.h"

//DH11引脚配置（PA1，3.3V供电）
const GPIO_Pin_InitTypedef DH11_IO_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOA,
																							.GPIOx = GPIOA,
																							.GPIO_Pin = GPIO_Pin_1};

float TemperatureData = 0;
float HumidityData = 0;

//初始化
void DH11_Init(void)
{
	RCC_APB2PeriphClockCmd(DH11_IO_Typedef.RCC_APB2Periph, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = DH11_IO_Typedef.GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DH11_IO_Typedef.GPIOx, &GPIO_InitStructure);
	
	GPIO_WriteBit(DH11_IO_Typedef.GPIOx, DH11_IO_Typedef.GPIO_Pin, Bit_SET);
	SYSTime_Delayms(1000);
}

/*等待引脚变电平
  参数1：等待电平
  参数2：超时时间（us）
  返回：1 等到，0 超时
*/
static uint8_t DH11_WaitingLevel(uint8_t Level, uint16_t Tim_us)
{
	uint16_t Tim_Flag = Time_GetTimeus();
	while(GPIO_ReadInputDataBit(DH11_IO_Typedef.GPIOx, DH11_IO_Typedef.GPIO_Pin) != Level)
	{
		if((uint16_t)(Time_GetTimeus() - Tim_Flag) > Tim_us)
		{
			return 0;
		}
	}
	return 1;
}

//读取DH11数据（阻塞读取）
void DH11_ReadData(void)
{
	uint8_t Data[5] = {0};
	uint16_t Tim_Flag = 0;
	
	GPIO_WriteBit(DH11_IO_Typedef.GPIOx, DH11_IO_Typedef.GPIO_Pin, Bit_SET);
	Time_Delayus(2);
	GPIO_WriteBit(DH11_IO_Typedef.GPIOx, DH11_IO_Typedef.GPIO_Pin, Bit_RESET);
	Time_Delayus(20000);
	GPIO_WriteBit(DH11_IO_Typedef.GPIOx, DH11_IO_Typedef.GPIO_Pin, Bit_SET);
	
	if(!DH11_WaitingLevel(Bit_RESET, 100)) return;
	if(!DH11_WaitingLevel(Bit_SET, 100)) return;
	if(!DH11_WaitingLevel(Bit_RESET, 100)) return;
	
	for(uint8_t i = 0; i < 40; i++)
	{
		if(!DH11_WaitingLevel(Bit_SET, 100)) return;
		
		Tim_Flag = Time_GetTimeus();
		while(GPIO_ReadInputDataBit(DH11_IO_Typedef.GPIOx, DH11_IO_Typedef.GPIO_Pin) == Bit_SET)
		{
			if(Time_GetTimeus() - Tim_Flag > 100) return;
		}
		
		if(Time_GetTimeus() - Tim_Flag > 40) Data[i / 8] |= (0x80 >> (i % 8));
	}
	
	if(Data[0] + Data[1] + Data[2] + Data[3] == Data[4])
	{
		HumidityData = Data[0] * 1.0 + Data[1] / 10.0;
		TemperatureData = Data[2] * 1.0 + Data[3] / 10.0;
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
