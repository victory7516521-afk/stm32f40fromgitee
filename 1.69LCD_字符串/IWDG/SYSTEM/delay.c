#include "delay.h"

u32 my_us = 21;   	//在21MHZ下，计21个数，用1us

u32 my_ms = 21000;   //在21MHZ下，计21000个数，用1ms



void Delay_init(void)
{
	//HCLK/8 = 168MHZ/8 = 21MHZ 
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

}

//u32 nus取值范围：0~798915
void delay_us(u32 nus)
{
	
	u32 temp = 0x00;
	
	//设置重载寄存器值
	SysTick->LOAD = my_us*nus-1; 
	
	//设置计数器值为0
	SysTick->VAL = 0x00;
	
	//启动定时器
	SysTick->CTRL |= (0x01<<0);
	
	//相当CPU在阻塞等待定时器计数完成。
	do
	{
		//获取控制和状态寄存器的值
		temp = SysTick->CTRL;
	//判断第0位是否为1(判断定时器是否开启)判断第16位是否为1，为1则退出循环
	}while( (temp & 0x01) && !(temp & (0x01<< 16)));
	
	//关闭定时器
	SysTick->CTRL &= ~(0x01<<0);	
	
}


//u32 nms取值范围：0~798
void delay_ms(u32 nms)
{
	
	u32 temp = 0x00;
	
	//设置重载寄存器值
	SysTick->LOAD = my_ms*nms-1; 
	
	//设置计数器值为0
	SysTick->VAL = 0x00;
	
	//启动定时器
	SysTick->CTRL |= (0x01<<0);
	
	//相当CPU在阻塞等待定时器计数完成。
	do
	{
		//获取控制和状态寄存器的值
		temp = SysTick->CTRL;
	//判断第0位是否为1(判断定时器是否开启)判断第16位是否为1，为1则退出循环
	}while( (temp & 0x01) && !(temp & (0x01<< 16)));
	
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

