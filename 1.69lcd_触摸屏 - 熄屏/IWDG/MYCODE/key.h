#ifndef __KEY_H
#define __KEY_H

#include "stm32f4xx.h"

enum
{ 
  KEY0_VALUE = 1,
  KEY1_VALUE, 
  KEY2_VALUE, 
  KEY3_VALUE,
};


void Key_init(void);

u8 key_scanf(u8 mode);

#endif
