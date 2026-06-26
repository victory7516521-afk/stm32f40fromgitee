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

//启动信号,启动成功返回0，异常返回其它值
int Dht11_Start(void)
{
	int t = 0;
	
	
	//PG9做为输出
	Dht11_Pin_Mode(GPIO_Mode_OUT);
	
	//开始信号
	PGout(9) = 1;
	delay_ms(2);
	PGout(9) = 0;
	delay_ms(20);
	PGout(9) = 1;
	delay_us(30);
	
	//PG9做为输入
	Dht11_Pin_Mode(GPIO_Mode_IN);	
	
	//过滤高电平，等待低电平到来
	while(PGin(9) == 1)
	{
		t++;
		delay_us(2);
		if(t >= 100) //200us未有低电平，则退出
			return -1;
	}
	
	//过滤低电平，等待高电平到来
	t = 0;
	while(PGin(9) == 0)
	{
	
		t++;
		delay_us(2);
		if(t >= 100) //200us未有高电平，则退出
			return -2;	
	
	}
	//过滤高电平，等待低电平到来
	t = 0;
	while(PGin(9) == 1)
	{
		t++;
		delay_us(2);
		if(t >= 100) //200us未有低电平，则退出
			return -3;	
	
	}

	return 0;
}

//读一个字节代码
u8 Dht11_Recv_Byte(void)
{
	//1011 1100
	u8 i, t=0, data = 0x00;   //1000 0000
	
	for(i=0; i<8; i++)
	{
		t = 0;
		//过滤低电平，等待高电平到来
		while(PGin(9) == 0)
		{
			t++;
			delay_us(1);
			if(t > 100) //100us未有高电平，则退出
				return 0;
		}
	
		delay_us(40);
		
		if(PGin(9)) //数据位为1
		{
			
			data |=(0x01<<(7-i));

			
			t = 0;
			//过滤高电平，等待低电平到来
			while(PGin(9) == 1)
			{
				t++;
				delay_us(1);
				if(t > 100) //100us未有低电平，则退出
					return 0;
			}
								
			
		}
//		else //数据位0
//		{
//			data &=~(0x01<<(7-i));
//		}
		
	
	}
	
	
	return data;

}


int Dht11_Data(u8 *data)
{
	int i, ret;
	
	ret = Dht11_Start();
	if(ret == 0)
	{
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
	else
		return -1;


}