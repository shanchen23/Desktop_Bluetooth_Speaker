#ifndef __MYSPI_H_
#define __MYSPI_H_

void MySPI_Init(uint32_t SS_RCC_APB2Periph, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void MySPI_SSStart(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void MySPI_SSStop(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t MySPI_SwapByte(uint8_t ByteSend);

#endif
