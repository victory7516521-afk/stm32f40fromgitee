#include "test.h"
#include "tim.h"
#include "gpio.h"
#include "led.h"
#include "delay.h"


//串口重定向
void UART_Print(char *str)
{
    while(*str)
    {
        USART_SendData(USART1, *str++);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
}


void TEST_Init(void)
{
		char buffer[50];
		
		GPIO_SetBits(GPIOA,GPIO_Pin_2);
		
		delay_us(20);//输出高电平20us;
		
		GPIO_ResetBits(GPIOA,GPIO_Pin_2);
		
		TIM3->CNT=0;//重装寄存器为0
		
		while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)==0);
		
		PFout(9) = 0;
	
		TIM_Cmd(TIM3, ENABLE);//使能时钟
		
		while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)==1);
			
		u16 num= 0;
	
		num=(TIM3->CNT)/58;
		
		TIM_Cmd(TIM3,DISABLE);//关闭时钟
		
		sprintf(buffer, "CNT=%d, Distance=%d cm\r\n", TIM3->CNT, num);
		
		UART_Print(buffer);
		
		delay_s(1);
}