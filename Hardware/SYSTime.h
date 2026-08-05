#ifndef __SYSTIME_H_
#define __SYSTIME_H_

typedef struct
{
	uint16_t tm_year;
	uint8_t tm_mon;
	uint8_t tm_mday;
	uint8_t tm_hour;
	uint8_t tm_min;
	uint8_t tm_sec;
	
}TimeDateTypedef;

void SYSTime_Init(void);

#endif
