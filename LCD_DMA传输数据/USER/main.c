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
#include "dma.h"

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
	u8 led_state = 0;
	u32 counter = 0;
	
	//中断优先级分组，且一个工程只能设置一次
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	// 初始化延时函数
	Delay_init();
	
	// 初始化串口（波特率115200）
	Usart1_init(115200);
	
	// 延时一小段时间，确保串口稳定
	delay_ms(100);
	
	// 打印启动信息（这是第一个输出，用来确认程序是否正常运行）
	printf("=== 系统启动 ===\r\n");
	
	// 初始化LED
	Led_Init();
	
	// LED闪烁一次，确认LED正常
	GPIO_ResetBits(GPIOF, GPIO_Pin_9); // PF9灯亮
	printf("LED亮\r\n");
	delay_ms(500);
	GPIO_SetBits(GPIOF, GPIO_Pin_9);   // PF9灯灭
	printf("LED灭\r\n");
	delay_ms(500);
	
	// 初始化LCD
	printf("开始初始化LCD...\r\n");
	lcd_init();
	printf("LCD初始化完成\r\n");
	
	// 清屏为白色
	lcd_clear(WHITE);
	printf("LCD清屏完成\r\n");
	
	// 设置显示方向为竖屏
	lcd_set_direction(2);
	printf("显示方向设置完成\r\n");
	
	// 初始化触摸屏
	printf("开始初始化触摸屏...\r\n");
	tp_init();
	printf("触摸屏初始化完成\r\n");
	
	// 使用标准方式显示图片
	printf("开始显示图片...\r\n");
	lcd_draw_picture_ex(0, 0, 240, 200, (const uint8_t *)g_image__240x200, WHITE);
	printf("图片显示完成！\r\n");
	
	// 主循环：闪烁LED
	while(1)
	{
		if(counter % 500000 == 0)
		{
			led_state = !led_state;
			if(led_state)
				GPIO_ResetBits(GPIOF, GPIO_Pin_9); // PF9灯亮
			else
				GPIO_SetBits(GPIOF, GPIO_Pin_9);   // PF9灯灭
		}
		counter++;
	}
	
	return 0;
}


