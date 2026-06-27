#include "led.h"


/*
引脚说明

LED0--PF9
LED1--PF10
LED2--PE13
LED3--PE14


PF9输出低电平，灯亮；输出高电平，灯灭

*/
void Led_Init(void)
{
	GPIO_InitTypeDef	GPIOF_InitStruct;
	GPIO_InitTypeDef	GPIOE_InitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);	
	

	GPIOF_InitStruct.GPIO_Pin	= GPIO_Pin_9|GPIO_Pin_10;	//引脚9，10
	GPIOF_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;//输出模式
	GPIOF_InitStruct.GPIO_OType	= GPIO_OType_PP;//推挽模式
	GPIOF_InitStruct.GPIO_Speed  = GPIO_Speed_25MHz; //速度
	GPIOF_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOF, &GPIOF_InitStruct);	
	

	GPIOE_InitStruct.GPIO_Pin	= GPIO_Pin_13|GPIO_Pin_14;	//引脚13,14
	GPIOE_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;//输出模式
	GPIOE_InitStruct.GPIO_OType	= GPIO_OType_PP;//推挽模式
	GPIOE_InitStruct.GPIO_Speed  = GPIO_Speed_25MHz; //速度
	GPIOE_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOE, &GPIOE_InitStruct);	
	
	PFout(9) = 1;
	PFout(10) = 1;
	PEout(13) = 1;
	PEout(14) = 1;
	
		
	
}
