#include "key.h"


/*
引脚说明

key0 -- PA0
key1 -- PE2
key2 -- PE3
key3 -- PE4
按下输入低电平
*/
void Key_Init(void)
{
	GPIO_InitTypeDef	GPIOA_InitStruct;
	GPIO_InitTypeDef	GPIOE_InitStruct;
	
	//使能GPIOA,E
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);	
	
	GPIOA_InitStruct.GPIO_Pin	= GPIO_Pin_0;	//引脚0
	GPIOA_InitStruct.GPIO_Mode	= GPIO_Mode_IN;//输入模式
//	GPIOA_InitStruct.GPIO_OType	= GPIO_OType_PP;//推挽模式输入模式下可省略
//	GPIOA_InitStruct.GPIO_Speed  = GPIO_Speed_25MHz; //速度
	GPIOA_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA, &GPIOA_InitStruct);	
	
	GPIOE_InitStruct.GPIO_Pin	= GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;	//引脚234
	GPIOE_InitStruct.GPIO_Mode	= GPIO_Mode_IN;//输入模式
//	GPIOE_InitStruct.GPIO_OType	= GPIO_OType_PP;//推挽模式
//	GPIOE_InitStruct.GPIO_Speed  = GPIO_Speed_25MHz; //速度
	GPIOE_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOE, &GPIOE_InitStruct);	
	
}
