#include "stm32f10x.h"
#include "MySPI.h"
#include "W25Q64_Ins.h"
#include "W25Q64.h"

uint32_t CharNum = 0;
//SS配置
const GPIO_Pin_InitTypedef W25Q64_SS_Typedef = {.RCC_APB2Periph = RCC_APB2Periph_GPIOA,
																								.GPIOx = (GPIO_TypeDef *)GPIOA,
																								.GPIO_Pin = GPIO_Pin_8};

//初始化
void W25Q64_Init(void)
{
	MySPI_Init(&W25Q64_SS_Typedef);
	W25Q64_GetCharNum();
}

//写使能
void W25Q64_WriteEnable(void)
{
	MySPI_SSStart(&W25Q64_SS_Typedef);
	MySPI_SwapByte(W25Q64_WRITE_ENABLE);
	MySPI_SSStop(&W25Q64_SS_Typedef);
}

//等待忙结束
void W25Q64_WaiteBuay(void)
{
	uint16_t PeriodTime= 0;
	
	MySPI_SSStart(&W25Q64_SS_Typedef);
	MySPI_SwapByte(W25Q64_READ_STATUS_REGISTER_1);
	while((MySPI_SwapByte(W25Q64_DUMMY_BYTE) & 0x01) == 0x01)
	{
		PeriodTime++;
		if(PeriodTime > 1000)
		{
			break;
		}
	}
	MySPI_SSStop(&W25Q64_SS_Typedef);
}

/*指定地址连续写入数据
参数1：24位地址
参数2：发送数据的数组
参数2：发送到8Bit数据数量
*/
void W25Q64_PageProgram(uint32_t Address, uint8_t *dataArray, uint16_t Count)
{
	W25Q64_WaiteBuay();
	W25Q64_WriteEnable();
	
	MySPI_SSStart(&W25Q64_SS_Typedef);
	MySPI_SwapByte(W25Q64_PAGE_PROGRAM);
	MySPI_SwapByte((uint8_t )(Address >> 16));
	MySPI_SwapByte((uint8_t )(Address >> 8));
	MySPI_SwapByte((uint8_t )Address);
	
	for(uint16_t i = 0; i < Count; i++)
	{
		MySPI_SwapByte(dataArray[i]);
	}
	
	MySPI_SSStop(&W25Q64_SS_Typedef);
}

//全部擦除
void W25Q64_ALLErase(void)
{
	W25Q64_WaiteBuay();
	W25Q64_WriteEnable();
	
	MySPI_SSStart(&W25Q64_SS_Typedef);
	MySPI_SwapByte(W25Q64_CHIP_ERASE);
	MySPI_SSStop(&W25Q64_SS_Typedef);
}

/*扇区擦除
参数1：扇区的起始地址
*/
void W25Q64_SectErase(uint32_t Address)
{
	W25Q64_WaiteBuay();
	W25Q64_WriteEnable();
	
	MySPI_SSStart(&W25Q64_SS_Typedef);
	MySPI_SwapByte(W25Q64_SECTOR_ERASE_4KB);
	MySPI_SwapByte((uint8_t )(Address >> 16));
	MySPI_SwapByte((uint8_t )(Address >> 8));
	MySPI_SwapByte((uint8_t )Address);
	MySPI_SSStop(&W25Q64_SS_Typedef);
}

/*指定地址连续读取数据
参数1：24位地址
参数2：数组存放读取数据
参数2：读取到8Bit数据数量
*/
void W25Q64_ReadData(uint32_t Address, uint8_t *dataArray, uint32_t Count)
{
	MySPI_SSStart(&W25Q64_SS_Typedef);
	MySPI_SwapByte(W25Q64_READ_DATA);
	MySPI_SwapByte((uint8_t )(Address >> 16));
	MySPI_SwapByte((uint8_t )(Address >> 8));
	MySPI_SwapByte((uint8_t )Address);
	
	for(uint32_t i = 0; i < Count; i++)
	{
		dataArray[i] = MySPI_SwapByte(W25Q64_DUMMY_BYTE);
	}
	
	MySPI_SSStop(&W25Q64_SS_Typedef);
}

/*获取字库字符数量，并将字符数量提供给后续函数
返回：字符数量
*/
uint32_t W25Q64_GetCharNum(void)
{
	uint32_t Num = 0;
	uint8_t NumData[4] = {0};
	
	W25Q64_ReadData(CHARNUM_START_ADDRESS, NumData, 4);
	Num = (uint32_t )(NumData[0] << 24)
			| (uint32_t )(NumData[1] << 16)
			| (uint32_t )(NumData[2] << 8)
			| (uint32_t ) NumData[3];
	
	CharNum = Num;
	return Num;
}

/*查找字符索引表
参数：索引表结构体
*/
void W25Q64_GetIndexTable(W25Q64_IndexTableTypedef *IndexTable)
{
	uint8_t Index_Flag = 0;// 1:找到了  0:没找到
	uint8_t ExtractArray[10];
	uint16_t Mid_CodePoint;
	int32_t Mid;
	int32_t Left;
	int32_t Right;
	
	Left = 0;
	Right = CharNum - 1;
	
	while(Left <= Right)
	{
		Mid = (Right - Left) / 2 + Left;
		
		W25Q64_ReadData(INDEXES_START_ADDRESS + Mid * 10, ExtractArray, 2);
		Mid_CodePoint = (uint16_t )(ExtractArray[0] << 8)
									| (uint16_t )ExtractArray[1];
		
		if(Mid_CodePoint == IndexTable->CodePoint)
		{
			Index_Flag = 1;
			break;
		}else if(IndexTable->CodePoint > Mid_CodePoint)
		{
			Left = Mid + 1;
		}else
		{
			Right = Mid - 1;
		}
	}
	
	if(Index_Flag == 1)
	{
		W25Q64_ReadData(INDEXES_START_ADDRESS + Mid * 10, ExtractArray, 10);
		
		IndexTable->Wide = (uint16_t )(ExtractArray[2] << 8)
											| (uint16_t )ExtractArray[3];
		
		IndexTable->Tall = (uint16_t )(ExtractArray[4] << 8)
											| (uint16_t )ExtractArray[5];
		
		IndexTable->DataOffset = (uint32_t )(ExtractArray[6] << 24)
														| (uint32_t )(ExtractArray[7] << 16)
														| (uint32_t )(ExtractArray[8] << 8)
														| (uint32_t ) ExtractArray[9];
	}else
	{
		//索引不到的传"口"的地址
		IndexTable->Wide = 32;
		IndexTable->Tall = 32;
		IndexTable->DataOffset = 0x11A0C0;
	}
	
}
