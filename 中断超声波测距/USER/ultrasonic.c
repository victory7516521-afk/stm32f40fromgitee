#include "ultrasonic.h"
#include "delay.h"
#include <stdio.h>
#include "stm32f4xx.h"
#include "stm32f4xx_exti.h"

u8 PA3_FLAG = 0;

void SR04_Init()
{
	GPIO_InitTypeDef GPIO_InitStruct;                           //GPIO结构体
    TIM_TimeBaseInitTypeDef	TIM_TimeBaseInitStruct;             //定时器结构体
    EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);           //使能GPIOF时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);            //使能TIM3时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);          //使能SYSCFG时钟
    
	GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_2;       //设置引脚
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;    //设置为通用输出模式
	GPIO_InitStruct.GPIO_OType  = GPIO_OType_PP;    //设置为推挽输出
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_25MHz; //设置为25MHz
	GPIO_InitStruct.GPIO_PuPd 	= GPIO_PuPd_DOWN;   //设置为下拉电阻
	GPIO_Init(GPIOA,&GPIO_InitStruct);              //初始化GPIOA
    
    GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_3;       //设置引脚
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_IN;     //设置为通用输入模式
    GPIO_Init(GPIOA,&GPIO_InitStruct);              //初始化GPIOA
    
	TIM_TimeBaseInitStruct.TIM_Prescaler	= 84-1; 		        //频率1MHZ
	TIM_TimeBaseInitStruct.TIM_Period		= 65535-1;	            //重装载值
	TIM_TimeBaseInitStruct.TIM_CounterMode	= TIM_CounterMode_Up;   //向上计数
	TIM_TimeBaseInitStruct.TIM_ClockDivision= TIM_CKD_DIV1;         //分频因子
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);                //初始化定时器TIM3
	
	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);                     //禁用TIM3定时器更新中断
    TIM_Cmd(TIM3, DISABLE);                                         //关闭定时器
    
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource3);   //GPIOA 的 Pin3 引脚映射到 EXTI3 外部中断线

	EXTI_InitStructure.EXTI_Line 	= EXTI_Line3;			//中断线3
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;  //中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //上升下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				//使能中断线
	EXTI_Init(&EXTI_InitStructure);                         //初始化中断线
	
	/* Enable and set EXTI Line0 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;            //中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;   //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 	= 1;        //响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             //使能中断通道
	NVIC_Init(&NVIC_InitStructure);                             //初始化中通道    
}

void EXTI3_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line3) == SET)
	{
        if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == 1) //上升沿
        {
            TIM3->CNT = 0;
            TIM_Cmd(TIM3, ENABLE);
            PA3_FLAG = 1;
        }
		//下降沿
        if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3) == 0 && PA3_FLAG == 1)
        {
            PA3_FLAG = 0;
            int temp = 0;
            int distance = 0;
            temp = TIM3->CNT;
            TIM_Cmd(TIM3, DISABLE);
            distance = temp/58;
            if(distance >=2 && distance <=400)
            {
                printf("距离为:%dcm\r\n", distance);
            }
            else
            {
                 printf("无效距离\n");
            }
            
        }
		EXTI_ClearITPendingBit(EXTI_Line3);
	}
}

void SR04_RAND()
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_2);
    delay_us(8);
    GPIO_SetBits(GPIOA,GPIO_Pin_2);
    delay_us(20);
    GPIO_ResetBits(GPIOA,GPIO_Pin_2);
}

