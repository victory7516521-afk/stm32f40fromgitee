#include "exti.h"

/*********************************************
S1(KEY1)连接 PA0
按键未按下，PA0为高电平
按键按下，   PA0为低电平

当按键按下瞬间，为下降沿。
********************************************/
void Exti_PA0_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStruct;
	EXTI_InitTypeDef  EXTI_InitStruct;
	NVIC_InitTypeDef  NVIC_InitStruct;
	
	
	//使能SYSCFG时钟： 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	//使能GPIO A组时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_0;		//引脚0
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_IN;		//输入模式
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;		//上拉
	GPIO_Init(GPIOA, &GPIO_InitStruct);	
	
	//设置IO口与中断线的映射关系。 PA0 -- EXTT0
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	
	
	EXTI_InitStruct.EXTI_Line	= EXTI_Line0;			//中断线0
	EXTI_InitStruct.EXTI_Mode	= EXTI_Mode_Interrupt;	//中断模式
	EXTI_InitStruct.EXTI_Trigger= EXTI_Trigger_Falling; //下降沿触发
	EXTI_InitStruct.EXTI_LineCmd= ENABLE;				//中断线使能
	//初始化线上中断，设置触发条件等。
    EXTI_Init(&EXTI_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel						= EXTI0_IRQn;		//中断通道 ，在stm32f4xx.h
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	= 2;				//抢占优先级
 	NVIC_InitStruct.NVIC_IRQChannelSubPriority			= 2;				//响应优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd					= ENABLE;			//中断通道使能 
	//配置中断分组（NVIC），并使能中断。
    NVIC_Init(&NVIC_InitStruct);

}

//中断服务函数，所有的中断函数可在startup_stm32f40_41xxx.s查找
//中断服务函数不需要程序员调用，当满足条件后，CPU自动调用
void  EXTI0_IRQHandler(void)
{
	
	if(EXTI_GetITStatus(EXTI_Line0) == SET)
	{
		
		//灯状态变更
		GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
	
	}
	//清除中断线0标志位
    EXTI_ClearITPendingBit(EXTI_Line0);
}
















