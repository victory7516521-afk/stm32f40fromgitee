#include <stdio.h>
#include "led.h"
#include "key.h"
#include "sys.h"
#include "exti.h"
#include "delay.h"
#include "tim.h"
#include "pwm.h"
#include "usart.h"
#include "string.h"
#include "gpio.h"
#define  P 10
volatile u8 g_data[10]={0};
volatile u8 g_flag=0;
volatile u8 g_i = 0;
void USART1_IRQHandler(void)
{
	
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		//清空标志位
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		//接收数据
		u8 ch = USART_ReceiveData(USART1);//一个一个字节收的
		//USART_SendData(USART1, ch);
		
		g_data[g_i]=ch;
		g_i++;
		
		if(ch == ':')
		{
	
			g_flag=1;
			g_i=0;
		}
		
		//发送数据
	}


}


int main(void)
{ 
	//一个项目中，只能有一次的中断优先级分组
	//中断优先级分组：第二组，抢占优先级范围:0~3 响应优先级范围:0~3 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	Led_Init();
	
	Delay_Init();
	gpio_Init();
	Tim3_Init();
	Exti_Init();
	Usart1_Init(115200);
	
	while(1)
	{
		
	
	}
	return 0;
}