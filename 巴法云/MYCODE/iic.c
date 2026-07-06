#include "iic.h"

/****************************************
引脚说明
SCL -- PB8
SDA -- PB9

*****************************************/

void Iic_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	
	//打开GPIOB组时钟 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	

	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_9|GPIO_Pin_8;		//引脚8
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_OUT;				//输出模式
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_OD;				//开漏输出 外部有上拉电阻，可配置为开漏
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;					//上拉
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz;				//速度
	
	GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
	//空闲状态
	SCL = 1;
	SDA_OUT = 1;
}

//数据引脚模式
void Iic_Sda_Mode(GPIOMode_TypeDef mode)
{

	GPIO_InitTypeDef  GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin	= GPIO_Pin_9;		//引脚8
	GPIO_InitStruct.GPIO_Mode	= mode;							//输出模式
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_OD;				//开漏输出
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;					//上拉
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz;				//速度
	
	GPIO_Init(GPIOB, &GPIO_InitStruct);	
}

//起始信号
void Iic_Start(void)
{
	//数据线做为输出
	Iic_Sda_Mode(GPIO_Mode_OUT);
	
	SCL = 1;
	SDA_OUT = 1;

	delay_us(5);
	
	SDA_OUT = 0;
	
	delay_us(5);
	
	SCL = 0;

}


//停止信号
void Iic_Stop(void)
{
	//数据线做为输出
	Iic_Sda_Mode(GPIO_Mode_OUT);

	SCL = 0;
	SDA_OUT = 0;

	delay_us(5);

	SCL = 1;

	delay_us(5);

	SDA_OUT = 1;

}

//发送一位数据：1或0
//发一位数据要1个脉冲
void Iic_Send_Ack(u8 ack)
{
	//数据线做为输出
	Iic_Sda_Mode(GPIO_Mode_OUT);	

	SCL = 0;
	//时钟线为低电平
	
	//引脚数据变更   准备数据
	if(ack==1)
		SDA_OUT = 1; 
	else
		SDA_OUT = 0;
	
	delay_us(5);
	
	SCL = 1;
	delay_us(5);
	SCL = 0;

}

//先发送高位数据
void Iic_Send_Byte(u8 txdata)
{
	u8 i;
	//数据线做为输出
	Iic_Sda_Mode(GPIO_Mode_OUT);	
	
	
	SCL = 0;
	
	for(i=0; i<8; i++)
	{
		//数据准备
		if(txdata & (0x01<<(7-i)))
			SDA_OUT = 1; //引脚输出电平
		else
			SDA_OUT = 0;
		
		delay_us(5);
		SCL = 1;
		delay_us(5);
		SCL = 0;
	}
	
}
//接受一位数据
u8 Iic_Recv_Ack(void)
{
	u8 ack = 0;
	
	//数据线做为输入
	Iic_Sda_Mode(GPIO_Mode_IN);		
	
	SCL = 0;

	delay_us(5);
	
	SCL = 1;
	
	//这是SCL为高电平区间，可以判断引脚电平
	
	delay_us(5);
	
	//这是SCL为高电平区间，可以判断引脚电平
	
	if(SDA_IN == 1)
		ack = 1;
	else
		ack = 0;
	
	
	
	SCL = 0;	
	
	
	return ack;
}

//接受一个字节数据
u8 Iic_Recv_Byte(void)
{
	u8 i, rxdata = 0x00;
	
	//数据线做为输入
	Iic_Sda_Mode(GPIO_Mode_IN);			
	
	
	SCL = 0;
	
	for(i=0; i<8; i++)
	{

		delay_us(5);
		
		SCL = 1;
		
		
		delay_us(5);
		
		//判断引脚电平
		if(SDA_IN == 1)
		{
			rxdata |=(0x01<<(7-i));
		}
//		else
//		{
//			rxdata &=~(0x01<<(7-i));
//		}
		
		SCL = 0;
	}	
	
	return rxdata;

}

/*****************************
u8 addr:写数据起始地址

u8 *write_buff：指向数组首地址的指针变量

u8 len：数据长度
*****************************/
void At24c02_Write_Page(u8 addr, u8 *write_buff, u8 len)
{
	u8 ack;
	
	//发起始信号
	Iic_Start();
	
	//发送设备地址执行写操作
	Iic_Send_Byte(AT24C02_ADD_W);
	ack = Iic_Recv_Ack();
	if(ack == 1)
	{
		printf("ack fail 1\r\n");
		Iic_Stop();
		return;
	}
	
	//发送写数据起始地址
	Iic_Send_Byte(addr);
	ack = Iic_Recv_Ack();
	if(ack == 1)
	{
		printf("ack fail 1\r\n");
		Iic_Stop();
		return;
	}	
	
	while(len--)
	{
		//往从机发送（写入数据到AT24C02）
		Iic_Send_Byte(*write_buff);
		ack = Iic_Recv_Ack();
		if(ack == 1)
		{
			printf("ack fail 1\r\n");
			Iic_Stop();
			return;
		}	

		write_buff++;
	}
	
	
	Iic_Stop();
	
	printf("write finish\r\n");

}

