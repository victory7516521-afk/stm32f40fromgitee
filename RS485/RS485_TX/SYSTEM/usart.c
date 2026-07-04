#include "usart.h"


volatile u8 g_flag = 0; //
volatile u8 g_data = 0; //


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
		//串口发送数据函数	
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
void Usart1_Init(int BaudRate)
{
	USART_InitTypeDef 	USART_InitStructure;
	NVIC_InitTypeDef 	NVIC_InitStructure;
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	/* Enable GPIO clock */
	//GPIOA时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	/* Enable USART clock */
	//串口时钟初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	/* Connect USART pins to AF7 */
	//引脚复用映射
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9,  GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	
	/* Configure USART Tx and Rx as alternate function push-pull */
	
	GPIO_InitStructure.GPIO_Pin   	= GPIO_Pin_9|GPIO_Pin_10; //引脚9 10
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;		//复用模式
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;	//速度
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;	//推挽
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;		//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	

	USART_InitStructure.USART_BaudRate 		= BaudRate;	//波特率
	USART_InitStructure.USART_WordLength 	= USART_WordLength_8b; //字节长度 1字节 
	USART_InitStructure.USART_StopBits 		= USART_StopBits_1;		//1位停止位
	USART_InitStructure.USART_Parity 		= USART_Parity_No;  	//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件控制流
	USART_InitStructure.USART_Mode 			= USART_Mode_Rx | USART_Mode_Tx; //收发
	USART_Init(USART1, &USART_InitStructure);
	
	
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;			//中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 	//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//通道使能
	NVIC_Init(&NVIC_InitStructure);
	
	//配置为接收中断（表示有数据过来，CPU要中断进行接收）
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);    
	
	/* Enable USART */
	USART_Cmd(USART1, ENABLE);


}

void USART1_IRQHandler(void)
{
	
	//判断标志位是否置位
	if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		 //收数据
		 g_data = USART_ReceiveData(USART1);
		

		 g_flag = 1;
		
		//清空标志位
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}

}


