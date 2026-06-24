#include "pwm.h"


/***********************************
引脚说明：

LED0连接在PF9,低电平灯亮；高电平，灯灭
TIM14_CH1(TIM14 -- APB1 16位  84MHZ)
CH1：通道1
************************************/

void Pwm_PF9_init(void)
{
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseInitStruct;
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM_OCInitTypeDef  TIM_OCInitStructure;

	
	//使能TIM14时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	
	//使能GPIOF时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	
	//配置为GPIOF9
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9;		//引脚9
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;		//复用功能
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;	//输出速度
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;	//推挽输出
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP ;	//上拉
	GPIO_Init(GPIOF, &GPIO_InitStructure); 
	
	//PF9复用功能选择TIM14
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14); 
	

	
	/* Time base configuration */
	TIM_TimeBaseInitStruct.TIM_Prescaler	= (84-1); 	//分频值84 定时器频率：84MHZ/84 = 1MHZ
	TIM_TimeBaseInitStruct.TIM_Period		= (1000-1);	//重载值周期这个周期等于PWM周期：1000/1MHZ = 1ms
	TIM_TimeBaseInitStruct.TIM_CounterMode	= TIM_CounterMode_Up; //向上计数
	TIM_TimeBaseInitStruct.TIM_ClockDivision= TIM_CKD_DIV1; //分频因子
	TIM_TimeBaseInit(TIM14, &TIM_TimeBaseInitStruct);
	
	/* PWM1 Mode configuration: Channel1 */
	//PWM1通道1 
	TIM_OCInitStructure.TIM_OCMode 		= TIM_OCMode_PWM1;  // PWM模式1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //通道输出使能
	TIM_OCInitStructure.TIM_Pulse 		= 0; //初始化时CCRx寄存器值 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性电平，PF9输出低电平，灯亮，所以这里选择极性电平为低电平
	//OC1--对应的是通道1
	TIM_OC1Init(TIM14, &TIM_OCInitStructure);
	
	//使能OC1重载值寄存器
	TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable);
	
	//使能定时器的重载值寄存器
	TIM_ARRPreloadConfig(TIM14, ENABLE);
	
	//使能定时14
	TIM_Cmd(TIM14, ENABLE);
	
}