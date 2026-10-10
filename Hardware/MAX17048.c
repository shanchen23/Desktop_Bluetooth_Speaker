#include "stm32f10x.h"                  // Device header

#define MAX17048_ADDRESS		0x6C

const GPIO_Pin_InitTypedef MAX17048_SCL_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOB,
																									.GPIOx = (GPIO_TypeDef *)GPIOB,
																									.GPIO_Pin = GPIO_Pin_6};

const GPIO_Pin_InitTypedef MAX17048_SDA_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOB,
																									.GPIOx = (GPIO_TypeDef *)GPIOB,
																									.GPIO_Pin = GPIO_Pin_7};


/*等待EVENT事件
  参数1：I2C外设
	参数2：等待的事件
*/
void MAX17048_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
{
	uint32_t Timeout = 10000;
	
	while (I2C_CheckEvent(I2Cx, I2C_EVENT) != SUCCESS)
	{
		Timeout --;
		if (Timeout == 0)
		{
			break;
		}
	}
}

/*向MAX17048寄存器发送16位数据
  参数1：寄存器地址
  参数2：写入寄存器的数据
  */
void MAX17048_WriteReg(uint8_t RegAddress, uint16_t Data)
{
	I2C_GenerateSTART(I2C1, ENABLE);
	MAX17048_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);
	
	I2C_Send7bitAddress(I2C1, MAX17048_ADDRESS, I2C_Direction_Transmitter);
	MAX17048_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
	
	I2C_SendData(I2C1, RegAddress);
	MAX17048_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING);
	
	I2C_SendData(I2C1, (uint8_t)(Data >> 8));
	MAX17048_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING);
	
	I2C_SendData(I2C1, (uint8_t)Data);
	MAX17048_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED);
	I2C_GenerateSTOP(I2C1, ENABLE);
}

/*读取MAX17048寄存器数据
  参数：寄存器地址
  返回：读取寄存器的数据
 */
uint16_t MAX17048_ReadReg(uint8_t RegAddress)
{
	uint16_t Data = 0;
	
	I2C_GenerateSTART(I2C1, ENABLE);
	MAX17048_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);
	
	I2C_Send7bitAddress(I2C1, MAX17048_ADDRESS, I2C_Direction_Transmitter);
	MAX17048_WaitEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
	
	I2C_SendData(I2C1, RegAddress);
	MAX17048_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED);
	
	I2C_GenerateSTART(I2C1, ENABLE);
	MAX17048_WaitEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT);
	
	I2C_Send7bitAddress(I2C1, MAX17048_ADDRESS, I2C_Direction_Receiver);
	MAX17048_WaitEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);
	
	MAX17048_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED);
	Data |= ((uint16_t )I2C_ReceiveData(I2C1) << 8);
	
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	I2C_GenerateSTOP(I2C1, ENABLE);
	
	MAX17048_WaitEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED);
	Data |= I2C_ReceiveData(I2C1);
	
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	
	return Data;
}

//初始化
void MAX17048_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_APB2PeriphClockCmd(MAX17048_SCL_Typedef.RCC_APB2Periph, ENABLE);
	RCC_APB2PeriphClockCmd(MAX17048_SDA_Typedef.RCC_APB2Periph, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Pin = MAX17048_SCL_Typedef.GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(MAX17048_SCL_Typedef.GPIOx, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Pin = MAX17048_SDA_Typedef.GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(MAX17048_SDA_Typedef.GPIOx, &GPIO_InitStructure);
	
	I2C_InitTypeDef I2C_InitStructure;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_ClockSpeed = 50000;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_Init(I2C1, &I2C_InitStructure);
	
	I2C_Cmd(I2C1, ENABLE);
	
	MAX17048_WriteReg(0x06, 0x4000);
}

/*获取电压
	返回：电压，单位为1mA
*/
uint16_t MAX17048_GetVoltage(void)
{
	uint32_t Raw;
	
	Raw = MAX17048_ReadReg(0x02);
	
	return (uint16_t)((Raw * 78125) / 1000);
}

/*获取放电状态剩余电量
	返回：放电状态剩余电量
*/
uint8_t MAX17048_GetSOC(void)
{
	return (uint8_t)(MAX17048_ReadReg(0x04) >> 8);
}
