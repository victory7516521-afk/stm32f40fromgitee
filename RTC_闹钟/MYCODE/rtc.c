#include "rtc.h"

//改变这个值，会重新更新时间
#define BKR   0x2615


void Rtc_Init(void)
{
	RTC_InitTypeDef		RTC_InitStruct;
	RTC_TimeTypeDef		RTC_TimeStruct;
	RTC_DateTypeDef		RTC_DateStruct;
	
	//1、使能PWR时钟：
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	//2、使能后备寄存器访问:(掉电可保存数据的寄存器)  
	PWR_BackupAccessCmd(ENABLE);
	
	
	
	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != BKR)
	{
	
		//3、配置RTC时钟源，使能RTC时钟：
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
		//如果使用LSE，要打开LSE：
		RCC_LSEConfig(RCC_LSE_ON);
		//4、等待时钟稳定 也可以在这里延时50ms
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
			
		
		
		
		RTC_InitStruct.RTC_HourFormat	= RTC_HourFormat_24;  //24小时制
		RTC_InitStruct.RTC_AsynchPrediv	= 0x7F; //异步分频器 128分频
		RTC_InitStruct.RTC_SynchPrediv	= 0xFF;//同步分频器 256分频
		//5、 初始化RTC(同步/异步分频系数和时钟格式)：
		RTC_Init(&RTC_InitStruct);
		
		
		RTC_TimeStruct.RTC_H12		= RTC_H12_AM; //上午，在这里无作用
		RTC_TimeStruct.RTC_Hours	= 11; //时
		RTC_TimeStruct.RTC_Minutes	= 10; //分
		RTC_TimeStruct.RTC_Seconds	= 20; //秒
		//6、 设置时间：
		RTC_SetTime (RTC_Format_BIN, &RTC_TimeStruct);
		
		RTC_DateStruct.RTC_Year		= 26;  //年
		RTC_DateStruct.RTC_Month	= 6;   //月
		RTC_DateStruct.RTC_Date		= 25;  //日
		RTC_DateStruct.RTC_WeekDay	= 4;   //星期
		//7、设置日期：
		RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct);	

		RTC_WriteBackupRegister(RTC_BKP_DR0, BKR);//写入后备寄存器
	}
}

void RTC_Alarm_AInit(void)
{
	RTC_AlarmTypeDef	RTC_AlarmStruct;
	EXTI_InitTypeDef    EXTI_InitStructure;
	NVIC_InitTypeDef    NVIC_InitStructure;
	//关闭闹钟--改
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE); 
	
	RTC_AlarmStruct.RTC_AlarmTime.RTC_H12	 = RTC_H12_AM; //上午，在这里无作用
	RTC_AlarmStruct.RTC_AlarmTime.RTC_Hours	 = 11; //时
	RTC_AlarmStruct.RTC_AlarmTime.RTC_Minutes= 11; //分
	RTC_AlarmStruct.RTC_AlarmTime.RTC_Seconds= 20; //秒
	RTC_AlarmStruct.RTC_AlarmMask			 = RTC_AlarmMask_None;//无掩码位，按实际情况响应闹钟
	RTC_AlarmStruct.RTC_AlarmDateWeekDaySel	 = RTC_AlarmDateWeekDaySel_Date;//按日期来响应
	RTC_AlarmStruct.RTC_AlarmDateWeekDay	 = 25;
	//配置闹钟参数--改
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStruct);

	
	EXTI_InitStructure.EXTI_Line 	= EXTI_Line17;			//外部中断线17 -- 改
	EXTI_InitStructure.EXTI_Mode 	= EXTI_Mode_Interrupt;  //中断
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //触发方式：下降沿 -- 看情况改
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;		//中断线使能
	EXTI_Init(&EXTI_InitStructure);
	
	//NVIC配置
    NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;   //通道编号，可在stm32f4xx.h中查看 -- 改
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;     //抢占优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;            //响应优先级 -- 看情况改
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                    //通道使能
    NVIC_Init(&NVIC_InitStructure);
	
	
   RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	//开启闹钟：
   RTC_AlarmCmd(RTC_Alarm_A,ENABLE);

}


void RTC_Alarm_IRQHandler(void) //--改
{
    //判断中断线状态，如果有中断，中断位标志为1
    if(EXTI_GetITStatus(EXTI_Line17) == SET) //--改
    {
		if(RTC_GetFlagStatus(RTC_FLAG_ALRAF) == SET)
		{
			PFout(9) = 0;
			
			RTC_ClearFlag(RTC_FLAG_ALRAF);
		}
		

        //清空中断线
        EXTI_ClearITPendingBit(EXTI_Line17);//--改
    }
}
