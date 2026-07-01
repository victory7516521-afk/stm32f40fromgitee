#ifndef __LED_H
#define __LED_H

#include "stm32f4xx.h"

#define RCC_AHB1ENR     *((volatile unsigned int *)(0x40023800+0x30)) //值强制转换地址，通过解引用访问地址空间
#define GPIOF_MODER		*((volatile unsigned int *)(0x40021400+0x00)) //值强制转换地址，通过解引用访问地址空间
#define GPIOF_OTYPER	*((volatile unsigned int *)(0x40021400+0x04)) //值强制转换地址，通过解引用访问地址空间
#define GPIOF_OSPEEDR	*((volatile unsigned int *)(0x40021400+0x08)) //值强制转换地址，通过解引用访问地址空间
#define GPIOF_PUPDR		*((volatile unsigned int *)(0x40021400+0x0C)) //值强制转换地址，通过解引用访问地址空间
#define GPIOF_ODR		*((volatile unsigned int *)(0x40021400+0x14)) //值强制转换地址，通过解引用访问地址空间


void Led_Init(void);



#endif
