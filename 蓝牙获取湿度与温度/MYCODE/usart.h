#ifndef __USART_H
#define __USART_H

#include "stm32f4xx.h"

void Usart1_Init(int myBaudRate);

void Usart3_Init(int myBaudRate);


void UART_Print(char *str);
#endif
