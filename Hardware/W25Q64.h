#ifndef __W25Q64_H
#define __W25Q64_H

#define CHARNUM_START_ADDRESS 0x00000000
#define INDEXES_START_ADDRESS 0x00000004
#define DATA_START_ADDRESS 0x00061B42

/*------------------------------------------------------------------

--------------------------------------------------------
|       内存地址范围       |           内容             |
--------------------------------------------------------
| 0x00000000 - 0x00000003 | 字模数量                    |
| 0x00000004 - 0x00061B41 | 索引表     40019 * 10 字节  |
| 0x00061B42 - 0x00542D01 | 点阵数据                    |
--------------------------------------------------------

索引表是按 Unicode 码点从小到大排列的，每条10字节
[ 码点 2 字节][ 宽 2 字节 ][ 高 2 字节 ][ 点阵偏移量 4 字节 ]

--------------------------------------------------------------------*/

/*索引表结构体
  参数1：码点
	参数2：宽
	参数3：高
	参数4：点阵偏移量
*/
typedef struct
{
	uint16_t CodePoint;
	uint16_t Wide;
	uint16_t Tall;
	uint32_t DataOffset;
}W25Q64_IndexTableTypedef;


void W25Q64_Init(void);
void W25Q64_WriteEnable(void);
void W25Q64_WaiteBuay(void);
void W25Q64_PageProgram(uint32_t Address, uint8_t *dataArray, uint16_t Count);
void W25Q64_ALLErase(void);
void W25Q64_SectErase(uint32_t Address);
void W25Q64_ReadData(uint32_t Address, uint8_t *dataArray, uint32_t Count);

uint32_t W25Q64_GetCharNum(void);
void W25Q64_GetIndexTable(W25Q64_IndexTableTypedef *IndexTable);

#endif
