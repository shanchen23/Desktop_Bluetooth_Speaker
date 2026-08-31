#ifndef __LCD_H
#define __LCD_H

#include "MyRTC.h"

void LCD_Init(void);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_ALLClear(uint16_t color);
void LCD_ClearWindow(uint16_t x, uint16_t y, uint16_t Stop_x, uint16_t Stop_y, uint16_t color);
void LCD_ShowChar(uint16_t x, uint16_t y, char Char, uint16_t color);
void LCD_ShowString(uint16_t x, uint16_t y, char *Char, uint16_t color);
void LCD_ShowUnsignNum(uint16_t x, uint16_t y, uint32_t Num, uint16_t color);
void LCD_ShowChinese(uint16_t x, uint16_t y, uint8_t HZ[], uint16_t color);
void LCD_ShowTimeNum(uint16_t x, uint16_t y, uint8_t Num, uint16_t color);

#endif
