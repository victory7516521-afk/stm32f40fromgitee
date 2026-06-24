#include "exti.h"
#include "sys.h"

/*********************
SET_VALUE值为1 抢占优先级不同，响应优先级相同
SET_VALUE值为2 抢占优先级相同，响应优先级不同
SET_VALUE值为3 抢占优先级相同，(同时发生)哪个子优先级高，哪个先执行



**********************/



#define SET_VALUE  3



/*
引脚说明

KEY0连接PA0
PA0---EXTI0
选择下降沿触发

KEY2连接PE2
PE2---KEY2
选择下降沿触发


*/
void Exti_Init(void)
{
	
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	//使能GPIOA组时钟--改
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	//使能GPIOE组时钟--改
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	
	//使能SYSCFG组时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	/* Configure PA0 pin as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  //输入模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//浮空
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;   //引脚0 -- 改
	GPIO_Init(GPIOA, &GPIO_InitStructure); //--改

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  //输入模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//浮空
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;   //引脚0 -- 改
	GPIO_Init(GPIOE, &GPIO_InitStructure); //--改



	/* Connect EXTI Line0 to PA0 pin */
	//PA0连接到EXTI0--改
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	//PE2连接到EXTI2--改
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);	
	
	
	/* Configure EXTI Line0 */
	EXTI_InitStructure.EXTI_Line 	= EXTI_Line0;			//外部中断线0 -- 改
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;  //中断
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //触发方式：下降沿 -- 看情况改
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;		//中断线使能
	EXTI_Init(&EXTI_InitStructure);
	

	
	EXTI_InitStructure.EXTI_Line 	= EXTI_Line2;			//外部中断线2 -- 改
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;  //中断
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //触发方式：下降沿 -- 看情况改
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;		//中断线使能
	EXTI_Init(&EXTI_InitStructure);	

#if  (SET_VALUE == 1)
	//NVIC配置
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;   //外部中断线0的通道编号，可在stm32f4xx.h中查看 -- 改
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;     //抢占优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;            //响应优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                    //通道使能
    NVIC_Init(&NVIC_InitStructure);
	
	//NVIC配置
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;   //外部中断线0的通道编号，可在stm32f4xx.h中查看 -- 改
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;     //抢占优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;            //响应优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                    //通道使能
    NVIC_Init(&NVIC_InitStructure);
	
#elif  (SET_VALUE == 2)
	//NVIC配置
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;   //外部中断线0的通道编号，可在stm32f4xx.h中查看 -- 改
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;     //抢占优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;            //响应优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                    //通道使能
    NVIC_Init(&NVIC_InitStructure);
	
	//NVIC配置
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;   //外部中断线0的通道编号，可在stm32f4xx.h中查看 -- 改
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;     //抢占优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;            //响应优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                    //通道使能
    NVIC_Init(&NVIC_InitStructure);

#elif  (SET_VALUE == 3)
	//NVIC配置
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;   //外部中断线0的通道编号，可在stm32f4xx.h中查看 -- 改
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;     //抢占优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;            //响应优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                    //通道使能
    NVIC_Init(&NVIC_InitStructure);
	
	//NVIC配置
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;   //外部中断线0的通道编号，可在stm32f4xx.h中查看 -- 改
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;     //抢占优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;            //响应优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                    //通道使能
    NVIC_Init(&NVIC_InitStructure);
	
#endif	
}

//在中断当中，使用粗延时足够了
void exti_delay(int n)
{
	int i, j;
	
	for(i=0; i<n; i++)
		for(j=0; j<30000; j++);

}


/***********************************

a.中断函数格式: void  中断服务函数名(void)
b.判断一个函数是否为中断服务函数名，需要查看能否在startup_stm32f40_41xxx.s中
查找的到，能查到，表示它是中断服务函数，否则不是。
c.中断服务函数是不需要调用的，当满足中断条件后,CPU自动去执行的函数。
d.中断是不能执行过长时间
e.中断服务函数是没有输入及返回值


************************************/

