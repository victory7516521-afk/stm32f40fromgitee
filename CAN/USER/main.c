#include "stm32f4xx.h" //这个头文件包含库所有头文件
#include "led.h"
#include "key.h"
#include "exti.h"
#include "delay.h"
#include "tim.h"
#include "pwm.h"
#include "usart.h"
#include "hsr04.h"
#include "dht11.h"
#include "iwdg.h"
#include "rtc.h"
#include "stdio.h"
#include "string.h"
#include "adc.h"
#include "iic.h"
#include "i2c_ee.h"
#include "dma.h"
#include "can.h"


#define SEND_BUF_SIZE 64

//内存--IRAM(内部的运行内存)
u8 SendBuff[SEND_BUF_SIZE] = "USART DMA Example: Communication between two USART using DMA\r\n";	//发送数据缓冲区




int main(void)
{	
	u8 key = 0;
	u8 write_buff[6] = "hello";
	u8 i, cnt = 0, canbuf[8] = {0};
	
	//中断优先级只能配置一次
	//中断优先级配置 第2组，抢占优先级范围:0~3 响应优先级范围:0~3
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	Delay_Init();
	Led_Init();
	Key_Init();
	Usart1_Init(115200);

	
	printf("test\r\n");

	CAN1_Mode_Init(CAN_SJW_1tq,CAN_BS2_6tq,CAN_BS1_7tq,6,0);	//CAN普通模式初始化,普通模式,波特率500Kbps
	
	delay_ms(20);
	
	
	while(1)
	{

		if(PEin(2) == 0)
		{
			delay_ms(10);

			if(PEin(2) == 0)
			{
				while(PEin(2) == 0);
				for(i=0;i<8;i++)
				{
					canbuf[i]=cnt+i;//填充发送缓冲区
					
				}
				cnt++;
				
				int res=CAN1_Send_Msg(canbuf,8);//发送8个字节
				if(res)
					printf("send fail\r\n");
				else
					printf("OK\r\n");
			}		
		}
		key=CAN1_Receive_Msg(canbuf);
		if(key)//接收到有数据
		{	
				
			printf("canbuf:%s  ID:0x%lx\r\n", canbuf,Num.StdId);
		}
		
		
		delay_ms(10);

	}

	
	return 0;
}


