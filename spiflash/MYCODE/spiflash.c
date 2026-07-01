#include "spiflash.h"

/*
引脚说明

使用SPI1
SCK连接PB3
MISO连接PB4
MOSI连接PB5
CS连接PB14(普通引脚输出)
*/
void Spiflash_Init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	SPI_InitTypeDef		SPI_InitStruct;
	
	//使能SPIx和IO口时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	//3、GPIO 初始化设置：要设置模式为复用功能。
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5; 		//引脚 -- 改
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;		//复用功能 
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP ;
	//引脚初始化--改
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
	
	//CS
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_14;	//引脚
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;//输出模式
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;//推挽模式
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_25MHz; //速度
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP; //上拉
	//引脚初始化--改
	GPIO_Init(GPIOB, &GPIO_InitStructure); 	
	
	
	
	//4、设置引脚复用器映射：调用 GPIO_PinAFConfig 函数。
	//引脚映射 -- 改  
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);		
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
	
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128; //分频
	//根据从机选择工作方式0
	SPI_InitStruct.SPI_CPHA		= SPI_CPHA_1Edge;  //第一边沿
	SPI_InitStruct.SPI_CPOL		= SPI_CPOL_Low;   //空闲电平为低电平
	SPI_InitStruct.SPI_DataSize	= SPI_DataSize_8b; //8位
	SPI_InitStruct.SPI_Direction= SPI_Direction_2Lines_FullDuplex; //全双工
	SPI_InitStruct.SPI_FirstBit	= SPI_FirstBit_MSB; //先传高位
	SPI_InitStruct.SPI_Mode		= SPI_Mode_Master;//主机
	SPI_InitStruct.SPI_NSS		= SPI_NSS_Soft;//使能端，这里选择软控制（指的是自己写程序何时使能或不使能）
	SPI_InitStruct.SPI_CRCPolynomial= 7; //CRC无使用，随意写数据
	//初始化SPIx,设置SPIx工作模式
	SPI_Init(SPI1, &SPI_InitStruct);
	//使能SPIx
	SPI_Cmd(SPI1, ENABLE);
	
	//不使能芯片
	F_CS = 1;

}

//收发数据
u8 Spi1_Send_Recv_Byte(u8 txdata)
{
	u8 rxdata = 0x00;
	
	//发送数据
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) ==  RESET);
	//将数据放入缓冲区
	SPI_I2S_SendData(SPI1, txdata);
	
	
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) ==  RESET);
	//获取缓冲区数据
	rxdata = SPI_I2S_ReceiveData(SPI1) ;
	
	return rxdata;
}

u16 W25q128_Id(void)
{
	u16 id = 0x00;
	
	//使能芯片
	F_CS = 0;
	
	//发送读生产商/设备ID命令
	Spi1_Send_Recv_Byte(0x90);
	//发送地址  0x000000地址分三次发送
	Spi1_Send_Recv_Byte(0x00);
	Spi1_Send_Recv_Byte(0x00);
	Spi1_Send_Recv_Byte(0x00);
	
	//发送任意字符获取生产商ID
	id |= Spi1_Send_Recv_Byte(0xAA)<<8; //生产商ID存放在15~8位
	id |= Spi1_Send_Recv_Byte(0x66); //设备ID存放在7~0位
	
	//不使能芯片
	F_CS = 1;	
	
	return id;
}

void Write_Enable(void)
{
	//使能芯片
	F_CS = 0;
 
	//发送写使能
	Spi1_Send_Recv_Byte(0x06);	
	
	//不使能芯片
	F_CS = 1;
}

//扇区擦除
void Erase_Sector(u32 addr)
{
	Write_Enable();
	
	//使能芯片
	F_CS = 0;

	//发送擦除扇区命令
	Spi1_Send_Recv_Byte(0x20);	
	
	//发送擦除扇区地址 0x800000
	Spi1_Send_Recv_Byte(addr>>16);  //发送23~16位地址	
	Spi1_Send_Recv_Byte(addr>>8);   //发送15~8位地址	
	Spi1_Send_Recv_Byte(addr);   	//发送7~0位地址	
	//不使能芯片
	F_CS = 1;	
	
	
	//使能芯片
	F_CS = 0;

	//等待擦除完成
	
	//发送读状态寄存器1命令
	Spi1_Send_Recv_Byte(0x05);		
	
	while(1)
	{
		//发送任意字符获取状态寄存器值 判断最低位是否为0，并判断BUSY是否为0
		if((Spi1_Send_Recv_Byte(0xEE) & 0x01) == 0x00)
			break;
	
	}
	
	//不使能芯片
	F_CS = 1;		

}


void Page_Write(u32 addr, u8 *write_buff, u32 len)
{
	//写使能
	Write_Enable();

	
	//使能芯片
	F_CS = 0;
	
	//发送写数据命令
	Spi1_Send_Recv_Byte(0x02);
	
	Spi1_Send_Recv_Byte(addr>>16); //先发23~16位地址
	Spi1_Send_Recv_Byte(addr>>8);  //发15~8位地址
	Spi1_Send_Recv_Byte(addr);     //发7~0位地址

	
	
	//写数据
	while(len--)
	{
		Spi1_Send_Recv_Byte(*write_buff); 
		write_buff++;
	}

	//不使能芯片
	F_CS = 1;
	
	
	//读状态寄存器
	F_CS = 0;
	//发送读状态寄存器1命令
	Spi1_Send_Recv_Byte(0x05);	
	
	
	
	while(1)
	{
		
	
		//判断最低位是否为0
		if( (Spi1_Send_Recv_Byte(0xFF) & 0x01) == 0)
			break;
	
	}
	
	F_CS = 1;
	
}

//读数据
void Read_Data(u32 addr, u8 *read_buff, u32 len)
{

	//使能芯片
	F_CS = 0;

	//发送读数据命令	
	Spi1_Send_Recv_Byte(0x03);

	Spi1_Send_Recv_Byte(addr>>16); //先发23~16位地址
	Spi1_Send_Recv_Byte(addr>>8);  //发15~8位地址
	Spi1_Send_Recv_Byte(addr);     //发7~0位地址	
	
	while(len--)
	{
		*read_buff = Spi1_Send_Recv_Byte(0x11);
		read_buff++;
	}
	
	//使能芯片
	F_CS = 1;
}

