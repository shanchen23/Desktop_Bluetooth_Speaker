#ifndef __MyRTC_H
#define __MyRTC_H

typedef struct
{
	uint16_t tm_year;
	uint8_t tm_mon;
	uint8_t tm_mday;
	uint8_t tm_hour;
	uint8_t tm_min;
	uint8_t tm_sec;
	
}TimeDateTypedef;

void MyRTC_Init(void);
void MyRTC_SetTime(TimeDateTypedef *SetTime);
TimeDateTypedef MyRTC_ReadTime(void);

#endif
