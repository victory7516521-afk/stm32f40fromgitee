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
u8 RecvBuff[SEND_BUF_SIZE] = {0};	//发送数据缓冲区

/*
	DMA 外设到存储器 的工作流程：

	1. 串口收到数据后，DMA自动将数据从串口数据寄存器（外设）搬运到内存数组 RecvBuff
	2. 当接收满15个字节后，触发DMA传输完成中断
	3. 中断处理函数设置 g_dma_send_flag = 1 标志
	4. 主程序检测到标志后，处理数据（打印、写入Flash）
	5. 重新启动DMA，等待下一次数据接收


*/


// DMA接收完成标志（中断与应用程序变量加volatile，防优化）
volatile u8  g_dma_recv_flag = 0;


int main(void)
{ 
	u8 Rcv[20]={0};
	//一个项目中，只能有一次的中断优先级分组
	//中断优先级分组：第二组，抢占优先级范围:0~3 响应优先级范围:0~3 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	Delay_Init();
	
	//配置串口（波特率115200）
	Usart1_Init(115200);
	
	//初始化LED（用于调试指示）
	Led_Init();

	
	// 配置DMA通道（从串口接收数据到内存）
	// 参数：DMA流、通道、外设地址(USART1数据寄存器)、内存地址(RecvBuff)、数据数量(15字节)
	MYDMA_Config(DMA2_Stream2, DMA_Channel_4, (u32)&USART1->DR, (u32)RecvBuff, 15);
	
	// 使能串口的DMA接收请求（串口与DMA联动，启动接收）
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

    while(1)
    {
		// 等待DMA接收完成
		if(g_dma_send_flag == 1) 
		{
		
			printf("接收到数据:%s\r\n", RecvBuff);
			
			//  重新启动DMA接收（设置新的传输数量并启用）
			
			DMA_Cmd(DMA2_Stream2, ENABLE);	
			USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
			
			// 清除传输完成标志，等待下一次传输
			g_dma_send_flag = 0;
			
		
		}
	 }
	
    return 0;
}

