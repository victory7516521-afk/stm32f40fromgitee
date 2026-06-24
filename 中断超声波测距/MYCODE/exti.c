//#include "exti.h"

///************************************
//引脚说明：

//KEY0连接PA0
//PA0---EXTI0
//选择下降沿触发
//*************************************/

//void Exti_PA0_init(void)
//{
//	EXTI_InitTypeDef   EXTI_InitStructure;
//	GPIO_InitTypeDef   GPIO_InitStructure;
//	NVIC_InitTypeDef   NVIC_InitStructure;
//	
//	//使能GPIOA组时钟
//	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
//	//使能SYSCFG时钟
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
//	
//	/* Configure PA0 pin as input floating */
//	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_0;		//引脚0
//	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN;		//输入模式
//	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL; //浮空，由于外部已经有上拉电阻，这里可以不设置为上拉
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
//	//中断0连接PA0  PA0 -- EXTI0
//	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
//	
//	/* Configure EXTI Line0 */
//	EXTI_InitStructure.EXTI_Line 	= EXTI_Line0;			//中断线0  EXTI0 -- 中断线0
//	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;  //中断模式
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				//中断线使能
//	//配置外部中断控制器
//	EXTI_Init(&EXTI_InitStructure);
//	
//	/* Enable and set EXTI Line0 Interrupt to the lowest priority */
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//中断通道，代码头文件STM32F4xx.h中typedef enum IRQn枚举中可查看到中断的通道编号
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 	//抢占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority 	= 1;        //响应优先级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//使能中断通道
//	NVIC_Init(&NVIC_InitStructure);


//}

///*******************************************
//中断服务函数在startup_stm32f40_41xxx.s中查找
//中断服务函数固定格式:void 中断服务函数(void)

//中断服务函数
//注意点：
//a.中断函数格式: void  中断服务函数名(void)
//b.判断一个函数是否为中断服务函数名，需要查看能否在startup_stm32f40_41xxx.s中
//查找的到，能查到，表示它是中断服务函数，否则不是。
//c.中断服务函数是不需要调用的，当满足中断条件后,CPU自动去执行的函数。
//d.中断是不能执行过长时间
//e.中断服务函数是没有输入及返回值

//********************************************/

//void EXTI0_IRQHandler(void)
//{
//	//判断标志位是否置1
//	if(EXTI_GetITStatus(EXTI_Line0) == SET)
//	{
//	
//		
//		//led灯变更
//		PFout(9) = !PFout(9);
//		
//		//清空中断标志位
//		EXTI_ClearITPendingBit(EXTI_Line0);
//	}
//}


//void EXTI2_IRQHandler(void)
//{
//	//判断标志位是否置1
//	if(EXTI_GetITStatus(EXTI_Line2) == SET)
//	{

//		//led灯变更
//		PFout(9) = !PFout(9);
//		
//		//清空中断标志位
//		EXTI_ClearITPendingBit(EXTI_Line2);
//	}
//}



