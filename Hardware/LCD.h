#ifndef __LCD_H
#define __LCD_H

#include "MyRTC.h"

/* RGB565 常用颜色宏 */
#define COLOR_BLACK             0x0000  // 黑色
#define COLOR_WHITE             0xFFFF  // 白色
#define COLOR_RED               0xF800  // 红色
#define COLOR_GREEN             0x07E0  // 绿色
#define COLOR_BLUE              0x001F  // 蓝色
#define COLOR_CYAN              0x07FF  // 青色
#define COLOR_MAGENTA           0xF81F  // 品红/洋红
#define COLOR_YELLOW            0xFFE0  // 黄色

/* 灰白类 */
#define COLOR_GRAY              0x8410  // 灰色
#define COLOR_SILVER            0xC618  // 银色
#define COLOR_LIGHT_GRAY        0xD69A  // 浅灰色
#define COLOR_DARK_GRAY         0xAD55  // 深灰色
#define COLOR_IVORY             0xFFFE  // 象牙白
#define COLOR_BEIGE             0xF7BB  // 米色
#define COLOR_WHITE_SMOKE       0xF7BE  // 烟白色
#define COLOR_EGGSHELL          0xF75B  // 蛋壳白/米白

/* 红橙黄类 */
#define COLOR_ORANGE            0xFD20  // 橙色
#define COLOR_ORANGE_RED        0xFA20  // 橙红色
#define COLOR_DARK_ORANGE       0xFC60  // 深橙色
#define COLOR_GOLD              0xFEA0  // 金色
#define COLOR_PINK              0xFE19  // 粉色
#define COLOR_DEEP_PINK         0xF8B2  // 深粉色
#define COLOR_HOT_PINK          0xFB56  // 亮粉色
#define COLOR_LIGHT_PINK        0xFDB8  // 浅粉色
#define COLOR_CRIMSON           0xD8A7  // 猩红色
#define COLOR_TOMATO            0xFB08  // 番茄色
#define COLOR_CORAL             0xFBEA  // 珊瑚色

/* 棕褐类 */
#define COLOR_BROWN             0xA145  // 棕色
#define COLOR_CHOCOLATE         0xD343  // 巧克力色
#define COLOR_PERU              0xCC27  // 秘鲁色
#define COLOR_TAN               0xD5B1  // 棕褐色
#define COLOR_WHEAT             0xF6F6  // 小麦色

/* 紫粉类 */
#define COLOR_PURPLE            0x8010  // 紫色
#define COLOR_VIOLET            0xEC1D  // 紫罗兰色
#define COLOR_INDIGO            0x4810  // 靛蓝色
#define COLOR_ORCHID            0xDB9A  // 兰花紫
#define COLOR_PLUM              0xDD1B  // 李子紫

/* 蓝青类 */
#define COLOR_NAVY              0x0010  // 海军蓝
#define COLOR_SKY_BLUE          0x867D  // 天蓝色
#define COLOR_DEEP_SKY_BLUE     0x05FF  // 深天蓝
#define COLOR_DODGER_BLUE       0x1C9F  // 道奇蓝
#define COLOR_ROYAL_BLUE        0x435C  // 皇家蓝
#define COLOR_STEEL_BLUE        0x4416  // 钢蓝色
#define COLOR_MIDNIGHT_BLUE     0x18CE  // 午夜蓝
#define COLOR_TURQUOISE         0x471A  // 绿松石色
#define COLOR_AQUAMARINE        0x7FFA  // 碧绿色

/* 绿色类 */
#define COLOR_LIME              0x07E0  // 酸橙色
#define COLOR_SPRING_GREEN      0x07EF  // 春绿色
#define COLOR_SEA_GREEN         0x2C4A  // 海绿色
#define COLOR_FOREST_GREEN      0x2444  // 森林绿
#define COLOR_DARK_GREEN        0x0320  // 深绿色
#define COLOR_LIGHT_GREEN       0x9772  // 浅绿色
#define COLOR_LIME_GREEN        0x3666  // 酸橙绿
#define COLOR_YELLOW_GREEN      0x9E66  // 黄绿色
#define COLOR_OLIVE             0x8400  // 橄榄色



void LCD_Init(void);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_ALLClear(uint16_t color);
void LCD_WindowClear(uint16_t x, uint16_t y, uint16_t Stop_x, uint16_t Stop_y, uint16_t color);
void LCD_ShowChar(uint16_t x, uint16_t y, char *utf8, uint16_t color, uint16_t BackgroundColor);
void LCD_ShowString(uint16_t x, uint16_t y, char *utf8, uint16_t color, uint16_t BackgroundColor);
void LCD_ShowUnsigNum(uint16_t x, uint16_t y, uint32_t Num, uint16_t color,  uint16_t BackgroundColor);
void LCD_ShowTimeNum(uint16_t x, uint16_t y, uint8_t Num, uint16_t color,  uint16_t BackgroundColor);
void LCD_ShowPattern(uint16_t x, uint16_t y,  uint16_t Stop_x, uint16_t Stop_y,const uint8_t *Pattern, uint16_t color,  uint16_t BackgroundColor);
uint16_t LCD_GetString_X_Offset(char *Data);


void LCD_ShowPortionChar(uint16_t x, uint16_t y, char *utf8, uint16_t StartColumn, uint16_t ShowWide, uint16_t color, uint16_t BackgroundColor);
void LCD_ShowStringRoll(uint16_t x, uint16_t y, uint16_t ShowWide, char *Data, uint16_t *offset, uint16_t color, uint16_t BackgroundColor);

#endif
