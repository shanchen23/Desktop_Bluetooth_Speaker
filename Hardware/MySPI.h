#ifndef __MYSPI_H
#define __MYSPI_H

void MySPI_Init(const GPIO_Pin_InitTypedef* SS);
void MySPI_SSStart(const GPIO_Pin_InitTypedef* SS);
void MySPI_SSStop(const GPIO_Pin_InitTypedef* SS);
uint8_t MySPI_SwapByte(uint8_t ByteSend);

#endif
