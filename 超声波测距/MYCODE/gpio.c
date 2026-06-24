#include "gpio.h"


void gpio_Init(void)
{
	GPIO_InitTypeDef	GPIOA_InitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIOA_InitStruct.GPIO_Pin	= GPIO_Pin_2;	//引脚2
	GPIOA_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;//输出模式
	GPIOA_InitStruct.GPIO_OType	= GPIO_OType_PP;//推挽模式
	GPIOA_InitStruct.GPIO_Speed  = GPIO_Speed_25MHz; //速度
	GPIOA_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA, &GPIOA_InitStruct);	
	
	GPIOA_InitStruct.GPIO_Pin	= GPIO_Pin_3;	//引脚3
	GPIOA_InitStruct.GPIO_Mode	= GPIO_Mode_IN;//输入模式
	GPIOA_InitStruct.GPIO_PuPd	= GPIO_PuPd_NOPULL; //下拉
	GPIO_Init(GPIOA, &GPIOA_InitStruct);	
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_2);//输出引脚初始化为低电平
	
	
	
}