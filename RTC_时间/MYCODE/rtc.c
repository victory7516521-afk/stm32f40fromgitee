#include "rtc.h"

void Rtc_Init(void)
{
	RTC_InitTypeDef		RTC_InitStruct;
	RTC_TimeTypeDef		RTC_TimeStruct;
	RTC_DateTypeDef		RTC_DateStruct;
	
	//1、使能PWR时钟：
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	//2、使能后备寄存器访问:(掉电可保存数据的寄存器)  
	PWR_BackupAccessCmd(ENABLE);
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


}