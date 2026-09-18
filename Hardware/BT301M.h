#ifndef __BT301M_H
#define __BT301M_H

#include <string.h>

extern char ErrorInformation[];

void BT301M_Init(void);
void BT301M_SendATCmd(char *Cmd);
void BT301M_EnQueue(char *Data);
uint8_t BT301M_DeQueue(char *OutData);
uint8_t BT301M_AwaitReply(void);

#endif
