#include "tim.h"



/*********************************
定时器说明

TIM3--APB1
频率：84MHZ
16位定时器

*********************************/
void Tim3_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeStructure;
	
	/* TIM3 clock enable */
	//使能定时器3 -- 改
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	


	
	/* Time base configuration */
	TIM_TimeStructure.TIM_Prescaler 	= (8400-1); 	//84分频 84MHZ/84 = 1MHZ  -- 看情况改
	TIM_TimeStructure.TIM_Period 		= (9000-1); //重载值为1000，在1MHZ，计1000个数，用时1ms -- 看情况改
	TIM_TimeStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数
	TIM_TimeStructure.TIM_ClockDivision = 0; //分频因子
	//--改
	TIM_TimeBaseInit(TIM3, &TIM_TimeStructure);
	

	/* Enable the TIM3 gloabal Interrupt */
	//配置定时器NVIC
	NVIC_InitStructure.NVIC_IRQChannel 					 = TIM3_IRQn; //改
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//-- 看情况改
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;	//-- 看情况改
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	
	/* TIM Interrupts enable */
	//配置为更新中断 -- 改
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	
	/* TIM3 enable counter */
	//使能定时器  开启定时器 -- 改
	TIM_Cmd(TIM3, ENABLE);

}

//1ms进入中断一次
void TIM3_IRQHandler(void)
{
    //判断更新中断标志是否置1
    if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
       //喂狗程序
		IWDG_ReloadCounter();
        
        //清空中断标志位
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }

}