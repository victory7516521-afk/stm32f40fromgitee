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
	/******************************************************************************
	void lcd_show_string(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 font_size,u8 mode)
	*函数说明：显示字符串
	*形参：
	u16 x：显示字符起点坐标x
	u16 y：显示字符起点坐标y
	const u8 *p:字符串首地址
	u16 fc：字符的颜色
	u16 bc：字符串的背景颜色
	u8 font_size：字符大小 16 24 32 
	u8 mode：是否叠加模式(是否要底色) 取值0或者1

	*返回值：  无
	******************************************************************************/	
	
	lcd_show_string(20, 16, "HELLOWORLD", BLACK, RED,32, 0);
	while(1);




	while(1)
	{

		
		delay_s(1);
	}
	
	return 0;
}

