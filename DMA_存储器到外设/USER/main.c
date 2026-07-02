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
#include "iic.h"
#include "i2c_ee.h"
#include "spiflash.h"
#include "flash.h"
#include "dma.h"


#define SEND_BUF_SIZE 64

//内存--IRAM(内部的运行内存)
u8 SendBuff[SEND_BUF_SIZE] = "USART DMA Example: Communication between two USART using DMA\r\n";	//发送数据缓冲区




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

	u16 id;
	//一个项目中，只能有一次的中断优先级分组
	//中断优先级分组：第二组，抢占优先级范围:0~3 响应优先级范围:0~3 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	
	Delay_Init();
	
	//配置串口
	Usart1_Init(115200);
	Led_Init();




	MYDMA_Config(DMA2_Stream7,DMA_Channel_4,(u32)&USART1->DR,(u32)SendBuff,strlen(SendBuff));
	
	//使能串口DMA发送,  串口与DMA联动，启动传输
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	
	

    while(1)
    {

		
		//等待传输完成再启动下一次传输
		if(g_dma_send_flag == 1) //可以再次启动DMA
		{
			//重新设置基地址
			DMA_Cmd(DMA2_Stream7, ENABLE);	/* 重新使能 DMA_STREAM */
			//改变缓冲区后，再发送数据
			SendBuff[0] = SendBuff[0]+1; //每次传输SendBuff[0]有变化
			//使能串口DMA发送,  串口与DMA联动，启动传输
			USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
			
		
			//传输已启动
			g_dma_send_flag = 0;
		}
		
		
		
	 }
	

    return 0;
}