#if (SET_VALUE == 1)

//抢占优先级不同，响应优先级相同

void EXTI0_IRQHandler(void) //--改
{
    //判断中断线状态，如果有中断，中断位标志为1
    if(EXTI_GetITStatus(EXTI_Line0) == SET) //--改
    {
		exti_delay(10);//约10ms 避免多次触发
		if(PAin(0) == 0)
		{
			//延时6S，实际上是不允许，这里仅仅测试使用
			for(int i=0; i<6; i++)
			{
				PFout(9) = !PFout(9);
				exti_delay(1000); //1S
			}
			
		
		}
        //清空中断线
        EXTI_ClearITPendingBit(EXTI_Line0);//--改
    }
}

void EXTI2_IRQHandler(void) //--改
{
    //判断中断线状态，如果有中断，中断位标志为1
    if(EXTI_GetITStatus(EXTI_Line2) == SET) //--改
    {
		exti_delay(10);//约10ms 避免多次触发
		if(PEin(2) == 0)
		{
			//延时6S，实际上是不允许，这里仅仅测试使用
			for(int i=0; i<6; i++)
			{
				PEout(14) = !PEout(14);
				exti_delay(1000); //1S
			}			
		}
        //清空中断线
        EXTI_ClearITPendingBit(EXTI_Line2);//--改
    }
}




#elif  (SET_VALUE == 2)

//抢占优先级相同，响应不相同

void EXTI0_IRQHandler(void) //--改
{
    //判断中断线状态，如果有中断，中断位标志为1
    if(EXTI_GetITStatus(EXTI_Line0) == SET) //--改
    {
		exti_delay(10);//约10ms 避免多次触发
		if(PAin(0) == 0)
		{
			//延时6S，实际上是不允许，这里仅仅测试使用
			for(int i=0; i<6; i++)
			{
				PFout(9) = !PFout(9);
				exti_delay(1000); //1S
			}
			
		
		}
        //清空中断线
        EXTI_ClearITPendingBit(EXTI_Line0);//--改
    }
}

void EXTI2_IRQHandler(void) //--改
{
    //判断中断线状态，如果有中断，中断位标志为1
    if(EXTI_GetITStatus(EXTI_Line2) == SET) //--改
    {
		exti_delay(10);//约10ms 避免多次触发
		if(PEin(2) == 0)
		{
			//延时6S，实际上是不允许，这里仅仅测试使用
			for(int i=0; i<6; i++)
			{
				PEout(14) = !PEout(14);
				exti_delay(1000); //1S
			}			
		}
        //清空中断线
        EXTI_ClearITPendingBit(EXTI_Line2);//--改
    }
}


#elif  (SET_VALUE == 3)

//抢占优先级相同，响应不相同

void EXTI0_IRQHandler(void) //--改
{
    //判断中断线状态，如果有中断，中断位标志为1
    if(EXTI_GetITStatus(EXTI_Line0) == SET) //--改
    {
		exti_delay(10);//约10ms 避免多次触发
		if(PAin(0) == 0)
		{
			//延时6S，实际上是不允许，这里仅仅测试使用
			for(int i=0; i<6; i++)
			{
				PFout(9) = !PFout(9);
				exti_delay(1000); //1S
			}
			
		
		}
        //清空中断线
        EXTI_ClearITPendingBit(EXTI_Line0);//--改
    }
}

void EXTI2_IRQHandler(void) //--改
{
    //判断中断线状态，如果有中断，中断位标志为1
    if(EXTI_GetITStatus(EXTI_Line2) == SET) //--改
    {
		exti_delay(10);//约10ms 避免多次触发
		if(PEin(2) == 0)
		{
			//延时6S，实际上是不允许，这里仅仅测试使用
			for(int i=0; i<6; i++)
			{
				PEout(14) = !PEout(14);
				exti_delay(1000); //1S
			}			
		}
        //清空中断线
        EXTI_ClearITPendingBit(EXTI_Line2);//--改
    }
}



#endif
