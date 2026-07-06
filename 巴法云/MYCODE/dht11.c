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


int Dht11_Start(void)
{
	u16 t = 0;
	
	/*发送开始信号*/
	//引脚做为输出
	Dht11_Pin_Mode(GPIO_Mode_OUT);
	PGout(9) = 1;
	delay_ms(2);
	PGout(9) = 0;
	delay_ms(20);
	PGout(9) = 1;
	delay_us(30);	
	
	//引脚做为输入
	Dht11_Pin_Mode(GPIO_Mode_IN);	

#if 0	
	
	
	/*下面为粗暴写法，如果硬件有问题，会导致程序卡死*/
	
	//等待低电平到来
	while(PGin(9) == 1);
	//程序执行到这里，表示PG9为低电平
	
	//等待高电平到来，过滤低电平
	while(PGin(9) == 0);
	//程序执行到这里，表示PG9为高电平
	
	//等待低电平到来，过滤高电平
	while(PGin(9) == 1);
	//程序执行到这里，表示PG9为低电平
	//后面时序图即是数据位

#else
	//等待低电平到来
	t = 0;
	while(PGin(9) == 1)
	{
		delay_us(2);
		t++;
		if(t >= 100)
			return -1; //等待200us，未有低电平，则返回
	
	}
	//程序执行到这里，表示PG9为低电平
	
	//等待高电平到来，过滤低电平
	t = 0;
	while(PGin(9) == 0)
	{
		delay_us(2);
		t++;
		if(t >= 100)
			return -2; //等待200us（一定大于80us），未有高电平，则返回	
	}
	//程序执行到这里，表示PG9为高电平
	
	//等待低电平到来，过滤高电平
	t = 0;
	while(PGin(9) == 1)
	{
		delay_us(2);
		t++;
		if(t >= 100)
			return -3; //等待200us，未有低电平，则返回	
	}
	//程序执行到这里，表示PG9为低电平
	//后面时序图即是数据位

	return 0;


#endif

}


u8 Dht11_Recv_Byte(void)
{
	
	u8 t, i, data = 0x00;
	
	for(i=0; i<8; i++)
	{
		//等待高电平到来，过滤低电平
		t = 0;
		while(PGin(9) == 0)
		{
			delay_us(2);
			t++;
			if(t >= 100)
				return 0; //等待200us，未有低电平，则返回		
		}
		//程序执行到这里，表示PG9为高电平
		
		
		delay_us(40);

		if(PGin(9) == 1)
		{
			data |= (0x01<<(7-i));
			
			t = 0;
			//等待低电平到来，过滤高电平
			while(PGin(9) == 1)
			{
				delay_us(2);
				t++;
				if(t >= 100)
					return 0; //等待200us，未有低电平，则返回				
			}
			//程序执行到这里，表示PG9为低电平			
			
		}
		else{
			data &= ~(0x01<<(7-i));
		}
		
	
	}


	return data;
}


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
	
	//校验判断
	if(data[4] == data[0]+data[1]+data[2]+data[3])
		return 0;
	else
		return -1;

}










