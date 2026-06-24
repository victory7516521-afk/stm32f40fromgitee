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
		
		
	
		TIM_Cmd(TIM3, ENABLE);//使能时钟
		
		while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)==1);
			
		u16 num= 0;
	
		num=(TIM3->CNT)/58;
		
		TIM_Cmd(TIM3,DISABLE);//关闭时钟
		if(num>0&&num<25)
		{	
			TIM_SetCompare1(TIM13,100);
			PFout(9) = 0;
			PFout(10) = 0;
			PEout(13) = 0;
			PEout(14) = 0;
		}else if(num>=25&&num<50)
		{
			TIM_SetCompare1(TIM13,400);
			PFout(9) = 0;
			PFout(10) = 0;
			PEout(13) = 0;
			PEout(14) = 1;
		
		}else if(num>=50&&num<75)
		{	
			TIM_SetCompare1(TIM13,700);
			PFout(9) = 0;
			PFout(10) = 0;
			PEout(13) = 1;
			PEout(14) = 1;
		
		}else if(num >= 75&&num<100)
		{
			TIM_SetCompare1(TIM13,800);
			PFout(9) = 0;
			PFout(10) = 1;
			PEout(13) = 1;
			PEout(14) = 1;
		}else
		{
			TIM_SetCompare1(TIM13,999);
			PFout(9) = 1;
			PFout(10) = 1;
			PEout(13) = 1;
			PEout(14) = 1;
		
		}
		sprintf(buffer, "CNT=%d, Distance=%d cm\r\n", TIM3->CNT, num);
		
		UART_Print(buffer);
		
		delay_ms(500);
}