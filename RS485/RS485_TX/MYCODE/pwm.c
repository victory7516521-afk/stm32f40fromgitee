#include "pwm.h"

/************************************
引脚说明：
LED0连接在PF9,低电平灯亮；高电平，灯灭
TIM14_CH1(TIM14 -- APB1 16位  84MHZ)

*************************************/
void Pwm_PF9_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	
	//定时器14初始化
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	
	//使能GPIOF组时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;		//引脚9
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;	//复用
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP ; //上拉
	GPIO_Init(GPIOF, &GPIO_InitStructure); 
	
	//TIM14映射到引脚PF9
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);

	
	
	/* Time base configuration */
	//配置定时器
	TIM_TimeBaseStructure.TIM_Period 	= (1000-1);	//周期值：ARR的值
	TIM_TimeBaseStructure.TIM_Prescaler = (84-1); 	//分频器  84MHZ/84 = 1MHZ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //分频因子
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数
	
	TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);
	
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode 		= TIM_OCMode_PWM1;  //模式PWM1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //输出使能
	TIM_OCInitStructure.TIM_Pulse 		= 0;  //CCR寄存器值
	TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_Low; //输出极性，这里选择低电平
	//OC1对应是通道1（TIM14_CH1）
	TIM_OC1Init(TIM14, &TIM_OCInitStructure);
	//使能预装载寄存器
	TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable);
	
	//使能自动重装载的预装载寄存器允许位    
	TIM_ARRPreloadConfig(TIM14, ENABLE);
	
	/* TIM3 enable counter */
	TIM_Cmd(TIM14, ENABLE);

}