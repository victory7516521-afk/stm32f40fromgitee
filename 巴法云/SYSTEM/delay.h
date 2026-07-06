#ifndef __DELAY_H
#define __DELAY_H


#include "stm32f4xx.h" //这个头文件包含所有头文件


void Delay_Init(void);
void delay_us(u32 nus);
void delay_ms(u32 nms);
void delay_s(u32 ns);


#endif