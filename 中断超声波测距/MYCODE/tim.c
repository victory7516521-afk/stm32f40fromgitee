#include "tim.h"

/***********************************
定时器说明：

TIM3是芯片的外设。

TIM3 -- APB1 时钟频率：84MHZ
TIM3 -- 16位定时器

************************************/

void Tim3_init(int psc, int period)
{
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	
	
	//1、使能定时器时钟。
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	
	//2、初始化定时器，配置ARR,PSC。
	TIM_TimeBaseInitStruct.TIM_Prescaler	= psc; 		//分频值
	TIM_TimeBaseInitStruct.TIM_Period		= period;	//重载值周期
	TIM_TimeBaseInitStruct.TIM_CounterMode	= TIM_CounterMode_Up; //向上计数
	TIM_TimeBaseInitStruct.TIM_ClockDivision= TIM_CKD_DIV1; //分频因子
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	
	
	//3、启定时器中断，配置NVIC。
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;			//中断通道，代码头文件STM32F4xx.h中typedef enum IRQn枚举中可查看到中断的通道编号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 	//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 	= 1;        //响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//使能中断通道
	NVIC_Init(&NVIC_InitStructure);

	//4、设置 TIM3_DIER  允许更新中断
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	
	//5、使能定时器。
	TIM_Cmd(TIM3, ENABLE);

}

//编写中断服务函数。
//1ms进入中断一次
void   TIM3_IRQHandler(void)
{
	//判断标志位是否置1
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		PFout(9) = !PFout(9);
		
		//清空中断标志位
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}

}