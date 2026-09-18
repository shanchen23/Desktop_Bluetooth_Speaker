#ifndef __LCD_H
#define __LCD_H

#include "MyRTC.h"

#define COLOR_WHITE 0xFFFF
#define COLOR_BLACK 0x0000
#define COLOR_CYAN 0x07FF
#define COLOR_ORANGRED 0XFA20

void LCD_Init(void);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_ALLClear(uint16_t color);
void LCD_WindowClear(uint16_t x, uint16_t y, uint16_t Stop_x, uint16_t Stop_y, uint16_t color);
void LCD_ShowChar(uint16_t x, uint16_t y, char *utf8, uint16_t color, uint16_t BackgroundColor);
void LCD_ShowString(uint16_t x, uint16_t y, char *utf8, uint16_t color, uint16_t BackgroundColor);
void LCD_ShowUnsigNum(uint16_t x, uint16_t y, uint32_t Num, uint16_t color,  uint16_t BackgroundColor);
void LCD_ShowTimeNum(uint16_t x, uint16_t y, uint8_t Num, uint16_t color,  uint16_t BackgroundColor);
void LCD_ShowPattern(uint16_t x, uint16_t y,  uint16_t Stop_x, uint16_t Stop_y, uint8_t PatternNum, uint16_t color,  uint16_t BackgroundColor);

#endif
