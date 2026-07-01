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
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_OD;				//开漏输出
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
	GPIO_InitStruct.GPIO_Mode	= mode;							//引脚模式
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_OD;				//开漏输出
	GPIO_InitStruct.GPIO_PuPd	= GPIO_PuPd_UP;					//上拉
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_50MHz;				//速度
	
	GPIO_Init(GPIOB, &GPIO_InitStruct);	
}

//启动信号--主机发出
void Iic_Start(void)
{
	
	Iic_Sda_Mode(GPIO_Mode_OUT);
	
	//空闲状态
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
	Iic_Sda_Mode(GPIO_Mode_OUT);
	
	
	SCL = 0;
	SDA_OUT = 0;	
	
	delay_us(5);
	
	SCL = 1;
	delay_us(5);
	
	SDA_OUT = 1;

}

//发送一位数据(一个脉冲)
//ack取值为1或者0
void Iic_Send_Ack(u8 ack)
{
	
	Iic_Sda_Mode(GPIO_Mode_OUT);
	
	
	
	SCL = 0;
	
	//时钟线为低电平区间，引脚可变更
	if(ack == 1)
	{
		SDA_OUT = 1; //引脚输出高电平
	}
	else
	{
		SDA_OUT = 0; //引脚输出低电平
	}
	
	
	delay_us(5);
	
	SCL = 1;
	
	delay_us(5);
	
	SCL = 0;
	
}

//发送一个字节(1字节8位)
//先发高位 
void Iic_Send_Byte(u8 data)  //1 0 1 1 0 0 1 1
{

	u8 i;
	
	Iic_Sda_Mode(GPIO_Mode_OUT);
	
	
	SCL = 0;
	
	for(i=0; i<8; i++)
	{
		//时钟线为低电平时，判断数据位，设置输出引脚电平
		if( data & (0x01<<(7-i)))
		{
			SDA_OUT = 1;
		}
		else
		{
			SDA_OUT = 0;
		}
		
	
		delay_us(5);
		
		SCL = 1;
		
		delay_us(5);
		
		SCL = 0;	
	}

}

//接收一位数据
u8 Iic_Recv_Ack(void)
{
	u8 ack = 0;
	
	Iic_Sda_Mode(GPIO_Mode_IN);
	
	
	
	SCL = 0;

	delay_us(5);
	
	//时钟线为高电平区间，判断引脚电平
	
	SCL = 1;
	
	delay_us(5);
	//判断引脚电平
	if(SDA_IN == 1)
		ack = 1;
	else
		ack = 0;
	
	SCL = 0;
	
	return ack;
}

//接收一个字节数据
u8 Iic_Recv_Byte(void)
{
	u8 i, data = 0x00;  
	
	Iic_Sda_Mode(GPIO_Mode_IN);
	
	
	SCL = 0;
	
	for(i=0; i<8; i++)
	{

		delay_us(5);
		
		SCL = 1;
		
		delay_us(5); 
		
		//判断引脚电平
		if(SDA_IN == 1) 
			data |= (0x01<<(7-i));
//		else
//			data &= ~(0x01<<(7-i));
			
		SCL = 0;	
	}
	
	
	return data;

}


void At24c02_Write_Page(u8 addr, u8 *write_buf, u8 len)
{
	u8 ack=0;
	
//	//数据数据长度是否大于8
//	if(len > 8)
//	{
//		printf("write data more than 8 byte\r\n");
//		return;
//	}
	
	//启动信号
	Iic_Start();

	//发送设备地址，执行写操作
	Iic_Send_Byte(AT24C02_WR_ADD);
	ack = Iic_Recv_Ack(); //判断ACK
	if(ack)
	{
		printf("recv fail1\r\n");
		return;
	}
	

	//发送写数据的起始地址
	Iic_Send_Byte(addr);
	ack = Iic_Recv_Ack(); //判断ACK
	if(ack)
	{
		printf("recv fail2\r\n");
		return;
	}	
	
	for(int i=0; i<len; i++)
	{
		Iic_Send_Byte(*write_buf++);
		ack = Iic_Recv_Ack(); //判断ACK
		if(ack)
		{
			printf("recv fail3\r\n");
			return;
		}		
	
	}
	
	printf("write finish\r\n");
	
	//停止信号
	Iic_Stop();

}

void At24c02_Read_Data(u8 addr, u8 *read_buff, u8 len)
{
	u8 ack=0;
	
	//启动信号
	Iic_Start();
	

	//发送设备地址，执行写操作
	Iic_Send_Byte(AT24C02_WR_ADD);
	ack = Iic_Recv_Ack(); //判断ACK
	if(ack)
	{
		printf("recv fail1\r\n");
		return;
	}	


	//发送读数据的起始地址
	Iic_Send_Byte(addr);
	ack = Iic_Recv_Ack(); //判断ACK
	if(ack)
	{
		printf("recv fail2\r\n");
		return;
	}	
	
	
	//启动信号
	Iic_Start();	
	
	//发送设备地址，执行读操作
	Iic_Send_Byte(AT24C02_RD_ADD);
	ack = Iic_Recv_Ack(); //判断ACK
	if(ack)
	{
		printf("recv fail1\r\n");
		return;
	}	
	//len = 5
	for(int i=0; i<len; i++)
	{
		*read_buff++ = Iic_Recv_Byte();
		
		if(i == len-1)
		{
			//发送无效应答
			Iic_Send_Ack(1);	
		}
		else
		{
			//发送有效应答
			Iic_Send_Ack(0);		
		}

		
	}

	
	//停止信号
	Iic_Stop();
}





