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


u8 rx_flag = 0;  //rx_flag = 1表示接受数据完成
u8 data = 0;    

u8 buffer[32] = {0};
u8 rx_buffer[32] = {0};
u8 rx_i = 0;
u8 rx_count = 0;



void USART2_IRQHandler(void)
{
	
	
	//判断中断标志位是否置为1
	if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		//清除中断标志位
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		//接收数据
		buffer[rx_count++] = USART_ReceiveData(USART2);

		//判断是否接受到帧尾
		if(buffer[rx_count - 1] == ':')
		{
			//过滤帧尾，并将数据存储到rx_buffer当中
			for(rx_i = 0; rx_i < (rx_count - 1); rx_i++)
			{
				rx_buffer[rx_i] = buffer[rx_i];
			}
			
			memset(buffer,0, sizeof(buffer));
			
			rx_count = 0;  //下一帧数据从头开始
			rx_flag = 1;
		}
	
	}

}


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
	//接受端
	PGout(8) = 0;

	
	while(1)
	{

		if(rx_flag == 1)
		{
			if(strcmp(rx_buffer,"H6CL11") == 0)
			{
				LED0_ON;
			}
			
			if(strcmp(rx_buffer,"H6CL10") == 0)
			{
				LED0_OFF;
			}			
			
			memset(rx_buffer,0, sizeof(rx_buffer));
			
			rx_flag = 0;
		}	

	}
	
	return 0;
}


