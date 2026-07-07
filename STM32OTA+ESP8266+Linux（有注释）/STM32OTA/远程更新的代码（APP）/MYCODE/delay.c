#include "delay.h"


u32 my_us = 21;			//1us可计21个数
u32 my_ms = 21000;		//1ms可计21000个数

void Delay_Init(void)
{
	//设置systick定时器的时钟频率 168/8 = 21MHZ
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

}

//延时微秒798,915  500000
void delay_us(u32 nus)
{
	u32 temp;
	
	//给重装值寄存器设置初始值
	SysTick->LOAD = my_us*nus - 1;
	//设置计数值为0
	SysTick->VAL = 0;
	//启动定时器
	SysTick->CTRL |= (0x01<<0);
	
	do
	{
		temp = SysTick->CTRL; //将寄存器值赋值给temp	
		   //判断定时器是否使能	 判断第十六位是否为1
	}while( (temp & (0x01<<0)) && !(temp&(0x01<<16)));
	
	//关闭定时器
	SysTick->CTRL &= ~(0x01<<0);
}

//延时毫秒798.915
void delay_ms(u32 nms)
{
	u32 temp;
	
	//给重装值寄存器设置初始值
	SysTick->LOAD = my_ms*nms - 1;
	//设置计数值为0
	SysTick->VAL = 0;
	//启动定时器
	SysTick->CTRL |= (0x01<<0);
	
	do
	{
		temp = SysTick->CTRL; //将寄存器值赋值给temp	
		   //判断定时器是否使能	 判断第十六位是否为1
	}while( (temp & (0x01<<0)) && !(temp&(0x01<<16)));
	
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

