#include "stm32f4xx.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "dht11.h"
#include "esp8266.h"
#include "esp8266_mqtt.h"

const uint8_t g_packet_heart_reply[2] = {0xc0,0x00};


char  g_mqtt_msg[526];

uint32_t g_mqtt_tx_len;

//MQTT发送数据
void mqtt_send_bytes(uint8_t *buf,uint32_t len)
{
    esp8266_send_bytes(buf,len);
}

//发送心跳包
int32_t mqtt_send_heart(void)
{	
	uint8_t buf[2]={0xC0,0x00};
    uint32_t cnt=2;
    uint32_t wait=0;	
	
#if 0	
	mqtt_send_bytes(buf,2);
	return 0;
#else
    while(cnt--)
    {	
		mqtt_send_bytes(buf,2);
		memset((void *)g_esp8266_rx_buf,0,sizeof(g_esp8266_rx_buf));
		g_esp8266_rx_cnt=0;	
		
		wait=3000;//等待3s时间
		
		while(wait--)
		{
			delay_ms(1);

			//检查心跳响应固定报头
			if((g_esp8266_rx_buf[0]==0xD0) && (g_esp8266_rx_buf[1]==0x00)) 
			{
				printf("心跳响应确认成功，服务器在线\r\n");
				return 0;
			}
		}
	}
	printf("心跳响应确认失败，服务器离线\r\n");
	return -1;
#endif	

}

//MQTT无条件断开
void mqtt_disconnect(void)
{
	uint8_t buf[2]={0xe0,0x00};
	
    mqtt_send_bytes(buf,2);
	
	esp8266_disconnect_server();
}


//MQTT连接服务器的打包函数
int32_t mqtt_connect(char *client_id,char *user_name,char *password)
{
    uint32_t client_id_len = strlen(client_id);
    uint32_t user_name_len = strlen(user_name);
    uint32_t password_len = strlen(password);
    uint32_t data_len;
    uint32_t cnt=2;
    uint32_t wait=0;
    g_mqtt_tx_len=0;
	
    //可变报头+Payload  每个字段包含两个字节的长度标识
    data_len = 10 + (client_id_len+2) + (user_name_len+2) + (password_len+2);

    //固定报头
    //控制报文类型
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0x10;		//MQTT Message Type CONNECT
    //剩余长度(不包括固定头部)
    do
    {
        uint8_t encodedByte = data_len % 128;
        data_len = data_len / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( data_len > 0 )
            encodedByte = encodedByte | 128;
        g_esp8266_tx_buf[g_mqtt_tx_len++] = encodedByte;
    } while ( data_len > 0 );

    //可变报头
    //协议名
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0;        	// Protocol Name Length MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 4;        	// Protocol Name Length LSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 'M';        // ASCII Code for M
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 'Q';        // ASCII Code for Q
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 'T';        // ASCII Code for T
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 'T';        // ASCII Code for T
    //协议级别
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 4;        	// MQTT Protocol version = 4
    //连接标志
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0xc2;        // conn flags
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0;        	// Keep-alive Time Length MSB
    //保持连接时间
	g_esp8266_tx_buf[g_mqtt_tx_len++] = 60;        	// Keep-alive Time Length LSB  60S心跳包
	//有效载荷
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(client_id_len);// Client ID length MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(client_id_len);// Client ID length LSB
    memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],client_id,client_id_len); //用户ID
    g_mqtt_tx_len += client_id_len;

    if(user_name_len > 0)
    {
        g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(user_name_len);		//user_name length MSB
        g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(user_name_len);    	//user_name length LSB
        memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],user_name,user_name_len); //存放账号到数组中
        g_mqtt_tx_len += user_name_len;
    }

    if(password_len > 0)
    {
        g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(password_len);		//password length MSB
        g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(password_len);    	//password length LSB
        memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],password,password_len);//存放密码到数组中
        g_mqtt_tx_len += password_len;
    }

	
    while(cnt--)
    {
        memset((void *)g_esp8266_rx_buf,0,sizeof(g_esp8266_rx_buf));
		g_esp8266_rx_cnt=0;
		//发送1号报文
        mqtt_send_bytes(g_esp8266_tx_buf,g_mqtt_tx_len);
		
        wait=3000;//等待3s时间
		
        while(wait--)
        {
			delay_ms(1);
			//判断2号报文
			//检查连接确认固定报头
            if((g_esp8266_rx_buf[0]==0x20) && (g_esp8266_rx_buf[1]==0x02)) 
            {
				if(g_esp8266_rx_buf[3] == 0x00)
				{
					printf("连接已被服务器端接受，连接确认成功\r\n");
					return 0;//连接成功
				}
				else
				{
					switch(g_esp8266_rx_buf[3])  //判断2号报文
					{
						case 1:printf("连接已拒绝，不支持的协议版本\r\n");
						break;
						case 2:printf("连接已拒绝，不合格的客户端标识符\r\n");
						break;		
						case 3:printf("连接已拒绝，服务端不可用\r\n");
						break;		
						case 4:printf("连接已拒绝，无效的用户或密码\r\n");
						break;	
						case 5:printf("连接已拒绝，未授权\r\n");
						break;
						default:printf("未知响应\r\n");
						break;
					}
					return 0;
				} 
            }  
        }
    }
	
    return -1;
}

