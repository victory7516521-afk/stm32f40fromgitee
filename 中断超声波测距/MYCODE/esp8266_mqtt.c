#include "stm32f4xx.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "esp8266.h"
#include "esp8266_mqtt.h"


int32_t esp8266_mqtt_init(void)
{
	int32_t rt;
	
	//esp8266初始化
	esp8266_init();

//	printf("esp8266_init");

	//退出透传模式，才能输入AT指令
	rt=esp8266_exit_transparent_transmission();
	if(rt)
	{
		printf("esp8266_exit_transparent_transmission fail\r\n");
		return -1;
	}	
	printf("esp8266_exit_transparent_transmission success\r\n");
	delay_s(2);
	
	//复位模块
	rt=esp8266_reset();
	if(rt)
	{
		printf("esp8266_reset fail\r\n");
		return -2;
	}
	printf("esp8266_reset success\r\n");
	delay_s(2);	
	
	
	g_esp8266_rx_cnt = 0;
	memset(g_esp8266_rx_buf, 0, sizeof g_esp8266_rx_buf);
	
	//关闭回显
	rt=esp8266_enable_echo(0);
	if(rt)
	{
		printf("esp8266_enable_echo(0) fail\r\n");
		return -3;
	}	
	printf("esp8266_enable_echo(0)success\r\n");
	delay_s(2);	
	
	g_esp8266_rx_cnt = 0;
	memset(g_esp8266_rx_buf, 0, sizeof g_esp8266_rx_buf);	
		
	
	//连接热点
	rt = esp8266_connect_ap(WIFI_SSID,WIFI_PASSWORD);
	if(rt)
	{
		printf("esp8266_connect_ap fail\r\n");
		return -4;
		
	}	
	printf("esp8266_connect_ap success\r\n");
	delay_s(2);
	
	g_esp8266_rx_cnt = 0;
	memset(g_esp8266_rx_buf, 0, sizeof g_esp8266_rx_buf);
	
	//进入单链接
	rt=esp8266_link_mode();
	if(rt)
	{
		printf("esp8266_enable_echo(0) fail\r\n");
		return -5;
	}		
	delay_s(2);	
	
	g_esp8266_rx_cnt = 0;
	memset(g_esp8266_rx_buf, 0, sizeof g_esp8266_rx_buf);	

	
	
	
	rt =esp8266_connect_server("TCP",WEATHER_URL,WEATHER_PORT);
	if(rt)
	{
		printf("esp8266_connect_server fail\r\n");
		return -6;
	}	
	printf("esp8266_connect_server success\r\n");
	delay_s(2);
	
	
	g_esp8266_rx_cnt = 0;
	memset(g_esp8266_rx_buf, 0, sizeof g_esp8266_rx_buf);
	
	//进入透传模式
	rt =esp8266_entry_transparent_transmission();
	if(rt)
	{
		printf("esp8266_entry_transparent_transmission fail\r\n");
		return -7;
	}	
	printf("esp8266_entry_transparent_transmission success\r\n");
	delay_s(2);
	
	g_esp8266_rx_cnt = 0;
	memset(g_esp8266_rx_buf, 0, sizeof g_esp8266_rx_buf);
	
	return 0;
}
