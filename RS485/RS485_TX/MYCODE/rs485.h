#ifndef __RS485_H
#define __RS485_H
#include "stm32f4xx.h"
#include "sys.h"


#define RS485_RE  PGout(8)



void Rs485_Init(int MYBaudRate);
void Rs485_Send(u8 *data, u16 len);


#endif