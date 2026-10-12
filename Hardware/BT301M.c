#include "stm32f10x.h"                  // Device header
#include "SYSTime.h"
#include "BT301M.h"
#include <string.h>
#include <stdio.h>

#include "LCD.h"

GPIO_Pin_InitTypedef BT301M_ResetPin = {.RCC_APB2Periph = RCC_APB2Periph_GPIOB,
																				.GPIOx = (GPIO_TypeDef *)GPIOB,
																				.GPIO_Pin = GPIO_Pin_15};
GPIO_Pin_InitTypedef PAM8403_Switch = {.RCC_APB2Periph = RCC_APB2Periph_GPIOB,
																				.GPIOx = (GPIO_TypeDef *)GPIOB,
																				.GPIO_Pin = GPIO_Pin_15};
BT301M_BTINFO_Typedef BT301M_BTINFO_Data = {0};
BT301M_LYRIC_Typedef BT301M_LYRIC_Data = {0};
uint8_t BT301M_MODE_Data = 4;//0:蓝牙模式，1:U盘模式，2:TF卡模式，3:LINEIN 模式，4:待机模式，5:深度休眠模式
uint8_t BT301M_WARNING_Data = 0;//0：禁用提示音 1：启用提示音
uint8_t BT301M_AUTOPLAY_Data = 0;//0：关闭蓝牙连接自动播放音乐 1：启用蓝牙连接自动播放音乐
uint8_t BT301M_LANGID_Data = 0;

char ErrorInformation[50] = {0};

char BT301M_RX_QUEUE[20][256] = {0};
char BT301M_RX_Buf[256] = {0};
uint8_t BT301M_RxWrite = 0;
uint8_t BT301M_RxRead = 0;
volatile uint8_t BT301M_RxCount = 0; 

char BT301M_TX_QUEUE[8][32] = {0};
char BT301M_TX_Buf[32] = {0};
uint8_t BT301M_TxWrite = 0;
uint8_t BT301M_TxRead = 0;
uint8_t BT301M_TxCount = 0; 

volatile uint8_t BT301M_TxStart = 0;  // 0=空闲 1=发命令 2=发\r 3=发\n
uint32_t BT301M_TXSendTime = 0;


//printf底层函数重写，将printf的数据从USART1串口发送
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	return ch;
}

/*将发送消息加入队列，一个队列以'\0'结尾
	参数：入队队列的首地址
*/
void BT301M_TXEnQueue(char *Data)
{
	if(BT301M_TxCount < 8)
	{
		strcpy(BT301M_TX_QUEUE[BT301M_TxWrite % 8], Data);
		BT301M_TxWrite = (BT301M_TxWrite + 1) % 8;
		BT301M_TxCount++;
	}else
	{
		sprintf(ErrorInformation, "TX队列已满");
	}
}

/*将发送消息取出队列，一个队列以'\0'结尾
	参数：输出数据存放的首地址
*/
void BT301M_TXDeQueue(char *OutData)
{
	strcpy(OutData, BT301M_TX_QUEUE[BT301M_TxRead % 8]);
	BT301M_TxRead = (BT301M_TxRead + 1) % 8;
	BT301M_TxCount--;
}


//在BT301M_Waiting空闲的时候处理发送AT指令
void BT301M_SendATCmd(void)
{
  if(BT301M_TxStart != 0)	return;
  if(BT301M_TxCount == 0)	return;
	if((SYSTime_Getms() - BT301M_TXSendTime) < 50)	return;

  BT301M_TXDeQueue(BT301M_TX_Buf);
	BT301M_TxStart = 1;
  USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
  BT301M_TXSendTime = SYSTime_Getms();
}

/*将接收消息加入队列，一个队列以'\0'结尾
	参数：入队队列的首地址
*/
void BT301M_RXEnQueue(char *Data)
{
	if(BT301M_RxCount < 20)
  {
		strcpy(BT301M_RX_QUEUE[BT301M_RxWrite % 20], Data);
		BT301M_RxWrite = (BT301M_RxWrite + 1) % 20;
		BT301M_RxCount++;
  }else
	{
		sprintf(ErrorInformation, "RX队列已满");
	}
}

