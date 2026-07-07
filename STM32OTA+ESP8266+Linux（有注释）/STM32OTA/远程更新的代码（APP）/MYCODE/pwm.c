#include "pwm.h"

/**********************************************
引脚说明：

LED0连接在PF9,低电平灯亮；高电平，灯灭
TIM14_CH1(TIM14 -- APB1 16位  84MHZ)

**********************************************/
void Pwm_PF9_Init(void)
{
	GPIO_InitTypeDef 		 GPIO_InitStruct;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef	 	 TIM_OCInitStruct;
	
	//使能定时器14时钟：
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	//使能GPIOF时钟：
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	

	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_9; 		//引脚9
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_AF;		//复用功能
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_PP;	//推挽
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz;	//50MHZ速度
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;		//上拉
	//初始化IO口为复用功能输出。
	GPIO_Init(GPIOF, &GPIO_InitStruct);	

	//GPIOF9复用映射到定时器14  引脚选定哪个复用功能
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource9,GPIO_AF_TIM14); 
	
	TIM_TimeBaseInitStruct.TIM_Prescaler		= 84-1;   				//84分频  84MHZ/84 = 1MHZ, 即1us计一个数
	TIM_TimeBaseInitStruct.TIM_Period			= 1000-1;   			//计1000个数，用时1ms 周期1ms
	TIM_TimeBaseInitStruct.TIM_CounterMode		= TIM_CounterMode_Up; 	//向上计数
	TIM_TimeBaseInitStruct.TIM_ClockDivision	= TIM_CKD_DIV1;			//分频因子
	//	2、初始化定时器，配置ARR,PSC。
	TIM_TimeBaseInit(TIM14, &TIM_TimeBaseInitStruct);
	
	
	
	TIM_OCInitStruct.TIM_OCMode 	 = TIM_OCMode_PWM1; 		//选择PWM的模式，选择PWM模式1
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; 	//用于设置输出的使能，使能PWM输出到端口
	TIM_OCInitStruct.TIM_OCPolarity  = TIM_OCPolarity_Low; 		//输出的极性，输出是高电平还是低电平，这里选择低(低电平灯亮)。
	//根据设定信息配置TIM14 OC1--通道1
	TIM_OC1Init(TIM14, &TIM_OCInitStruct); 

	//使能预装载寄存器： 
	TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable); 
	//使能自动重装载的预装载寄存器允许位	
	TIM_ARRPreloadConfig(TIM14,ENABLE);
	//使能定时器。
	TIM_Cmd(TIM14, ENABLE);
	

}