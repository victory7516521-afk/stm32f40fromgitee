#include "exti.h"


/************************************
引脚说明：

KEY0连接PA0
PA0---EXTI0
选择下降沿触发
*************************************/
void Exti_PA0_Init(void)
{
	
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	//使能GPIOA时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	//能使SYSCFG（系统配置控制器）时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	/* Configure PA0 pin as input floating */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;   //引脚
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;	//输入 
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;//浮空
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* Connect EXTI Line0 to PA0 pin */
	//连续中断线0到PA0
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	
	/* Configure EXTI Line0 */
	//配置外部中断/事件控制器
	EXTI_InitStructure.EXTI_Line 	= EXTI_Line0;   //中断0线  EXTI0--Line0
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;   //中断
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;     //中断控制使能
	EXTI_Init(&EXTI_InitStructure);
	
	/* Enable and set EXTI Line0 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel 					 = EXTI0_IRQn; //NVIC中断通道，可在stm32fxx.h头文件中查找
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 		 = 0x02;		  //响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd 				 = ENABLE; //通道使能
	NVIC_Init(&NVIC_InitStructure);


}

/************************************
中断服务函数
注意点：
a.中断函数格式: void  中断服务函数名(void)
b.判断一个函数是否为中断服务函数名，需要查看能否在startup_stm32f40_41xxx.s中
查找的到，能查到，表示它是中断服务函数，否则不是。
c.中断服务函数是不需要调用的，当满足中断条件后,CPU自动去执行的函数。
d.中断是不能执行过长时间
e.中断服务函数是没有输入及返回值

*************************************/


void  EXTI0_IRQHandler(void)
{
    //判断中断线状态，如果有中断，中断位标志为1
    if(EXTI_GetITStatus(EXTI_Line0) == SET)
    {
        GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
        
        //清空中断线标志位
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}