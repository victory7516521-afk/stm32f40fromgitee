#include "key.h"

/*****************************************
引脚说明

S1按键连接 PA0
按键按下，PA0为低电平
按键未按上，PA0为高电平
*****************************************/
void Key_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	//使能GPIO A组时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_0; 		//引脚0
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_IN;		//输入
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;		//上拉
	GPIO_Init(GPIOA, &GPIO_InitStruct);

}