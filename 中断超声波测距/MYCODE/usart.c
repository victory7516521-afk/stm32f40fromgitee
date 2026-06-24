#include "usart.h"

/*
引脚说明
PA9  ---- USART1_TX(发送端)
PA10  ---- USART1_RX(接收端)

*/
void Usart1_Init(int myBaudRate)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	
	//串口时钟使能，GPIO 时钟使能。
	//--改
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	//3、GPIO 初始化设置：要设置模式为复用功能。
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9|GPIO_Pin_10; 		//引脚 -- 改
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;		//复用功能 
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP ;
	//引脚初始化--改
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
	//4、设置引脚复用器映射：调用 GPIO_PinAFConfig 函数。
	//引脚映射 -- 改  
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);		
	
	USART_InitStruct.USART_BaudRate	= myBaudRate;					//波特率
	USART_InitStruct.USART_Mode		= USART_Mode_Tx|USART_Mode_Rx;	//全双工
	USART_InitStruct.USART_Parity	= USART_Parity_No;				//无奇偶校验位
	USART_InitStruct.USART_StopBits	= USART_StopBits_1;				//1位停止位
	USART_InitStruct.USART_WordLength= USART_WordLength_8b;			//字长
	USART_InitStruct.USART_HardwareFlowControl= USART_HardwareFlowControl_None;//无硬件控制流
	//5、串口参数初始化：设置波特率，字长，奇偶校验等参数。
	USART_Init(USART1, &USART_InitStruct);
	
	//6、开启中断并且初始化 NVIC，使能中断（如果需要开启串口中断才需要这个步骤）。
	NVIC_InitStructure.NVIC_IRQChannel 					 = USART1_IRQn; //改
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//-- 看情况改
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;	//-- 看情况改
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	

	//7、配置为接收中断（表示有数据过来，CPU要中断进行接收）
	//--改
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);    
	//8、使能串口。
	USART_Cmd(USART1, ENABLE);
}

