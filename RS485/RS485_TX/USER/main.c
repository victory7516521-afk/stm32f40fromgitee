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
#include "rs485.h"


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

	

	Rs485_Init(9600);
	//发送端
	PGout(8) = 1;
	printf("test\n");
	
	while(1)
	{

		//判断按键是否按下
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
		{
			delay_ms(15);  //延时消抖
			//判断按键是否按下
			if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
			{
				while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET); //等待按键松开
				Rs485_Send("H6CL11:", 7);

			}		
		}
		
		
		if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2) == Bit_RESET)
		{
			delay_ms(15);  //延时消抖
			//判断按键是否按下
			if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2) == Bit_RESET)
			{
				//按键按下要处理的事件
				while(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2) == Bit_RESET); //等待按键松开
				Rs485_Send("H6CL10:", 7);

				
			}		
		}		

	}

	
	return 0;
}


