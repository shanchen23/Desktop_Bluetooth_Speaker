#include "stm32f10x.h"                  // Device header
#include "MySPI.h"

#define SPI_SCK_Pin GPIO_Pin_5     //PA5
#define SPI_MISO_Pin GPIO_Pin_6    //PA6
#define SPI_MOSI_Pin GPIO_Pin_7	   //PA7

/*初始化
  参数：SS引脚参数
*/
void MySPI_Init(const MySPI_SS_InitTypedef* SS)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//配置SCK和MOSI
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = SPI_SCK_Pin | SPI_MOSI_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	//配置MISO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = SPI_MISO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	
	RCC_APB2PeriphClockCmd(SS->RCC_APB2Periph ,ENABLE);
	//配置SS
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = SS->GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SS->GPIOx,&GPIO_InitStructure);
	
	//配置SPI
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//36M
	SPI_InitStructure.SPI_CPHA = 0;
	SPI_InitStructure.SPI_CPOL = 0;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;//高位先行
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(SPI1,&SPI_InitStructure);
	
	SPI_Cmd(SPI1,ENABLE);
	
	//初始化片选为高电平
	GPIO_WriteBit(SS->GPIOx,SS->GPIO_Pin,Bit_SET);
}


/*片选信号的开始时序
  参数：SS引脚参数
*/
void MySPI_SSStart(const MySPI_SS_InitTypedef* SS)
{
	GPIO_WriteBit(SS->GPIOx,SS->GPIO_Pin,Bit_RESET);
}

/*片选信号的停止时序
  参数：SS引脚参数
*/
void MySPI_SSStop(const MySPI_SS_InitTypedef* SS)
{
	GPIO_WriteBit(SS->GPIOx,SS->GPIO_Pin,Bit_SET);
}

/*交换数据
	参数：发送的数据
	返回：接收的数据
*/
uint8_t MySPI_SwapByte(uint8_t ByteSend)
{
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE) != SET);
	SPI_I2S_SendData(SPI1,ByteSend);
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) != SET);
	return SPI_I2S_ReceiveData(SPI1);
}

/*只发送数据
	参数：发送的数据
*/
void MySPI_SendByte(uint8_t ByteSend)
{
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE) != SET);
	SPI_I2S_SendData(SPI1,ByteSend);
}

/*只读数据
	返回：接收的数据
*/
uint8_t MySPI_WriteByte(void)
{
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) != SET);
	return SPI_I2S_ReceiveData(SPI1);
}
