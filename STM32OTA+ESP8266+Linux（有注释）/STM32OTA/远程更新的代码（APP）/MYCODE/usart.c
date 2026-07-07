#include "usart.h"
#include "stdio.h"

#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
int _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数   printf 是一个宏
int fputc(int ch, FILE *f)
{ 	
	USART_SendData(USART1,ch);  //通过串口发送数据
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);      
	return ch;
}

/**********************************************
引脚说明：
Uasrt1_TX连接PA9
Uasrt1_RX连接PA10
**********************************************/
void Usart1_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;	
	NVIC_InitTypeDef  NVIC_InitStruct;
	
	
	
	//使能 USART1 时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	//使用的是串口 1，串口 1 对应着芯片引脚 PA9,PA10 需要使能PA的时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 

	//引脚复用器映射配置，需要配置PA9，PA10 的引脚，调用函数为：
	//PA9 复用为 USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); 
	//PA10 复用为 USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);

	GPIO_InitStruct.GPIO_Pin 	= GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9 与 GPIOA10
	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_AF;	//配置IO口复用功能
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_50MHz; //速度 50MHz
	GPIO_InitStruct.GPIO_OType 	= GPIO_OType_PP; //推挽复用输出
	GPIO_InitStruct.GPIO_PuPd 	= GPIO_PuPd_UP; 	//上拉
	//初始化 PA9，PA10
	GPIO_Init(GPIOA,&GPIO_InitStruct); 


	
	USART_InitStruct.USART_BaudRate 			= 115200;							//一般设置为 115200;
	USART_InitStruct.USART_WordLength 			= USART_WordLength_8b;				//字长为 8 位数据格式
	USART_InitStruct.USART_StopBits 			= USART_StopBits_1;					//一个停止位
	USART_InitStruct.USART_Parity 				= USART_Parity_No;					//无奇偶校验位
	USART_InitStruct.USART_HardwareFlowControl 	= USART_HardwareFlowControl_None;	//无硬件控制流
	USART_InitStruct.USART_Mode 				= USART_Mode_Rx | USART_Mode_Tx;	//收发模式  全双工
	//初始化串口
	USART_Init(USART1, &USART_InitStruct); 


	NVIC_InitStruct.NVIC_IRQChannel						= USART1_IRQn;		//中断通道 ，在stm32f4xx.h
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	= 2;				//抢占优先级
 	NVIC_InitStruct.NVIC_IRQChannelSubPriority			= 2;				//响应优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd					= ENABLE;			//中断通道使能 
	//配置中断分组（NVIC），并使能中断。
    NVIC_Init(&NVIC_InitStruct);

	
	//配置为接受中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	//	5、使能串口。
	USART_Cmd(USART1, ENABLE);
	
}