/*将接收消息取出队列，一个队列以'\0'结尾
	参数：输出数据存放的首地址
*/
void BT301M_RXDeQueue(char *OutData)
{
	strcpy(OutData, BT301M_RX_QUEUE[BT301M_RxRead % 20]);
	BT301M_RxRead = (BT301M_RxRead + 1) % 20;
	BT301M_RxCount--;
}

//分发接收到的消息，一个队列以'\0'结尾
void DistributeRXData(void)
{
	char RX_Buf[256];

	while(BT301M_RxCount > 0)
	{
		BT301M_RXDeQueue(RX_Buf);
		
		if(strncmp(RX_Buf, "ERROR", 5) == 0)
		{
			sprintf(ErrorInformation, "BT301错误");
		}else if(strstr(RX_Buf, "BTINFO") != NULL)
		{
			BT301M_Dispose_BTINFO(RX_Buf);
		}else if(strstr(RX_Buf, "<GC>") != NULL 
						&& strstr(RX_Buf, "</GC>") != NULL 
						&& strstr(RX_Buf, "<QM>") != NULL 
						&& strstr(RX_Buf, "</QM>") != NULL) 
		{
			BT301M_Dispose_LYRIC(RX_Buf);
		}else if(strstr(RX_Buf, "MODE") != NULL)
		{
			BT301M_Dispose_MODE(RX_Buf);
		}else if(strstr(RX_Buf, "WARNING") != NULL )
		{
			BT301M_Dispose_WARNING(RX_Buf);
		}else if(strstr(RX_Buf, "AUTOPLAY") != NULL )
		{
			BT301M_Dispose_AUTOPLAY(RX_Buf);
		}else if(strstr(RX_Buf, "LANGID") != NULL )
		{
			BT301M_Dispose_LANGID(RX_Buf);
		}
		
	}
}

/*处理BTINFO的消息
	参数：处理的数据存放的首地址
*/
void BT301M_Dispose_BTINFO(char *Buf_Data)
{
	uint8_t N = 0;
	
	BT301M_BTINFO_Data.BTINFO_ALLTime = 0;
	BT301M_BTINFO_Data.BTINFO_PlayTime = 0;
	BT301M_BTINFO_Data.BTINFO_Status = 0;
	
	while(Buf_Data[N] != '=' && Buf_Data[N] != '\0')
	{
		N++;
	}
	if(Buf_Data[N] == '\0') return;
	N++;
	
	while(Buf_Data[N] != ',' && Buf_Data[N] != '\0')
	{
		BT301M_BTINFO_Data.BTINFO_ALLTime <<= 4;
		
		if(Buf_Data[N] >= '0' && (Buf_Data[N]) <= '9')
		{
			BT301M_BTINFO_Data.BTINFO_ALLTime |= Buf_Data[N] - '0';
		}else if(Buf_Data[N] >= 'A' && (Buf_Data[N]) <= 'F')
		{
			BT301M_BTINFO_Data.BTINFO_ALLTime |= Buf_Data[N] - 'A' + 10;
		}else if(Buf_Data[N] >= 'a' && (Buf_Data[N]) <= 'f')
		{
			BT301M_BTINFO_Data.BTINFO_ALLTime |= Buf_Data[N] - 'a' + 10;
		}
		N++;
	}
	if(Buf_Data[N] == '\0') return;
	N++;
	
	while(Buf_Data[N] != ',' && Buf_Data[N] != '\0')
	{
		BT301M_BTINFO_Data.BTINFO_PlayTime <<= 4;
		
		if(Buf_Data[N] >= '0' && (Buf_Data[N]) <= '9')
		{
			BT301M_BTINFO_Data.BTINFO_PlayTime |= Buf_Data[N] - '0';
		}else if(Buf_Data[N] >= 'A' && (Buf_Data[N]) <= 'F')
		{
			BT301M_BTINFO_Data.BTINFO_PlayTime |= Buf_Data[N] - 'A' + 10;
		}else if(Buf_Data[N] >= 'a' && (Buf_Data[N]) <= 'f')
		{
			BT301M_BTINFO_Data.BTINFO_PlayTime |= Buf_Data[N] - 'a' + 10;
		}
		N++;
	}
	if(Buf_Data[N] == '\0') return;
	N++;
	
	while(Buf_Data[N] >= '0' && Buf_Data[N] <= '9')
	{
		BT301M_BTINFO_Data.BTINFO_Status = BT301M_BTINFO_Data.BTINFO_Status * 10 + (Buf_Data[N] - '0');
		N++;
	}
}

