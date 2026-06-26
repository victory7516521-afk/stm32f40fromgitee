#include "delay.h"

u32 my_nus = 21; 	//在21MHZ下，计21个数，	用时1us
u32 my_nms = 21000; //在21MHZ下，计21000个数，用时1ms

void Delay_Init(void)
{
	//SysTick时钟频率:168MHZ/8 = 21MHZ
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

}

//u32 nus:nus范围1~798,915
void delay_us(u32 nus) 
{
	u32 temp = 0x00;
	
	//重载寄存器的值  210
	SysTick->LOAD = nus*my_nus - 1;
	
	//设置计数器为0
	SysTick->VAL = 0x00;
	//使能定时器
	SysTick->CTRL |= (0x01<<0);
	
	do{
		temp = SysTick->CTRL;
		
	//(temp & 0x01 )判断定时器是否开启	 判断第16位是否为1
	}while( (temp & 0x01 ) && (!(temp & (0x01<<16))));
	
	//关闭定时器
	SysTick->CTRL &= ~(0x01<<0);
}

//nms范围1~798
void delay_ms(u32 nms) 
{
	u32 temp = 0x00;
	
	//重载寄存器的值  
	SysTick->LOAD = nms*my_nms - 1;
	
	//设置计数器为0
	SysTick->VAL = 0x00;
	//使能定时器
	SysTick->CTRL |= (0x01<<0);
	
	do{
		temp = SysTick->CTRL;
		
	//(temp & 0x01 )判断定时器是否开启	 判断第16位是否为1
	}while( (temp & 0x01 ) && (!(temp & (0x01<<16))));
	
	//关闭定时器
	SysTick->CTRL &= ~(0x01<<0);
}

void delay_s(u32 ns)
{
	int i;
	
	for(i=0; i<ns; i++)
	{	
		delay_ms(500);
		delay_ms(500);
	}
}	