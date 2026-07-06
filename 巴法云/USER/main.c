#include "stm32f4xx.h" //这个头文件包含库所有头文件
#include "led.h"
#include "key.h"
#include "exti.h"
#include "delay.h"
#include "tim.h"
#include "pwm.h"
#include "usart.h"
#include "hsr04.h"
#include "dht11.h"
#include "iwdg.h"
#include "rtc.h"
#include "stdio.h"
#include "string.h"
#include "adc.h"
#include "iic.h"
#include "i2c_ee.h"
#include "dma.h"
#include "esp8266.h"
#include "esp8266_mqtt.h"



int main(void)
{	
	uint32_t 	i=0;
	uint32_t 	delay_1ms_cnt=0;
	uint8_t		buf[5]={20,05,56,8,20};
	uint32_t	key_sta=0;
	int32_t		rt=0;


	//中断优先级只能配置一次
	//中断优先级配置 第2组，抢占优先级范围:0~3 响应优先级范围:0~3
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	Delay_Init();
	Led_Init();
	//按键检测
	key_init();
	
	//定时器初始化
	tim3_init();
	Dht11_Init();
	//串口1初始化波特率为115200bps
	usart1_init(115200);
	
	//串口延迟一会，确保芯片内部完成全部初始化,printf无乱码输出
	delay_ms(500);
	
	//打印开机信息
	printf("This is esp8266 mqtt with aliyun test by teacher.chen\r\n");
	
	//等待包头发送完成----非常重要，需要深入理解
	while(esp8266_mqtt_init())
	{
		printf("esp8266_mqtt_init ...");
		
		delay_s(1);
	}
	
	//连接服务器状态指示灯，点亮-连接成功
	CONNECT_MQTT_LED(1);

	printf("esp8266 connect aliyun with mqtt success\r\n");	
	
	while(1)
	{
		//检查接收到数据
		if(g_esp8266_rx_end && g_esp8266_transparent_transmission_sta)
		{
	
			
			//printf("g_esp8266_rx_buf:%s\n",g_esp8266_rx_buf);
			if(strstr(g_esp8266_rx_buf+3,"d9KlACjUD002on")!=NULL)
			{
				PFout(9)=0;
			}else if(strstr(g_esp8266_rx_buf+3,"d9KlACjUD002off")!=NULL)
			{
				PFout(9)=1;
			}
			printf("收到服务器数据:");
			for(int i=0; i<g_esp8266_rx_cnt; i++)
			{
				printf(" %X", g_esp8266_rx_buf[i]);
			}
			
			
			printf("\r\n");
			
			
			//上面自己解析控制灯

			//清空接收缓冲区、接收计数值、接收结束标志位
			memset((void *)g_esp8266_rx_buf,'\0',sizeof g_esp8266_rx_buf);
			g_esp8266_rx_cnt=0;
			g_esp8266_rx_end=0;
		}
		
		delay_1ms_cnt++;
		delay_ms(1);
		
		//6秒时间到达
		if((delay_1ms_cnt % 6000) ==0)
		{	
			
		
			//向服务器发布消息
			mqtt_report_devices_status();	
		}
		
		//60秒时间到达
		if((delay_1ms_cnt % 60000) ==0)
		{
			/*	设备端在保活时间间隔内(保护时间在mqtt_connect设置为60s)，至少需要发送一次报文，包括ping请求。
				连接保活时间的取值范围为30秒~1200秒。建议取值300秒以上。
				从物联网平台发送CONNACK响应CONNECT消息时，开始心跳计时。收到PUBLISH、SUBSCRIBE、PING或 PUBACK消息时，会重置计时器。
			*/
			//发送心跳包，过于频繁发送心跳包，服务器将会持续一段时间不发送响应信息[可选]
			rt = mqtt_send_heart();
			
			if(rt == 0)
				CONNECT_MQTT_LED(1);
			else 
				CONNECT_MQTT_LED(0);			
		}		
		
		//按键检测
		if(key_sta_get())
		{
			delay_ms(50);
			
			key_sta=key_sta_get();
			
			if(key_sta & 0x01)
			{
				printf("connect aliyun mqtt\r\n");
				
				//重连阿里云物联网平台
				rt = esp8266_mqtt_init();
				
				if(rt == 0)
					CONNECT_MQTT_LED(1);
				else 
					CONNECT_MQTT_LED(0);
			}

			if(key_sta & 0x02)				
			{
				printf("disconnect aliyun mqtt\r\n");
				
				//断开阿里云物联网平台
				mqtt_disconnect();
				CONNECT_MQTT_LED(0);
			}		
		
		}
	}
}


