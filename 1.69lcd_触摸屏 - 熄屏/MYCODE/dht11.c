#include "dht11.h"


/**********************************
引脚说明：

PG9 -- DQ(DHT11)
***********************************/

void Dht11_Init(void)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	
	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_9;				//第9号引脚
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;			//输出模式
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;			//推挽输出，增强驱动能力，引脚的输出电流更大
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_25MHz;			//引脚的速度最大为25MHz
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;			//没有使用内部上拉电阻
	GPIO_Init(GPIOG, &GPIO_InitStructure);	
	
	//温湿度模块还没有工作，那么它的触发引脚是高电平
	PGout(9) = 1;
}

//引脚模式变更
void Dht11_Pin_Mode(GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef  	GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;				//第9号引脚
	GPIO_InitStructure.GPIO_Mode  = mode;					//输入/输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出，增强驱动能力，引脚的输出电流更大
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;		//引脚的速度最大为100MHz
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;		//没有使用内部上拉电阻
	GPIO_Init(GPIOG, &GPIO_InitStructure);	
}

//启动DHT11
//返回值为0,表示启动DHT11成功
int Dht11_Start(void)
{
	u16 t = 0;
	
	//启动信号
	//PG9配置为输出
	Dht11_Pin_Mode(GPIO_Mode_OUT);
	PGout(9) = 1;
	delay_ms(2);
	PGout(9) = 0;
	delay_ms(20);
	PGout(9) = 1;
	delay_us(30);
	
	//响应信号
	//PG9配置为输入
	Dht11_Pin_Mode(GPIO_Mode_IN);	
	
	t = 0;
	//等待低电平到来
	while(PGin(9) == 1)
	{
		delay_us(2);
		t++;
		if(t >= 250) //500us未有低电平
			return -1;
		
	
	}
	//程序执行到这里，说明DATA有低电平到来(低电平由DHT11发出的)
	
	
	t = 0;
	//等待高电平到来，过滤低电平
	while(PGin(9) == 0)
	{
		delay_us(2);
		t++;
		if(t >= 100) //200us未有高电平
			return -2;

	}	
	
	//程序执行到这里，说明DATA有高电平到来(高电平由DHT11发出的)
	
	
	
	t = 0;
	//等待低电平到来，过滤高电平
	while(PGin(9) == 1)
	{
		delay_us(2);
		t++;
		if(t >= 100) //200us未有低电平
			return -2;

	}		

	
	return 0;
	

}

#if 1

//接受一个字节
u8 Dht11_Recv_Byte(void)
{
	u16 t = 0, tim_us;
	//1 1 0 1 1 0 0 1
	u8 i, rxdata = 0x00; //0000 0000
	
	for(i=0; i<8; i++)
	{
		TIM3->CNT = 0;
		
		//过滤低电平，等待高电平到来
		while(PGin(9) == 0);
		//使能定时器
		TIM_Cmd(TIM3, ENABLE);
		
		//过滤高电平，等待低电平到来
		while(PGin(9) == 1);		
		//获取高电平的持续时间
		tim_us = TIM3->CNT;
		//不使能定时器
		TIM_Cmd(TIM3, DISABLE);	

		//通过判断高电平的时长，确定数据位为1还是0
		if(tim_us > 55 &&  tim_us < 80)
		{
			rxdata |= (0x01<<(7-i));
		}
		else
		{
			rxdata &= ~(0x01<<(7-i));
		}

	}
	
	return rxdata;
	
}

#else

//接受一个字节
u8 Dht11_Recv_Byte(void)
{
	u16 t = 0;
	u8 i, rxdata = 0x00; //0000 0000
	
	for(i=0; i<8; i++)
	{
		t = 0;
		//等待高电平的到来,过滤低电平
		while(PGin(9) == 0)
		{
			t++;
			delay_us(2);
			
			if(t >= 100) //等待200us未有高电平到来，则返回
				return 0;
		}	
			
		//程序来到这里，说明是高电平
		
		delay_us(40);
		
		//延时40us后，再查看引脚电平，如果还是高电平，说明数据位为1
		
		if(PGin(9))
		{
			//先发高位，先合成高位
			rxdata |= (1<<(7-i));
			
			
			//还有大概30us高电平
			t = 0;
			//等待低电平的到来，过滤高电平
			while(PGin(9) == 1)
			{
				t++;
				delay_us(2);
				
				if(t >= 100) //等待200us未有低电平到来，则返回
					return 0;
			}			
		}
		
		//下面代码可要可不要
//		else  //数据位0
//		{
//			rxdata &= ~(1<<(7-i));
//		
//		}
//	
	
	}
	
	return rxdata;
	
}




#endif

//获取DHT11数据
int Dht11_Data(u8 *data)
{
	int i, ret;
	
	ret = Dht11_Start();
	if(ret != 0)
		return -1;

	for(i=0; i<5; i++)
	{
		data[i] = Dht11_Recv_Byte();
	}
	
	if(data[4] == data[0]+data[1]+data[2]+data[3])
	{
		return 0;
	}
	else
	{
		return -1;
	}
	
}

