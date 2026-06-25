#include <stdio.h>
#include "led.h"
#include "key.h"
#include "sys.h"
#include "exti.h"
#include "delay.h"
#include "tim.h"
#include "pwm.h"
#include "usart.h"
#include "sys.h"
#include "string.h"
#include "sr04.h"
#include "dht11.h"
#include "iwdg.h"
#include "rtc.h"
#include "adc.h"

//中断与应用程序变量加volatile，防优化
volatile u8  g_i, g_count = 0,g_rxflag = 0;
volatile u8 buffer[32] = {0},g_buffer[32]={0};


//接收数据属于后端
void USART1_IRQHandler(void)
{
	
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		//清空标志位
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		//接收数据
		buffer[g_count++] = USART_ReceiveData(USART1);
		//buffer[g_count-1]才是刚刚接受的字符
		if(buffer[g_count-1] == ':') //判断到:表示数据传输结束
		{
			//将buffer数据存放在g_buffer
			for(g_i=0; g_i<g_count-1; g_i++)
			{
				g_buffer[g_i] = buffer[g_i]; //没有将:存放到g_buffer
			}
			
			
			memset(buffer, 0, sizeof(buffer));
			
			//重新置0，重新从buffer[0]开始接受数据
			g_count = 0;
			
			g_rxflag = 1; //置1，表示1帧数据接受完毕
		}
		
	
		//去处理发送数据
        printf("%c\r\n", g_buffer[g_count-1]);  
		
	}

}


int main(void)
{ 
	u16 value;
	float sum;
	//一个项目中，只能有一次的中断优先级分组
	//中断优先级分组：第二组，抢占优先级范围:0~3 响应优先级范围:0~3 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	Delay_Init();
	Usart1_Init(115200);
	Led_Init();

	Adc_PA4_Init();
	
	//处理逻辑--前端
	while(1)
	{
		value = Get_Adc_Value();
		sum = (3.3*value)/4095;
		printf("value:%d  电压：%.2f\r\n", value,sum);
		
		
		delay_s(1);
	}
    return 0;
}

