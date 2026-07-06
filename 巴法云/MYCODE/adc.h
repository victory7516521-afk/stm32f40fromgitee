#ifndef __ADC_H
#define __ADC_H


#include "stm32f4xx.h" //这个头文件包含所有头文件
#include "sys.h"



void Adc_PA5_Init(void);


u16 Get_Adc_Value(void);


#endif