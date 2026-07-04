#ifndef __USART_H
#define __USART_H


#include "stm32f4xx.h" //这个头文件包含所有头文件
#include "sys.h"


extern  volatile u8 g_flag; 
extern  volatile u8 g_data; 

void Usart1_Init(int BaudRate);




#endif