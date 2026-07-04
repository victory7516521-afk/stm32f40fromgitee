#include "rtc.h"

//更改此值，时间重头开始
#define BKP 0x2532

void Rtc_Init(void)
{
	
	
	RTC_InitTypeDef		RTC_InitStruct;
	RTC_TimeTypeDef		RTC_TimeStruct;
	RTC_DateTypeDef		RTC_DateStruct;
	
	
	
	
	//1、使能PWR时钟：
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	//2、使能后备寄存器访问:   
	PWR_BackupAccessCmd(ENABLE);
	//3、配置RTC时钟源，使能RTC时钟：
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	RCC_RTCCLKCmd(ENABLE);
	//如果使用LSE，要打开LSE：
	RCC_LSEConfig(RCC_LSE_ON);
	//4、等待时钟稳定
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
	
	//判断后备寄存器
	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != BKP)
	{
	
		RTC_InitStruct.RTC_HourFormat	= RTC_HourFormat_24; //24小时制
		RTC_InitStruct.RTC_AsynchPrediv = 0x7F; //异步分频 128分频 
		RTC_InitStruct.RTC_SynchPrediv  = 0xFF; //同步分频 256分频
		//5、 初始化RTC(同步/异步分频系数和时钟格式)：
		RTC_Init(&RTC_InitStruct);
		
		
		RTC_TimeStruct.RTC_H12		= RTC_H12_PM; //下午，在24小时制时，此参数无用
		RTC_TimeStruct.RTC_Hours	= 16;
		RTC_TimeStruct.RTC_Minutes	= 44;
		RTC_TimeStruct.RTC_Seconds	= 40;
		//6、 设置时间：
		RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct);
		
		RTC_DateStruct.RTC_Year		= 25;
		RTC_DateStruct.RTC_Month	= 11;
		RTC_DateStruct.RTC_Date		= 28;
		RTC_DateStruct.RTC_WeekDay	= 5;
		//7、设置日期：
		RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
		
		//往后备寄存器写入数据
		RTC_WriteBackupRegister(RTC_BKP_DR0, BKP);
			
	}

}

void RTC_Alarm_AInit(void)
{
	
	RTC_AlarmTypeDef	RTC_AlarmStruct;
	RTC_TimeTypeDef 	RTC_AlarmTime_Set;
	EXTI_InitTypeDef   EXTI_InitStructure;

	NVIC_InitTypeDef   NVIC_InitStructure;	
	
	//2、关闭闹钟：
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
	
	//时间
	RTC_AlarmTime_Set.RTC_H12		= RTC_H12_PM; //下午，在24小时制时，此参数无用
	RTC_AlarmTime_Set.RTC_Hours		= 16;
	RTC_AlarmTime_Set.RTC_Minutes	= 45;
	RTC_AlarmTime_Set.RTC_Seconds	= 40;	
	
	RTC_AlarmStruct.RTC_AlarmTime 	= RTC_AlarmTime_Set;  //时间设置
	RTC_AlarmStruct.RTC_AlarmMask	= RTC_AlarmMask_None;//无掩码位，按实际设置时间响应闹钟
	RTC_AlarmStruct.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date; //按日期设置闹钟
	RTC_AlarmStruct.RTC_AlarmDateWeekDay    = 28;
	
	//3、配置闹钟参数：
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
	
	//4、开启配置闹钟中断：
	EXTI_InitStructure.EXTI_Line 	= EXTI_Line17;   //中断17线
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;   //中断
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;     //中断控制使能
	EXTI_Init(&EXTI_InitStructure);
	
	/* Enable and set EXTI Line0 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel 					 = RTC_Alarm_IRQn;  //NVIC中断通道，可在stm32fxx.h头文件中查找
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  			//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 		 = 0x02;		  	//响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd 				 = ENABLE; 			//通道使能
	NVIC_Init(&NVIC_InitStructure);

	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
	
	//5、开启闹钟：
	RTC_AlarmCmd(RTC_Alarm_A,ENABLE);
}


//闹钟A与闹钟B中断服务函数
void RTC_Alarm_IRQHandler(void)
{

    //判断中断线状态，如果有中断，中断位标志为1
    if(EXTI_GetITStatus(EXTI_Line17) == SET)
    {
		if(RTC_GetFlagStatus(RTC_FLAG_ALRAF) == SET)
		{
		
			LED0_ON;
			
			//清空标志位
			RTC_ClearFlag(RTC_FLAG_ALRAF);
		}
		
		
        
        //清空中断线标志位
        EXTI_ClearITPendingBit(EXTI_Line17);
    }	
	
	
}