/*处理LYRIC的消息
	参数：处理的数据存放的首地址
*/
void BT301M_Dispose_LYRIC(char *Buf_Data)
{
	uint8_t N = 0;
	uint8_t i = 0;
	
	while(Buf_Data[N] != '>' 
				|| Buf_Data[N - 1] != 'C'
				|| Buf_Data[N - 2] != 'G' 
				|| Buf_Data[N - 3] != '<')
	{
		if(Buf_Data[N] == '\0')
		{
			sprintf(ErrorInformation, "BT301歌词查找失败");
			return;
		}
		N++;
	}
	N++;
	i = 0;
	
	while(Buf_Data[N] != '<' 
				|| Buf_Data[N + 1] != '/'
				|| Buf_Data[N + 2] != 'G' 
				|| Buf_Data[N + 3] != 'C' 
				|| Buf_Data[N + 4] != '>')
	{
		if(Buf_Data[N] == '\0')
		{
			sprintf(ErrorInformation, "BT301歌词查找失败");
			return;
		}
		BT301M_LYRIC_Data.LYRIC_Libretto[i] = Buf_Data[N];
		N++;
		i++;
	}
	BT301M_LYRIC_Data.LYRIC_Libretto[i] = '\0';
	N += 5;
	i = 0;
	
	
	
	while(Buf_Data[N] != '>' 
				|| Buf_Data[N - 1] != 'M'
				|| Buf_Data[N - 2] != 'Q' 
				|| Buf_Data[N - 3] != '<')
	{
		if(Buf_Data[N] == '\0')
		{
			sprintf(ErrorInformation, "BT301歌词查找失败");
			return;
		}
		N++;
	}
	N++;
	i = 0;
	
	while(Buf_Data[N] != '<' 
				|| Buf_Data[N + 1] != '/'
				|| Buf_Data[N + 2] != 'Q' 
				|| Buf_Data[N + 3] != 'M' 
				|| Buf_Data[N + 4] != '>')
	{
		if(Buf_Data[N] == '\0')
		{
			sprintf(ErrorInformation, "BT301歌词查找失败");
			return;
		}
		BT301M_LYRIC_Data.LYRIC_Name[i] = Buf_Data[N];
		N++;
		i++;
	}
	BT301M_LYRIC_Data.LYRIC_Name[i] = '\0';
	N += 5;
	i = 0;
}

/*处理MODE的消息
	参数：处理的数据存放的首地址
*/
void BT301M_Dispose_MODE(char *Buf_Data)
{
	if(strstr(Buf_Data, "MODE=BT") != NULL)
	{
		BT301M_MODE_Data = 0;
	}else if(strstr(Buf_Data, "MODE=USB") != NULL)
	{
		BT301M_MODE_Data = 1;
	}else if(strstr(Buf_Data, "MODE=SD") != NULL)
	{
		BT301M_MODE_Data = 2;
	}else if(strstr(Buf_Data, "MODE=AUX") != NULL)
	{
		BT301M_MODE_Data = 3;
	}else if(strstr(Buf_Data, "MODE=IDLE") != NULL)
	{
		BT301M_MODE_Data = 4;
	}else if(strstr(Buf_Data, "MODE=SLEEP") != NULL)
	{
		BT301M_MODE_Data = 5;
	}
}

