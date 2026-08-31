#ifndef __MYSPI_H
#define __MYSPI_H


/*配置S片选的引脚参数
  参数1：SS外设时钟初始化
	参数2：SS引脚的GPIOx
	参数3：SS引脚的Pin
*/
typedef struct 
{
	uint32_t RCC_APB2Periph;
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_Pin;
}MySPI_SS_InitTypedef;


void MySPI_Init(const MySPI_SS_InitTypedef* SS);
void MySPI_SSStart(const MySPI_SS_InitTypedef* SS);
void MySPI_SSStop(const MySPI_SS_InitTypedef* SS);
uint8_t MySPI_SwapByte(uint8_t ByteSend);
void MySPI_SendByte(uint8_t ByteSend);
uint8_t MySPI_WriteByte(void);

#endif
