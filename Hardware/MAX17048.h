#ifndef _MAX17048_H
#define _MAX17048_H

void MAX17048_Init(void);
void MAX17048_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT);
void MAX17048_WriteReg(uint8_t RegAddress, uint16_t Data);
uint16_t MAX17048_ReadReg(uint8_t RegAddress);
uint16_t MAX17048_GetVoltage(void);
uint8_t MAX17048_GetSOC(void);
	
#endif