//MQTT订阅/取消订阅数据打包函数
//topic       主题
//qos         消息等级
//whether     订阅/取消订阅请求包
int32_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether)
{
    
	
    uint32_t cnt=2;
    uint32_t wait=0;
	
    uint32_t topiclen = strlen(topic);

    uint32_t data_len = 2 + (topiclen+2) + (whether?1:0);//可变报头的长度（2字节）加上有效载荷的长度
	
	g_mqtt_tx_len=0;
	
    //固定报头
    //控制报文类型
    if(whether) 
		g_esp8266_tx_buf[g_mqtt_tx_len++] = 0x82; //消息类型和标志订阅
    else	
		g_esp8266_tx_buf[g_mqtt_tx_len++] = 0xA2; //取消订阅

    //剩余长度
    do
    {
        uint8_t encodedByte = data_len % 128;
        data_len = data_len / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( data_len > 0 )
            encodedByte = encodedByte | 128;
        g_esp8266_tx_buf[g_mqtt_tx_len++] = encodedByte;
    } while ( data_len > 0 );

    //可变报头
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0;				//消息标识符 MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0x01;           //消息标识符 LSB
	
    //有效载荷
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(topiclen);//主题长度 MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(topiclen);//主题长度 LSB
    memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],topic,topiclen);
	
    g_mqtt_tx_len += topiclen;

    if(whether)
    {
        g_esp8266_tx_buf[g_mqtt_tx_len++] = qos;//QoS级别
    }


    while(cnt--)
    {
		g_esp8266_rx_cnt=0;
        memset((void *)g_esp8266_rx_buf,0,sizeof(g_esp8266_rx_buf));
		//发送订阅消息
        mqtt_send_bytes(g_esp8266_tx_buf,g_mqtt_tx_len);
		
        wait=3000;//等待3s时间
        while(wait--)
        {
			delay_ms(1);
			
			//检查订阅确认报头
            if(g_esp8266_rx_buf[0]==0x90)
            {
				printf("订阅主题确认成功\r\n");
				
				//获取剩余长度
				if(g_esp8266_rx_buf[1]==3)
				{
					printf("Success - Maximum QoS 0 is %02X\r\n",g_esp8266_rx_buf[2]);
					printf("Success - Maximum QoS 2 is %02X\r\n",g_esp8266_rx_buf[3]);		
					printf("Failure is %02X\r\n",g_esp8266_rx_buf[4]);	
				}
				//获取剩余长度
				if(g_esp8266_rx_buf[1]==2)
				{
					printf("Success - Maximum QoS 0 is %02X\r\n",g_esp8266_rx_buf[2]);
					printf("Success - Maximum QoS 2 is %02X\r\n",g_esp8266_rx_buf[3]);			
				}				
				
				//获取剩余长度
				if(g_esp8266_rx_buf[1]==1)
				{
					printf("Success - Maximum QoS 0 is %02X\r\n",g_esp8266_rx_buf[2]);		
				}	
			
                return 0;//订阅成功
            }
            
        }
    }
	
    if(cnt) 
		return 0;	//订阅成功
	
    return -1;
}

