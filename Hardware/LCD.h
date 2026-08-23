#ifndef __LCD_H
#define __LCD_H

void LCD_Init(void);
void LCD_WriteCom(uint8_t LCD_Com);
void LCD_WriteData(uint8_t LCD_Data);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_Clear(uint16_t color);
void LCD_ShowChar(uint16_t x, uint16_t y, char Char, uint16_t color);
void LCD_ShowString(uint16_t x, uint16_t y, char *Char, uint16_t color);
void LCD_ShowSignedNum(uint16_t x, uint16_t y, uint32_t Num, uint16_t color);
void LCD_ShowChinese(uint16_t x, uint16_t y, uint8_t HZ[], uint16_t color);

#endif
