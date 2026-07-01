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
	
	//空闲状态 这里可以不写
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

//启动信号--主机发出
void Iic_Start(void)
{
	//SDA做为输出
	Iic_Sda_Mode(GPIO_Mode_OUT);
	
	SCL = 1;
	SDA_OUT = 1;
	delay_us(5);
	
	SDA_OUT = 0;
	delay_us(5);
	//钳住总线
	SCL = 0;
}

//停止信号--主机发出
void Iic_Stop(void)
{
	//SDA做为输出
	Iic_Sda_Mode(GPIO_Mode_OUT);
	
	SCL = 0;
	SDA_OUT = 0;

	delay_us(5);
	
	SCL = 1;
	
	delay_us(5);
	
	SDA_OUT = 1;
}
//发送一位数据
void Iic_Send_Ack(u8 ack)
{

	//SDA做为输出
	Iic_Sda_Mode(GPIO_Mode_OUT);

	//发一位数据，需要一个时钟脉冲
	//时钟线为低电平区间，引脚电平可变化
	SCL = 0;
	
	//引脚数据变更
	if(ack == 1)
	{
		SDA_OUT = 1; //数据位为1，引脚输出高电平
	}	
	else
	{
		SDA_OUT = 0; //数据位为0，引脚输出低电平
	}		

	delay_us(5);
	

	SCL = 1;
	

	delay_us(5);
		

	SCL = 0;
	
}

//发送一个字节数据(1字节8位数据) 先发高位
//data值：0x87 (1 0 0 0 0 1 1 1)
void Iic_Send_Byte(u8 data)
{
	u8 i;

	//SDA做为输出
	Iic_Sda_Mode(GPIO_Mode_OUT);
	

	SCL = 0;

	for(i=0; i<8; i++)
	{
		//在时钟线为低电平区间，改变引脚状态
		if(data & (0x01<<(7-i)))
			SDA_OUT = 1;
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
	u8 ack=0;
	
	
		//SDA做为输入
	Iic_Sda_Mode(GPIO_Mode_IN);
	
	SCL = 0;
	
	delay_us(5);
	
	SCL = 1;
	
	delay_us(5);
	//判断引脚	
	if(SDA_IN)
		ack = 1;
	else
		ack = 0;


	SCL = 0;	

	return ack;
}


//接受一个数据
u8 Iic_Recv_Byte(void)
{
	u8 i, data = 0x00;

	//SDA做为输出
	Iic_Sda_Mode(GPIO_Mode_IN);
	

	SCL = 0;

	for(i=0; i<8; i++)
	{


		delay_us(5);
		

		SCL = 1;
		

		delay_us(5);
		//判断引脚电平，数据合成
		if(SDA_IN) //数据位为1
		{
			data |=(0x01<<(7-i));

		}
//		else //数据位0
//		{
//			data &=~(0x01<<(7-i));
//		}
		

		SCL = 0;	
	
	}
	return data;
}
/***********************************
函数说明：At24c02写页操作
u8 addr：写数据起始地址
u8 *write_buff：数据起始地址
u8 len：数据长度
***********************************/
void At24c02_Write_Page(u8 addr, u8 *write_buff, u8 len)
{
	u8 ack;
	
	if(len > 8)
	{
		printf("data too long\r\n");
		return;
	}
	
	//启动信号--主机发出
	Iic_Start();
	
	//发送设备地址，执行写操作
	Iic_Send_Byte(AT24C02_ADD_W);
	ack = Iic_Recv_Ack();
	if(ack == 1)
	{
		printf("ack failure1\r\n");
		return;
	}
	
	//发送写数据起始地址
	Iic_Send_Byte(addr);
	ack = Iic_Recv_Ack();
	if(ack == 1)
	{
		printf("ack failure2\r\n");
		return;
	}	
	
	while(len--)
	{
	
		//写数据
		Iic_Send_Byte(*write_buff);
		ack = Iic_Recv_Ack();
		if(ack == 1)
		{
			printf("ack failure3\r\n");
			return;
		}	
		//地址加1，指向下一个数据
		write_buff++;
	}
//	for(int i = 0; i<len; i++)
//	{
//		//写数据
//		//Iic_Send_Byte(*write_buff++);
//		Iic_Send_Byte(write_buff[i]);
//		ack = Iic_Recv_Ack();
//		if(ack == 1)
//		{
//			printf("ack failure1\r\n");
//			return;
//		}		
//	
//	}
	
	//停止信号--主机发出
	Iic_Stop();
	
	printf("write finish\r\n");

}

void At24c02_Read_Data(u8 addr, u8 *read_buff, u8 len)
{
	u8 ack;
	
	
	//启动信号--主机发出
	Iic_Start();
	
	//发送设备地址，执行写操作
	Iic_Send_Byte(AT24C02_ADD_W);
	ack = Iic_Recv_Ack();
	if(ack == 1)
	{
		printf("ack failure1\r\n");
		return;
	}
	
	//发送读数据起始地址
	Iic_Send_Byte(addr);
	ack = Iic_Recv_Ack();
	if(ack == 1)
	{
		printf("ack failure1\r\n");
		return;
	}

	
	//启动信号--主机发出
	Iic_Start();
	
	//发送设备地址，执行读操作
	Iic_Send_Byte(AT24C02_ADD_R);
	ack = Iic_Recv_Ack();
	if(ack == 1)
	{
		printf("ack failure1\r\n");
		return;
	}
	
	while(len--)  
	{
		//接受字节
		*read_buff = Iic_Recv_Byte();
		if(len > 0)
		{
			//发送有效应答
			Iic_Send_Ack(0);	
		}
		read_buff++;
	}
	//发送无效应答
	Iic_Send_Ack(1);

	Iic_Stop();
}