#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"



void Dht11_Init(void);
int Dht11_Start(void);
u8 Dht11_Recv_Byte(void);

/*************************************
函数功能：获取温湿度
返回值
0:获取数据成功
-1：获取数据失败

参数:u8 *data
传入存储数据数组

data[0]:湿度整数
data[1]:湿度小数
data[2]:温度整数
data[3]:温度小数
data[4]:校验值

**************************************/
int Dht11_Data(u8 *data);

#endif
