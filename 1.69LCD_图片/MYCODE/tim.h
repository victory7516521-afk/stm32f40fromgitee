#ifndef __TIM_H
#define __TIM_H

#include "stm32f4xx.h"
#include "sys.h"


void Tim3_init(int psc, int period);
void Tim3_count_init(int psc, int period);


#endif
