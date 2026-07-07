#include "delay.h"

//u32 == unsigned int
u32 my_us = 21;				//计21个数用1us
u32 my_ms = 21000;			//计21000个数用1ms

void Delay_Init(void)
{
	//SysTick时钟配置 168MHZ/8 = 21MHZ
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

}

//nus取值范围：1~798 915
void delay_us(u32 nus)
{
	u32 temp = 0;

	//设置重装值寄存器
	SysTick->LOAD = my_us*nus - 1;
	//设置计数器的值为0
	SysTick->VAL = 0x00;
	//开启定时器
	SysTick->CTRL |= (0x01<<0);
	
	do
	{
		temp = SysTick->CTRL;
			//判断定时是否关闭	//判断SysTick->CTRL第16位是否为1
	}while((temp&(0x01<<0)) && (!(temp & (0x01<<16))));
		
	//关闭定时器
	SysTick->CTRL &= ~(0x01<<0);
}

//nms取值范围：1~798
void delay_ms(u32 nms)
{
	u32 temp = 0;

	//设置重装值寄存器
	SysTick->LOAD = my_ms*nms - 1;
	//设置计数器的值为0
	SysTick->VAL = 0x00;
	//开启定时器
	SysTick->CTRL |= (0x01<<0);
	
	do
	{
		temp = SysTick->CTRL;
			//判断定时是否关闭	//判断SysTick->CTRL第16位是否为1
	}while((temp&(0x01<<0)) && (!(temp & (0x01<<16))));
		
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

