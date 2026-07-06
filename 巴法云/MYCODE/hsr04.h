#ifndef __HSR04_H
#define __HSR04_H


#include "stm32f4xx.h" //这个头文件包含所有头文件
#include "sys.h"
#include "delay.h"

void Hsr04_Init(void);
int Get_Hsr04_distance(void);



#endif