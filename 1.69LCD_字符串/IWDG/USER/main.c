#include "stm32f4xx.h"
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
#include "dht11.h"
#include "iwdg.h"


volatile u8 g_rx_flag = 0;
volatile u8 g_count = 0;

volatile u8 g_buffer[32] = {0};  
volatile u8 g_rxuffer[32] = {0};


void USART1_IRQHandler(void)
{

	//判断标志位是否置1
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		//清空中断标志位
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);

		//串口1接收数据
		g_buffer[g_count++] = USART_ReceiveData(USART1);
		
		if(g_buffer[g_count - 1] == ':') //判断结束符是否为:
		{
			//过滤结束符   HCL11:   HCL10:
			for(int i=0; i<g_count - 1; i++)
			{
				g_rxuffer[i] = g_buffer[i];
			}
			
			g_rx_flag = 1; //接收数据标志位
			
			g_count = 0;//新的数据帧从g_buffer[0]开始接受
			
			
			memset(g_buffer, 0, sizeof(g_buffer));
		
		
		}
		       
	
	}

}




int main(void)
{
	int ret;
	u8 data[5];
	
	//中断优先级分组，且一个工程只能设置一次
	//抢占优先级范围:0~3  响应优先级:0~3
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	Delay_init();

	Key_init();
	Led_Init();

	Usart1_init(115200);
	
	Tim3_count_init(83, 20000);
	Dht11_Init();
	
	Iwdg_init();
	
	//如果打印这一句话，表示系统重启。
	printf("Iwdg_test\r\n");
	
	
	while(1)
	{
		ret = Dht11_Data(data);
		if(ret == 0)
		{
			printf("湿度：%d.%d%%\r\n", data[0], data[1]);
			printf("温度：%d.%d℃  \r\n", data[2], data[3]);	
		}

			
			
        delay_s(1);
        delay_ms(500);
        delay_ms(400);
        
        //保证上面程序运行的时间少于2S
        
        //应用程序喂狗:
        IWDG_ReloadCounter();
	}
	
	return 0;
}

