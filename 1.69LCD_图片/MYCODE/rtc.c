#include "rtc.h"

//如果需要重新设置时间，需要修改BKP_VALE
#define BKP_VALE   0x2512


void Rtc_init(void)
{
	
	RTC_InitTypeDef		RTC_InitStruct;
	RTC_TimeTypeDef 	RTC_TimeStruct;
	RTC_DateTypeDef		RTC_DateStruct;
	
	
	//1、使能PWR时钟：
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	//2、使能后备寄存器访问,后备寄存器是断电可保存数据的寄存器  
	PWR_BackupAccessCmd(ENABLE);
	
	//读后备寄存器
	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != BKP_VALE)
	{

		//3、配置RTC时钟源，使能RTC时钟：LSE：外部晶振时钟
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
		//如果使用LSE，要打开LSE：
		RCC_LSEConfig(RCC_LSE_ON);
		
		//等待时钟源稳定  
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
		{
		}
	
	
	
		RTC_InitStruct.RTC_AsynchPrediv	= 0x7F; //异步分频器 128分频	
		RTC_InitStruct.RTC_SynchPrediv	= 0xFF; //同步分频器 256分频	
		RTC_InitStruct.RTC_HourFormat	= RTC_HourFormat_24;//24小时制
		//4、 初始化RTC(同步/异步分频系数和时钟格式)：
		RTC_Init(&RTC_InitStruct);
		
		RTC_TimeStruct.RTC_H12		= RTC_H12_PM;   //下午 24小时制，无上/下午说法。
		RTC_TimeStruct.RTC_Hours	= 10;//时
		RTC_TimeStruct.RTC_Minutes	= 49;//分
		RTC_TimeStruct.RTC_Seconds	= 1;//秒
		//5、 设置时间：
		RTC_SetTime (RTC_Format_BIN, &RTC_TimeStruct);
		
		
		RTC_DateStruct.RTC_Year		= 25; //年
		RTC_DateStruct.RTC_Month	= 8;  //月
		RTC_DateStruct.RTC_Date		= 18; //日
		RTC_DateStruct.RTC_WeekDay	= 1;  //星期 
		//6、设置日期：
		RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);
		
		//写入后备寄存器
		RTC_WriteBackupRegister(RTC_BKP_DR0, BKP_VALE);
	}

}

void RTC_Alarm_Ainit(void)
{
	RTC_AlarmTypeDef	RTC_AlarmStruct;
	RTC_TimeTypeDef 	RTC_TimeStruct; 
	EXTI_InitTypeDef    EXTI_InitStructure;
	NVIC_InitTypeDef    NVIC_InitStructure;

	
	//2、关闭闹钟：
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
	
	//时间设置
	RTC_TimeStruct.RTC_H12		= RTC_H12_PM;   //下午 24小时制，无上/下午说法。
	RTC_TimeStruct.RTC_Hours	= 10;//时
	RTC_TimeStruct.RTC_Minutes	= 50;//分
	RTC_TimeStruct.RTC_Seconds	= 10;//秒	

	
	RTC_AlarmStruct.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date; //按日期响应闹钟
	RTC_AlarmStruct.RTC_AlarmDateWeekDay	= 18;  //18号
	RTC_AlarmStruct.RTC_AlarmTime	= RTC_TimeStruct; //时间设置
	RTC_AlarmStruct.RTC_AlarmMask   = RTC_AlarmMask_None;//无掩码位，按实际设置的时间来响应闹钟
	
	//3、配置闹钟参数：
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);
	


	EXTI_InitStructure.EXTI_Line 	= EXTI_Line17;			//中断线17  EXTI17 -- 中断线17
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;  //中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				//中断线使能
	//配置外部中断控制器
	EXTI_Init(&EXTI_InitStructure);
	
	/* Enable and set EXTI Line0 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;			//中断通道，代码头文件STM32F4xx.h中typedef enum IRQn枚举中可查看到中断的通道编号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 	//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 	= 1;        //响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//使能中断通道
	NVIC_Init(&NVIC_InitStructure);

	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	//5、开启闹钟：
	RTC_AlarmCmd(RTC_Alarm_A,ENABLE);

}

void RTC_Alarm_IRQHandler(void)
{

	//判断标志位是否置1
	if(EXTI_GetITStatus(EXTI_Line17) == SET)
	{
		//判断是否为闹钟A
		if(RTC_GetFlagStatus(RTC_FLAG_ALRAF) == SET)
		{
			PFout(9) = 0;
			
			RTC_ClearFlag(RTC_FLAG_ALRAF);
		}

		
		//清空中断标志位
		EXTI_ClearITPendingBit(EXTI_Line17);
	}


}

