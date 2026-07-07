#include "tim.h"

/*********************************************
功能说明：
TIM3 -- APB1 -- 16位定时器(65535)
TIM3时钟频率： APB1*2 = 42*2 = 84MHZ
**********************************************/

void Tim3_Init(void)
{
	NVIC_InitTypeDef  NVIC_InitStruct;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruct;
	
	//	1、使能定时器时钟。
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	
//	TIM_TimeBaseInitStruct.TIM_Prescaler		= 84-1;   				//84分频  84MHZ/84 = 1MHZ, 即1us计一个数
//	TIM_TimeBaseInitStruct.TIM_Period			= 1000-1;   			//计1000个数，用时1ms
//	TIM_TimeBaseInitStruct.TIM_CounterMode		= TIM_CounterMode_Up; 	//向上计数
//	TIM_TimeBaseInitStruct.TIM_ClockDivision	= TIM_CKD_DIV1;			//分频因子
//	//	2、初始化定时器，配置ARR,PSC。
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);

	TIM_TimeBaseInitStruct.TIM_Prescaler		= 8400-1;   			//8400分频  84000 000HZ/84000 = 10000HZ
	TIM_TimeBaseInitStruct.TIM_Period			= 10000-1;   			//计10000个数，用时1s
	TIM_TimeBaseInitStruct.TIM_CounterMode		= TIM_CounterMode_Up; 	//向上计数
	TIM_TimeBaseInitStruct.TIM_ClockDivision	= TIM_CKD_DIV1;			//分频因子
	//	2、初始化定时器，配置ARR,PSC。
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);	
	
	NVIC_InitStruct.NVIC_IRQChannel						= TIM3_IRQn;		//中断通道 ，在stm32f4xx.h
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	= 2;				//抢占优先级
 	NVIC_InitStruct.NVIC_IRQChannelSubPriority			= 2;				//响应优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd					= ENABLE;			//中断通道使能 
	//	3、启定时器中断，配置NVIC。
    NVIC_Init(&NVIC_InitStruct);
	
	//	4、设置 TIM3_DIER  允许更新中断
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	//	5、使能定时器。
	TIM_Cmd(TIM3, ENABLE);
	
}

//	6、编写中断服务函数。
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		
		//灯状态变更
		GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
	
	}
	//清除中断线0标志位
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}
