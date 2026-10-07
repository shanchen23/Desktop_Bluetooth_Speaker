#ifndef __BT301M_H
#define __BT301M_H

#include <string.h>

/*音乐的BT消息
  参数1：当前音乐播放总时长
	参数2：当前音乐播放进度的时间
	参数3：当前音乐播放状态
*/
typedef struct 
{
	uint32_t BTINFO_ALLTime;
	uint32_t BTINFO_PlayTime;
	uint8_t BTINFO_Status;
	
}BT301M_BTINFO_Typedef;

typedef struct 
{
	char LYRIC_Name[256];
	char LYRIC_Libretto[256];
	
}BT301M_LYRIC_Typedef;

extern char ErrorInformation[];
extern BT301M_BTINFO_Typedef BT301M_BTINFO_Data;
extern BT301M_LYRIC_Typedef BT301M_LYRIC_Data;
extern uint8_t BT301M_MODE_Data;
extern uint8_t BT301M_WARNING_Data;
extern uint8_t BT301M_AUTOPLAY_Data;
extern uint8_t BT301M_LANGID_Data;

void BT301M_Init(void);
void BT301M_TXEnQueue(char *Data);
void BT301M_TXDeQueue(char *OutData);
void BT301M_SendATCmd(void);
void BT301M_RXEnQueue(char *Data);
void DistributeRXData(void);


void BT301M_Dispose_BTINFO(char *Buf_Data);
void BT301M_Dispose_LYRIC(char *Buf_Data);
void BT301M_Dispose_MODE(char *Buf_Data);
void BT301M_Dispose_WARNING(char *Buf_Data);
void BT301M_Dispose_AUTOPLAY(char *Buf_Data);
void BT301M_Dispose_LANGID(char *Buf_Data);
#endif
