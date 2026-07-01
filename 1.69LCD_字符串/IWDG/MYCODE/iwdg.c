#include "iwdg.h"


void Iwdg_init(void)
{
	//1、 取消寄存器写保护：
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	//2、设置独立看门狗的预分频系数，确定时钟:32KHZ/128 = 250HZ
	IWDG_SetPrescaler(IWDG_Prescaler_128);
	//3、设置看门狗重装载值，确定溢出时间:500/250HZ = 2s(必须在2s喂狗)
	IWDG_SetReload(500);
	//4、使能看门狗，计数器从0xFFF开始喂狗
	IWDG_Enable();
	//5、应用程序喂狗,计数器从500开始喂狗
	IWDG_ReloadCounter();


}