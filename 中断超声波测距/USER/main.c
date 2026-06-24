#include "stm32f4xx.h"
#include <stdio.h>
#include "led.h"
#include "key.h"
#include "sys.h"
#include "exti.h"
#include "delay.h"
#include "tim.h"
#include "pwm.h"
#include "usart.h"
#include "string.h"
#include "esp8266.h"
#include "esp8266_mqtt.h"
#include "cjson.h"
#include "ultrasonic.h"
#include "delay.h"


int main(void)
{
	//中断优先级分组，且一个工程只能设置一次
	//抢占优先级范围:0~3  响应优先级:0~3
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	Delay_init();
	Key_init();
	Led_Init();
    SR04_Init();
    Usart1_init(115200);

    while(1)
    {
        delay_s(1);
        SR04_RAND();
    }


}
	

