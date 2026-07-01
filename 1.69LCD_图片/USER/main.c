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
#include "rtc.h"
#include  "adc.h"
#include "iic.h"
#include "spiflash.h"
#include "tft.h"
#include "bmp.h"
#include "touch.h"  


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


	u16 id;
	//中断优先级分组，且一个工程只能设置一次
	//抢占优先级范围:0~3  响应优先级:0~3
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Delay_init();


	Usart1_init(115200);
	/* lcd初始化 */
	lcd_init();
	
	/* 清屏为白色 */
	lcd_clear(WHITE);	
	
	/* 显示方向， 竖屏 */
	lcd_set_direction(2);
	
	
	tp_init();



	//printf("g_image_test addr:%p\r\n", &g_image_test);
	//printf("g_image_1_120x140 addr:%p\r\n", g_image_1_120x140);


	//lcd_draw_picture(20	, 30, 120, 140, (const uint8_t *)g_image_1_120x140);
	lcd_draw_picture(0	, 0, 240, 200, (const uint8_t *)g_image__240x200);
	

	while(1);


	while(1)
	{

//		lcd_draw_picture(0	, 0, 120, 120, (const uint8_t *)g_image_frame_001_120x120);
//		delay_us(10);
//		lcd_draw_picture(0	, 0, 120, 120, (const uint8_t *)g_image_frame_004_120x120);
//		delay_us(10);
//		lcd_draw_picture(0	, 0, 120, 120, (const uint8_t *)g_image_frame_007_120x120);
//		delay_us(10);
//		lcd_draw_picture(0	, 0, 120, 120, (const uint8_t *)g_image_frame_014_120x120);
//		delay_us(10);
	
	
	
	}
	
	return 0;
}


