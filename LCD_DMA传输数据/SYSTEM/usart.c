#include "usart.h"
#include "stdio.h"

#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
    int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
int _sys_exit(int x) 
{ 
    x = x; 
} 
//����_ttywrch()�Ա���ʹ�ð�����ģʽ
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}
//printf������¶��򵽴������
int fputc(int ch, FILE *f)
{     
    USART_SendData(USART1,ch);  //ͨ�����ڷ�������
    //�ȴ����ݷ������
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);     

	
    return ch;
}

/************************************
����˵����

PA9  ---- USART1_TX(���Ͷ�)
PA10  ---- USART1_RX(���ն�)
*************************************/
void Usart1_init(int BaudRate)
{
	//����ʱ��ʹ�ܣ�GPIO ʱ��ʹ�ܡ�
	//�ṹ��
	GPIO_InitTypeDef 	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStruct;
	NVIC_InitTypeDef   	NVIC_InitStructure;
	
	
	//ʹ��GPIOAʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	//ʹ��USART1ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);


	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9|GPIO_Pin_10;//����9 10
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;		//���ù���
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;	//����ٶ�
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;	//�������
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP ;	//����
	GPIO_Init(GPIOA, &GPIO_InitStructure); 	


	
	//�������Ÿ�����ӳ�䣺���� GPIO_PinAFConfig ������
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1); 
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); 
	
	
	USART_InitStruct.USART_BaudRate	= BaudRate; 	//������
	USART_InitStruct.USART_Mode		= USART_Mode_Tx|USART_Mode_Rx; //����Ϊ�շ�ģʽ  ȫ˫��
	USART_InitStruct.USART_Parity	= USART_Parity_No; //����żУ��λ
	USART_InitStruct.USART_StopBits	= USART_StopBits_1; //ֹͣλ
	USART_InitStruct.USART_WordLength = USART_WordLength_8b; //8λ
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ��������
	//���ڲ�����ʼ�������ò����ʣ��ֳ�����żУ��Ȳ�����
	USART_Init(USART1, &USART_InitStruct);
	
	
	NVIC_InitStructure.NVIC_IRQChannel 			= USART1_IRQn;			//�ж�ͨ��������ͷ�ļ�STM32F4xx.h��typedef enum IRQnö���пɲ鿴���жϵ�ͨ�����
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 	//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 	= 0;        //��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//ʹ���ж�ͨ��
	//�����жϲ��ҳ�ʼ�� NVIC��ʹ���жϣ������Ҫ���������жϲ���Ҫ������裩��
	NVIC_Init(&NVIC_InitStructure);
	
	//����Ϊ�����жϣ���ʾ�����ݹ�����CPUҪ�жϽ��н��գ�
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);    
	//ʹ�ܴ��ڡ�
	USART_Cmd(USART1, ENABLE);


}