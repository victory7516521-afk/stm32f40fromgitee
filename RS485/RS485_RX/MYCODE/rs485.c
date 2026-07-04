#include "rs485.h"

/*****************************************
引脚说明：

PA2 -- U2_TX
PA3 -- U2_RX

RS485_RE -- PG8
****************************************/

void Rs485_Init(int MYBaudRate)
{

	
	GPIO_InitTypeDef  GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef  NVIC_InitStruct;
	
	
	//使能 USART2 时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	//B.  使用的是串口 1，串口 1 对应着芯片引脚 PA2,PA3 需要使能PA的时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE); 
	//2)  设置引脚复用器映射
	//PA2 复用为 USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); 
	//PA3 复用为 USART3
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);

	
	
	GPIO_InitStruct.GPIO_Pin 	= GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2 与 GPIOA3
	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_AF;				//配置IO口复用功能
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_50MHz; 		//速度 50MHz
	GPIO_InitStruct.GPIO_OType 	= GPIO_OType_PP; 			//推挽复用输出
	GPIO_InitStruct.GPIO_PuPd 	= GPIO_PuPd_UP; 			//上拉
	//初始化 PA2 PA3
	GPIO_Init(GPIOA,&GPIO_InitStruct); 
	
	
	
	USART_InitStruct.USART_BaudRate 	= MYBaudRate;					//波特率 115200;
	USART_InitStruct.USART_WordLength 	= USART_WordLength_8b;		//字长为 8 位数据格式
	USART_InitStruct.USART_StopBits 	= USART_StopBits_1;			//一个停止位
	USART_InitStruct.USART_Parity 		= USART_Parity_No;			//无奇偶校验位
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件控制流
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	//初始化串口
	USART_Init(USART2, &USART_InitStruct); 
	
	
	NVIC_InitStruct.NVIC_IRQChannel 					= USART2_IRQn;  //中断通道
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority	=3;				//抢占优先级 3
	NVIC_InitStruct.NVIC_IRQChannelSubPriority 			=3;  			//响应优先级 3
	NVIC_InitStruct.NVIC_IRQChannelCmd 					= ENABLE; 		//IRQ 通道使能
	//根据指定的参数初始化 VIC 寄存器
	NVIC_Init(&NVIC_InitStruct);  
	
	//接收中断
	USART_ITConfig(USART2, USART_IT_RXNE ,ENABLE);	
	
	//中断使能
	USART_Cmd(USART2, ENABLE);	
	

	GPIO_InitStruct.GPIO_Pin 	= GPIO_Pin_8; 				//GPIOG8
	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_OUT;			//配置IO口输出模式
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_50MHz; 		//速度 50MHz
	GPIO_InitStruct.GPIO_OType 	= GPIO_OType_PP; 			//推挽复用输出
	GPIO_InitStruct.GPIO_PuPd 	= GPIO_PuPd_UP; 			//上拉
	GPIO_Init(GPIOG,&GPIO_InitStruct); 


}

void Rs485_Send(u8 *data, u16 len)
{
	u16 i;
	
	for(i=0; i<len; i++)
	{
		USART_SendData(USART2,*(data+i));  //通过串口发送数据
		while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);  
	
	
	}
	
	

	

}









