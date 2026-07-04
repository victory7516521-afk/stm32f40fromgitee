#include "delay.h"


//在21MHZ，1us计21个数
u32 my_us = 21;
//在21MHZ，1ms计21000个数
u32 my_ms = 21000;

void Delay_Init(void)
{
	//配置Systick定时器时钟源：168MHZ/8 = 21MHZ
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

}

//nus取值范围：1~798915
void delay_us(u32 nus) 
{
	u32 temp;
	//设置定时器重载值寄存器
	SysTick->LOAD = my_us*nus-1;
	//设置计数器值为0
	SysTick->VAL = 0x00;
	
	
	//做延时是不需要产生异常
	
	//启动定时器
	SysTick->CTRL |= 0x01;
	
	do
	{
		temp = SysTick->CTRL;
		
		
	//temp & 0x01判断定时器是否使能	
	//!(temp  & (0x01<<16))判断计数器是否计数到0
	}while( (temp & 0x01) && !(temp  & (0x01<<16)));
	
	//关闭定时器
	SysTick->CTRL &= ~0x01;

}
//nms取值范围：1~798
void delay_ms(u32 nms)
{
	u32 temp;
	//设置定时器重载值寄存器
	SysTick->LOAD = my_ms*nms-1;
	//设置计数器值为0
	SysTick->VAL = 0x00;
	
	
	//做延时是不需要产生异常
	
	//启动定时器
	SysTick->CTRL |= 0x01;
	
	do
	{
		temp = SysTick->CTRL;
		
		
	//temp & 0x01判断定时器是否使能	
	//!(temp  & (0x01<<16))判断计数器是否计数到0
	}while( (temp & 0x01) && !(temp  & (0x01<<16)));
	
	//关闭定时器
	SysTick->CTRL &= ~0x01;

}


void delay_s(u32 ns)
{
	for(int i=0; i<ns; i++)
	{
		delay_ms(500);
		delay_ms(500);
	}

}
