#include "led.h"

void Led_Init(void)
{
	//打开GPIOF组时钟
//	RCC_AHB1ENR |= (0x01<<5); 		//5位置1

//	//PF9配置为通用输出模式
//	GPIOF_MODER &= ~(0x01<<19);  	//19位清0
//	GPIOF_MODER |= (0x01<<18);   	//18位置1
//	
//	//输出推挽
//	GPIOF_OTYPER &= ~(0x01<<9);  	//9位清0
//	
//	//输出速度25MHZ
//	GPIOF_OSPEEDR &= ~(0x01<<19);  	//19位清0
//	GPIOF_OSPEEDR |= (0x01<<18);   	//18位置1
//	
//	//设置为上拉
//	GPIOF_PUPDR &= ~(0x01<<19);  	//19位清0
//	GPIOF_PUPDR |= (0x01<<18);   	//18位置1
	
	//结构体
	GPIO_InitTypeDef GPIO_InitStruct;
	//使能GPIOF时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	//使能GPIOE时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	
	//设置引脚
	GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_9|GPIO_Pin_10;
	//设置为通用输出模式
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;
	//设置为推挽输出
	GPIO_InitStruct.GPIO_OType  = GPIO_OType_PP;
	//设置为25MHz
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_25MHz;
	//设置为上拉电阻
	GPIO_InitStruct.GPIO_PuPd 	= GPIO_PuPd_UP;
	
	//初始化GPIO
	GPIO_Init(GPIOF,&GPIO_InitStruct);
	
	//设置引脚
	GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_13|GPIO_Pin_14;
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	
	//灯灭
	GPIO_SetBits(GPIOF,GPIO_Pin_9);
	GPIO_SetBits(GPIOF,GPIO_Pin_10);
	GPIO_SetBits(GPIOE,GPIO_Pin_13);
	GPIO_SetBits(GPIOE,GPIO_Pin_14);
	
}
