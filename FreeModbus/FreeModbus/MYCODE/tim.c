#include "tim.h"

/*********************************
定时器说明
TIM3 -- APB1(定时器频率：84MHZ)

TIM3是16位定时器
**********************************/

void Tim3_Init(uint16_t tim)
{

	//4、设置 TIM3_DIER  允许更新中断
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	
	//5、使能定时器。
	TIM_Cmd(TIM3, ENABLE);


	TIM_TimeBaseInitTypeDef  	TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef  			NVIC_InitStruct;
	//1、能定时器时钟。
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	
	TIM_TimeBaseInitStruct.TIM_Prescaler	= (4200-1);			//4200分频，定时器频率84MHZ/4200 = 20000HZ
	TIM_TimeBaseInitStruct.TIM_Period		= tim;				//定时器周期 20000
	TIM_TimeBaseInitStruct.TIM_CounterMode	= TIM_CounterMode_Up;	//向上计数
	TIM_TimeBaseInitStruct.TIM_ClockDivision= TIM_CKD_DIV1;			//分频因子	1脉冲计一个数
	//2、初始化定时器，配置ARR,PSC。
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);


	
	NVIC_InitStruct.NVIC_IRQChannel						= TIM3_IRQn; 		//中断通道，可在stm32f4xx.h文件当中查找
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	= 1;				//抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority			= 1;				//响应优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd					= ENABLE;			//通道使能
	//3、启定时器中断，配置NVIC。
	NVIC_Init(&NVIC_InitStruct);	

	//清除溢出中断标志位  
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  
	//定时器溢出中断关闭  
	  TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE);  
	//失能定时器  
	TIM_Cmd(TIM3, DISABLE);   
 
}

////编写中断服务函数。 每隔1ms进入中断
//void  TIM3_IRQHandler(void)
//{
//	//判断更新标志位是否1
//	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
//	{

//		//清空更新标志位
//		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	
//	}



//}
