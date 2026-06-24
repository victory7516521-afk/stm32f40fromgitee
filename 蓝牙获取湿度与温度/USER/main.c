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


//中断与应用程序变量加volatile，防优化
volatile u8  g_i, g_count = 0,g_rxflag = 0,g_num;
volatile u8 buffer[32] = {0},g_buffer[32]={0};


//接收数据属于后端
void USART3_IRQHandler(void)
{
	
	
	if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
	{
		//清空标志位
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		//接收数据 Auto:  Manual:
		buffer[g_count++] = USART_ReceiveData(USART3);
		//buffer[g_count-1]才是刚刚接受的字符
		if(buffer[g_count-1] == ':'||buffer[g_count-1] == '1') //判断到:表示数据传输结束
		{
			//将buffer数据存放在g_buffer
			for(g_i=0; g_i<g_count-1; g_i++)
			{
				g_buffer[g_i] = buffer[g_i]; //没有将:存放到g_buffer
			}
			
			memset(buffer, 0, sizeof(buffer));
			g_buffer[g_count-1] = '\0'; 
			//重新置0，重新从buffer[0]开始接受数据
			g_count = 0;
			
			g_rxflag = 1; //置1，表示1帧数据接受完毕
		}
		
		
	
		//去处理发送数据
       // printf("%c\r\n", g_buffer[g_count-1]);  
		
	}

}


int main(void)
{ 
	u8 data[5] = {0};
	int ret;
	char buffer[20]={0},uffer[20]={0};
	
	//一个项目中，只能有一次的中断优先级分组
	//中断优先级分组：第二组，抢占优先级范围:0~3 响应优先级范围:0~3 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	
	Delay_Init();
	Usart1_Init(115200);//usb
	Usart3_Init(9600);//蓝牙
	Led_Init();
	
	Tim3_Init();//看门狗定时器
	Dht11_Init();
	
	Exti_Init();

	Iwdg_Init();
	Tim2_Init();
	//打印这一句话，表示系统重启
	printf("iwdg test\r\n");
	
	u8 flag = 0;
	//处理逻辑--前端
	while(1)
	{
		
		
		if(g_rxflag==1)
		{
			printf("%s\n",g_buffer);
			if(strcmp(g_buffer,"Auto")==0||g_buffer[0]<='9')
			{		
					if(g_buffer[0]<='9')
					{
						u16 count = 0;
						int time_val = atoi((char*)g_buffer); 
						count =  time_val*10000;
						TIM_SetAutoreload(TIM2, count - 1); 
						
					}
					TIM_Cmd(TIM2, ENABLE);//自动发送 16位 最多到6秒。
				
				
			}else if (strcmp(g_buffer,"Manual")==0)
			{	
				TIM_Cmd(TIM2, DISABLE);
				flag=1;
			}
			memset(g_buffer,0,sizeof(g_buffer));
			g_rxflag=0;
		}
		//手动发送
		if(flag == 1)
		{	flag=0;
			ret = Dht11_Data(data);
			if(ret == 0)
			{		
				
				sprintf(buffer, "湿度：%d.%d\r\n", data[0], data[1]);
				sprintf(uffer, "温度：%d.%d\r\n", data[2], data[3]);
				UART_Print(buffer);
				UART_Print(uffer);
				
			}
		}
		delay_s(2);
		
		
	}
    return 0;
}
//printf("湿度:%d.%d ", data[0], data[1]);
//printf("温度:%d.%d\r\n", data[2], data[3]);
