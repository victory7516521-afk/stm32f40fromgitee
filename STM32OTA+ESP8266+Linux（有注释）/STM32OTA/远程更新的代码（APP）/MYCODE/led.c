#include "led.h"


/***********************************************
引脚说明：
LED0连接PF9
PF9输出VCC(1),灯灭
PF9输出GND(0),灯亮

LED1连接PF10
LED2连接PE13
LED3连接PE14

***********************************************/

void Led_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	//使能GPIO F组时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	//使能GPIO E组时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);	
	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_9|GPIO_Pin_10; 		//引脚9，引脚10
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;	//输出
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;	//推挽
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz;	//50MHZ速度
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;		//上拉
	GPIO_Init(GPIOF, &GPIO_InitStruct);
	

	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_13|GPIO_Pin_14; 		//引脚13，引脚14
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;	//输出
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;	//推挽
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz;	//50MHZ速度
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;		//上拉
	GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	
	GPIO_SetBits(GPIOF, GPIO_Pin_9);
	GPIO_SetBits(GPIOF, GPIO_Pin_10);
	GPIO_SetBits(GPIOE, GPIO_Pin_13);
	GPIO_SetBits(GPIOE, GPIO_Pin_14);
}

