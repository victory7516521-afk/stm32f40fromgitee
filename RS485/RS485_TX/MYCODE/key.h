#ifndef __KEY_H
#define __KEY_H


#include "stm32f4xx.h" //这个头文件包含所有头文件
#include "sys.h"

typedef enum
{ 
	KEY0_VALUE=1,
	KEY1_VALUE,
	KEY2_VALUE,
	KEY3_VALUE,
}key_value;


void Key_Init(void);
u8 Key_scan(u8 mode);



#endif