//MQTT发布数据打包函数
//topic   主题
//message 消息
//qos     消息等级
uint32_t mqtt_publish_data(char *topic, char *message, uint8_t qos)
{
static 
	uint16_t id=0;	
    uint32_t topicLength = strlen(topic);
    uint32_t messageLength = strlen(message);

    uint32_t data_len;
	uint8_t encodedByte;

    g_mqtt_tx_len=0;
    //有效载荷的长度这样计算：用固定报头中的剩余长度字段的值减去可变报头的长度
    //QOS为0时没有标识符
    //数据长度             主题名   报文标识符   有效载荷
    if(qos)	data_len = (2+topicLength) + 2 + messageLength;
    else	data_len = (2+topicLength) + messageLength;

    //固定报头
    //控制报文类型
    g_esp8266_tx_buf[g_mqtt_tx_len++] = 0x30;    // MQTT Message Type PUBLISH

    //剩余长度
    do
    {
        encodedByte = data_len % 128;
        data_len = data_len / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( data_len > 0 )
            encodedByte = encodedByte | 128;
        g_esp8266_tx_buf[g_mqtt_tx_len++] = encodedByte;
    } while ( data_len > 0 );

    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(topicLength);//主题长度MSB
    g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(topicLength);//主题长度LSB
	
    memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],topic,topicLength);//拷贝主题
	
    g_mqtt_tx_len += topicLength;

    //报文标识符
    if(qos)
    {
        g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE1(id);
        g_esp8266_tx_buf[g_mqtt_tx_len++] = BYTE0(id);
        id++;
    }
	
    memcpy(&g_esp8266_tx_buf[g_mqtt_tx_len],message,messageLength);
	
    g_mqtt_tx_len += messageLength;
	
	//报文发送
	mqtt_send_bytes(g_esp8266_tx_buf,g_mqtt_tx_len);
	
	
	//咱们的Qos等级设置的是00，因此阿里云物联网平台是没有返回响应信息的
	
	return g_mqtt_tx_len;
}



//数据上报
void mqtt_report_devices_status(void)
{


	
	static u8 ret , data[4]={0};
	u8 i =0; 
	memset(g_mqtt_msg, 0, sizeof(g_mqtt_msg));
	while(i<4)
	{
		ret = Dht11_Data(data);
		if(ret == 0)
		{
			printf("传感器收取数据成功\r\n");
		}
		delay_ms(1);
		i++;
	}
	
	//25：温度   temp:湿度
	sprintf(g_mqtt_msg,"#%d#%d#",data[2],data[0]);

    //上报信息到平台服务器
    mqtt_publish_data(MQTT_T_H_PUBLISH_TOPIC,g_mqtt_msg,0);
}

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
	
	//关闭回显
	rt=esp8266_enable_echo(0);
	if(rt)
	{
		printf("esp8266_enable_echo(0) fail\r\n");
		return -3;
	}	
	printf("esp8266_enable_echo(0)success\r\n");
	delay_s(2);	
		
	//连接热点
	rt = esp8266_connect_ap(WIFI_SSID,WIFI_PASSWORD);
	if(rt)
	{
		printf("esp8266_connect_ap fail\r\n");
		return -4;
	}	
	printf("esp8266_connect_ap success\r\n");
	delay_s(2);
	
	//连接巴法云服务器
	rt =esp8266_connect_server("TCP",MQTT_BROKERADDRESS,PORT);
	if(rt)
	{
		printf("esp8266_connect_server fail\r\n");
		return -5;
	}	
	printf("esp8266_connect_server success\r\n");
	delay_s(2);
	
	//进入透传模式
	rt =esp8266_entry_transparent_transmission();
	if(rt)
	{
		printf("esp8266_entry_transparent_transmission fail\r\n");
		return -6;
	}	
	printf("esp8266_entry_transparent_transmission success\r\n");
	delay_s(2);
	
	//MQTT连接巴法云 1号报文
	if(mqtt_connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD))
	{
		printf("mqtt_connect fail\r\n");
		return -7;	
	
	}
	printf("mqtt_connect success\r\n");
	delay_s(2);		
	//STM32向服务发送消息--订阅
	if(mqtt_subscribe_topic(MQTT_SUBSCRIBE_TOPIC,0,1))
	{
		printf("mqtt_subscribe_topic fail\r\n");
		return -8;
	}	
	
	printf("mqtt_subscribe_topic success\r\n");
	
	
	delay_s(2);		
	//STM32向服务发送消息--订阅
	if(mqtt_subscribe_topic(MQTT_T_H_SUBSCRIBE_TOPIC,0,1))
	{
		printf("mqtt_subscribe_topic fail\r\n");
		return -8;
	}	
//	
	printf("mqtt_subscribe_topic success\r\n");
		
	
	return 0;
}