/*处理WARNING的消息
	参数：处理的数据存放的首地址
*/
void BT301M_Dispose_WARNING(char *Buf_Data)
{
	uint8_t N = 0;
	
	while(Buf_Data[N] != '=' && Buf_Data[N] != '\0')
	{
		N++;
	}
	if(Buf_Data[N] == '\0') return;
	N++;
	
	BT301M_WARNING_Data = Buf_Data[N] - '0';
}

/*处理WARNING的消息
	参数：处理的数据存放的首地址
*/
void BT301M_Dispose_AUTOPLAY(char *Buf_Data)
{
	uint8_t N = 0;
	
	while(Buf_Data[N] != '=' && Buf_Data[N] != '\0')
	{
		N++;
	}
	if(Buf_Data[N] == '\0') return;
	N++;
	
	BT301M_AUTOPLAY_Data = Buf_Data[N] - '0';
}

/*处理LANGID的消息
	参数：处理的数据存放的首地址
*/
void BT301M_Dispose_LANGID(char *Buf_Data)
{
	uint8_t N = 0;
	
	while(Buf_Data[N] != '=' && Buf_Data[N] != '\0')
	{
		N++;
	}
	if(Buf_Data[N] == '\0') return;
	N++;
	
	BT301M_LANGID_Data = Buf_Data[N] - '0';
}

//初始化
void BT301M_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(BT301M_ResetPin.RCC_APB2Periph, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = BT301M_ResetPin.GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(BT301M_ResetPin.GPIOx, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = PAM8403_Switch.GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PAM8403_Switch.GPIOx, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1,&USART_InitStructure);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
	
	//硬件复位
	GPIO_WriteBit(BT301M_ResetPin.GPIOx, BT301M_ResetPin.GPIO_Pin, Bit_SET);
	SYSTime_Delayms(6);
	GPIO_WriteBit(BT301M_ResetPin.GPIOx, BT301M_ResetPin.GPIO_Pin, Bit_RESET);
	
	SYSTime_Delayms(500); 
	
	BT301M_TXEnQueue("AT+AUTOCONN=0");//关闭上电回连
	DistributeRXData();
	BT301M_TXEnQueue("AT+BTINFO=10");//获取播放时间和状态
	DistributeRXData();
	BT301M_TXEnQueue("AT+LYRIC=1");//获取歌词
	DistributeRXData();
	BT301M_TXEnQueue("AT+MODE");//查询蓝牙模式
	DistributeRXData();
	BT301M_TXEnQueue("AT+MODE=IDLE");//进入待机模式
	DistributeRXData();
	
	GPIO_WriteBit(PAM8403_Switch.GPIOx, PAM8403_Switch.GPIO_Pin, Bit_RESET);
}


//串口1接收中断
void USART1_IRQHandler(void)
{
	static uint8_t RX_N = 0;
	static uint8_t TX_N = 0;
	
	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
	{
    switch(BT301M_TxStart)
    {
        case 1:
            if(BT301M_TX_Buf[TX_N] != '\0')
						{
							USART_SendData(USART1, BT301M_TX_Buf[TX_N]);
							TX_N++;
						}else
						{
               BT301M_TxStart = 2;
						}
            break;
        case 2:
            USART_SendData(USART1, '\r');
            BT301M_TxStart = 3;
            break;
        case 3:
            USART_SendData(USART1, '\n');
            BT301M_TxStart = 0;
						TX_N = 0;
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
            break;
        default: break;
    }
	}
	
	
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
	{
		
		BT301M_RX_Buf[RX_N] = USART_ReceiveData(USART1);
		if (BT301M_RX_Buf[RX_N] == '\n' && BT301M_RX_Buf[RX_N - 1] == '\r')
		{
			BT301M_RX_Buf[RX_N - 1] = '\0';
			BT301M_RXEnQueue(BT301M_RX_Buf);
			RX_N = 0;
		}else
		{
			if(RX_N < 256 - 2)
			{
				RX_N++;
			}else
			{
				sprintf(ErrorInformation, "BT301消息越界");
				RX_N = 0;
			}
		}
	}
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}
