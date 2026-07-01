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
//printf输出重新定向到串口输出
int fputc(int ch, FILE *f)
{     
    USART_SendData(USART1,ch);  //通过串口发送数据
    //等待数据发送完毕
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);     

	
    return ch;
}

/************************************
引脚说明：

PA9  ---- USART1_TX(发送端)
PA10  ---- USART1_RX(接收端)
*************************************/
void Usart1_init(int BaudRate)
{
	//串口时钟使能，GPIO 时钟使能。
	//结构体
	GPIO_InitTypeDef 	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStruct;
	NVIC_InitTypeDef   	NVIC_InitStructure;
	
	
	//使能GPIOA时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	//使能USART1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);


	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9|GPIO_Pin_10;//引脚9 10
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;		//复用功能
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;	//输出速度
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;	//推挽输出
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP ;	//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure); 	


	
	//设置引脚复用器映射：调用 GPIO_PinAFConfig 函数。
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1); 
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); 
	
	
	USART_InitStruct.USART_BaudRate	= BaudRate; 	//波特率
	USART_InitStruct.USART_Mode		= USART_Mode_Tx|USART_Mode_Rx; //配置为收发模式  全双工
	USART_InitStruct.USART_Parity	= USART_Parity_No; //无奇偶校验位
	USART_InitStruct.USART_StopBits	= USART_StopBits_1; //停止位
	USART_InitStruct.USART_WordLength = USART_WordLength_8b; //8位
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件控制流
	//串口参数初始化：设置波特率，字长，奇偶校验等参数。
	USART_Init(USART1, &USART_InitStruct);
	
	
	NVIC_InitStructure.NVIC_IRQChannel 			= USART1_IRQn;			//中断通道，代码头文件STM32F4xx.h中typedef enum IRQn枚举中可查看到中断的通道编号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 	//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 	= 0;        //响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//使能中断通道
	//开启中断并且初始化 NVIC，使能中断（如果需要开启串口中断才需要这个步骤）。
	NVIC_Init(&NVIC_InitStructure);
	
	//配置为接收中断（表示有数据过来，CPU要中断进行接收）
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);    
	//使能串口。
	USART_Cmd(USART1, ENABLE);


}