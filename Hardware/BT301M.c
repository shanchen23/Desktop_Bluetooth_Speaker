#include "stm32f10x.h"                  // Device header
#include "SYSTime.h"
#include "BT301M.h"
#include <string.h>
#include <stdio.h>

GPIO_Pin_InitTypedef BT301M_ResetPin = {.RCC_APB2Periph = RCC_APB2Periph_GPIOB,
																					.GPIOx = (GPIO_TypeDef *)GPIOB,
																					.GPIO_Pin = GPIO_Pin_15};
char ErrorInformation[50] = {0};
uint8_t Queue[256] = {0};
uint16_t QueueWrite  = 0;
uint16_t QueueRead = 0;							

//printf底层函数重写，将printf的数据从USART1串口发送
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	return ch;
}

/*发送AT指令
	参数：AT指令
*/
void BT301M_SendATCmd(char *Cmd)
{
	for(uint8_t i = 0; Cmd[i] != '\0'; i++)
	{
		USART_SendData(USART1, Cmd[i]);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	}
	USART_SendData(USART1, 0x0D);
	USART_SendData(USART1, 0x0A);
}

/*将消息加入队列，一个队列以'\0'结尾
	参数：入队队列的首地址
*/
void BT301M_EnQueue(char *Data)
{
	for(uint16_t i = 0; Data[i] != '\0'; i++)
	{
		Queue[QueueWrite  & 255] = Data[i];
		QueueWrite ++;
	}
	Queue[QueueWrite  & 255] = '\0';
	QueueWrite ++;
}

/*将消息取出队列，一个队列以'\0'结尾
	参数：输出数据存放的首地址
	返回：是否收到消息
*/
uint8_t BT301M_DeQueue(char *OutData)
{
	uint16_t i = 0;
	
	if(QueueWrite  == QueueRead)
	{
		return 0;
	}
	
	while(Queue[QueueRead] != '\0')
	{
		OutData[i] = Queue[QueueRead & 255];
		QueueRead++;
		i++;
	}
	QueueRead++;
	OutData[i] = '\0';
	
	return 1;
}

/*等待回复"OK"或者"ERROR"
返回：是否收到"OK"，否收为1，没收到或超时为0
*/
uint8_t BT301M_AwaitReply(void)
{
	uint32_t n = 0;
	char RxBuf[256] = {0};
	
	while(n < 100000)
	{
		if(BT301M_DeQueue(RxBuf))
		{
			if(strncmp(RxBuf, "OK", 5) == 0) return 1;
			if(strncmp(RxBuf, "ERROR", 5) == 0)
			{
				sprintf(ErrorInformation, "BT301错误");
				return 0;
			}
		}
		n++;
	}
	sprintf(ErrorInformation, "响应超时");
	return 0;
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
	
	BT301M_SendATCmd("AT");
	BT301M_AwaitReply();
	
	BT301M_SendATCmd("AT+RESTORE");
	BT301M_AwaitReply();
	
	BT301M_SendATCmd("AT+SSP=1");
	BT301M_AwaitReply();
	
//	//char *strpbrk(const char *str1, const char *str2)   索引str1中是否含有str2，并返回位置
}


//串口1接收中断
void USART1_IRQHandler(void)
{
	static char Data_Buf[256] = {0};
	static uint16_t n = 0;
	
	if(USART_GetFlagStatus(USART1, USART_IT_RXNE) == SET)
	{
		Data_Buf[n] = USART_ReceiveData(USART1);
		if (Data_Buf[n] == '\n' && Data_Buf[n - 1] == '\r')
		{
			Data_Buf[n - 1] = '\0';
			BT301M_EnQueue(Data_Buf);
			n = 0;
		}else
		{
			n++;
		}
	}
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}


