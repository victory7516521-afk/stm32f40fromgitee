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

	uint8_t buf[32]={0};
	
	uint16_t tp_x,tp_y;
	
	uint8_t tp_finger_num=0;
	
	uint8_t tp_sta;
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
	//触摸屏初始化
	tp_init();


	Tim3_init(10499,63999);

	while(1)
	{

		
		/* 检测是否有触摸屏事件触发 */
		if(g_tp_event)
		{
			lcd_send_cmd(0x11);
			TIM_SetCounter(TIM3, 0);
			/* 持续访问 */
			while(1)
			{
				/* 检测是否有触摸屏事件触发 */
				if(g_tp_event == 0)
					continue;
				
				g_tp_event=0;
				
				/* 获取点击的坐标值 */
				if(tp_sta=tp_read(&tp_x,&tp_y))
				{
					/* 显示描点 */
					lcd_fill(tp_x,tp_y,5,5,RED);
					
					/* 清除上次的坐标值显示区域 */
					//lcd_fill(20,g_lcd_height-20,160,20,BLUE);
					
					/* 显示坐标值 */
					//tp_sta 输出的是手势码
					sprintf(buf,"x=%d y=%d sta=%02X",tp_x,tp_y,tp_sta);
					printf("%s\r\n",buf);
					//lcd_show_string(20,g_lcd_height-20,buf,WHITE,BLUE,16,0);
					
				}		
			
				if(tp_finger_num_get() == 0)
					break;				
			}
			
			delay_ms(2);
			
			/* 清屏 */
			lcd_fill(0,0,g_lcd_width,g_lcd_height,WHITE);
			
			/* 显示标题 */
			lcd_show_string(30,140,"TP Test By Teacher.Chen",RED,WHITE,16,0);
		}	
	}
	
	return 0;
}

