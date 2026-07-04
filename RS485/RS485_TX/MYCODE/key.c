#include "key.h"


/************************************
引脚说明：
KEY0连接PA0
KEY0按下，PA0为低电平
KEY0未按下，PA0为高电平

KEY1连接PE2
KEY2连接PE3
KEY3连接PE4

*************************************/
void Key_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStruct;
	
	//使能GPIOA组时钟//通用输出
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	//使能GPIOE组时钟//通用输出
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		
	
	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_0;	//引脚0
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_IN; //输入模式
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	

	
	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;	//引脚234
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_IN; //输入模式
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOE, &GPIO_InitStruct);
		

}


void Delay(int n)
{
	int i, j;
	for(i=0; i<n; i++)
		for(j=0; j<20000; j++);

}
//u8 == unsigned char
//u8 mode值为1：支持连按; 值为0：不支持连按   
u8 Key_scan(u8 mode)   
{
	static u8 flag = 1; // 0表示不支持连按    1支持连按标志
	
	if(mode)
		flag = 1;
	
	
	//是否支持连按，由下面代码是进入一次还是多次
	if( flag && (PAin(0) == 0 || PEin(2) == 0 ||(PEin(3) == 0) || (PEin(4) == 0)))
	{
		
		
		Delay(10);
		flag = 0;
		
		if(PAin(0) == 0)
		{	
			return KEY0_VALUE;
		}
		if(PEin(2) == 0)
		{
			return KEY1_VALUE;
		}
		if(PEin(3) == 0)
		{
			return KEY2_VALUE;
		}
		if(PEin(4) == 0)
		{
			return KEY3_VALUE;
		}		
		
	}
	//按键松开
	else if((PAin(0)) && (PEin(2)) &&(PEin(3)) && (PEin(4)))
	{
		flag = 1;
	
	}
	
	
	//返回0表示无按键按下
	return 0;

}
