#include "pwm.h"


/*
引脚说明

LED0连接在PF9,低电平灯亮；高电平，灯灭
TIM14_CH1(TIM14 -- APB1 16位  84MHZ)

*/
void Pwm_PF9_Init(void)
{
	GPIO_InitTypeDef 			GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef	 	TIM_TimeBaseStructure;
	TIM_OCInitTypeDef			TIM_OCInitStructure;
	//使能TIM14时钟--改
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	
	//使能GPIOF时钟--改
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	

	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9; 		//引脚 -- 改
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;		//复用功能 
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP ;
	//引脚初始化--改
	GPIO_Init(GPIOF, &GPIO_InitStructure); 
	
	//引脚映射 -- 改  
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);


	
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period 	= (1000-1);  //在1MHZ下，计1000个数，用时1ms(PWM周期)--看情况改
	TIM_TimeBaseStructure.TIM_Prescaler = (84-1);    //84分频 84MHZ/84 = 1MHZ  --看情况改
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	 //分频因子
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数
	//定时器初始化--改
	TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);
	
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode 			= TIM_OCMode_PWM1;		//PWM1模式--看情况改
	TIM_OCInitStructure.TIM_OCPolarity 	= TIM_OCPolarity_Low;   //极性电平，这里选择低电平--看情况改
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//通道使能
	TIM_OCInitStructure.TIM_Pulse 		= 0;  //CCR  捕获/比较寄存器的初始化
	//通道1初始化--改
	//OC1--CH1
	TIM_OC1Init(TIM14, &TIM_OCInitStructure);
	//使能预装载寄存器--改
	//OC1--CH1
	TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable);
	
	//使能重装值寄存器--改
	TIM_ARRPreloadConfig(TIM14, ENABLE);
	
	//使能定时器--改
	TIM_Cmd(TIM14, ENABLE);
}