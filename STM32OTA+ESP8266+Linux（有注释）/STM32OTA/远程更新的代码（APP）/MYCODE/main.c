#include "stm32f4xx.h"
#include "led.h"
#include "key.h"
#include "exti.h"
#include "delay.h"
#include "tim.h"
#include "pwm.h"
#include "usart.h"
#include "string.h"
#include "sys.h"
#include "dht11.h"

u8 buffer[32] = {0};
u8 rx_buffer[32] = {0};
u8 count = 0, rx_i;
u8 rx_flag = 0;  //rx_flag = 1说明接受到数据

//USART中断接收
void USART1_IRQHandler(void)
{
	
   //若是非空，则返回值为1，与RESET（0）判断，不相等则判断为真
   if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
   {	
		//判断为真后，为下次中断做准备，则需要对中断的标志清零
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
		
	   /* DR读取接受到的数据*/
		buffer[count++] = USART_ReceiveData(USART1);	
	   
	   if(buffer[count-1] == ':')
	   {
	   
		   //过滤帧尾
		   for(rx_i=0; rx_i < (count-1); rx_i++)
		   {
				rx_buffer[rx_i] = buffer[rx_i];
		   }
		   
		   count = 0;
		   //清空数组
		   memset(buffer, 0, sizeof(buffer));
		   
			rx_flag = 1;//标志一帧数据接受完毕
	   }
	 
   }
   
}



int main(void)
{
	int ret;
	u8 data[5] = {0};
	
	//NVIC中断分组:第二组， 抢占优先级取值范围：0x00~0x03; 响应优先级取值范围：0x00~0x03; 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	Delay_Init();
	Led_Init();
	Usart1_Init();
	Dht11_Init();


	while(1)
	{
		ret = Dht11_Start();
			
		if(ret == 0)
		{
			ret = Dht11_Read(data);
			if(ret == 0)
			{
				printf("温度：%d.%d, 湿度：%d,%d\n",data[2], data[3], data[0], data[1]);
			}
			
		}
		
		
		GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
		
		delay_s(2);
	
	
	}
	return 0;
}
