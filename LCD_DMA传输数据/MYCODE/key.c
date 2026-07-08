#include "key.h"

/*
PA0 key0 
PE2 key1
PE3 key2
PE4 key3
*/


void Key_init(void)
{
	// 开启GPIOA端口时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	// 开启GPIOE端口时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	
	// 配置GPIOA 0引脚为输入模式
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin	=	GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode	=	GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd	=	GPIO_PuPd_UP;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	
	// 配置GPIOE 2 3 4引脚为输入模式
	GPIO_InitStruct.GPIO_Pin	=	GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode	=	GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd	=	GPIO_PuPd_UP;
	GPIO_Init(GPIOE,&GPIO_InitStruct);
}


//粗延时,表示不准确
void delays(int n)
{
	int i, j;
	
	for(i=0; i<n; i++)
		for(j=0; j<35000; j++);
}


// mode == 1支持连按 mode == 0不支持连按
u8 key_scanf(u8 mode)
{
	static u8 flag = 1; //静态变量
	
	if(mode == 1)
		flag = 1;
	
	// 按键按下
	if(flag == 1 && (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0 
					|| GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2) == 0
					|| GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3) == 0
					|| GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4) == 0))
	{
		delays(10);
		flag = 0; // 清空标志位
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0)
		{
			return KEY0_VALUE;
		}
		else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2) == 0)
		{
			return KEY1_VALUE;
		}
		else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3) == 0)
		{
			return KEY2_VALUE;
		}
		else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4) == 0)
		{
			return KEY3_VALUE;
		}
	}
	else if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1 
			&& GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2) == 1
			&& GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3) == 1
			&& GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4) == 1) // 按键松开
	{
		flag = 1;
	}
	return 0;
}
