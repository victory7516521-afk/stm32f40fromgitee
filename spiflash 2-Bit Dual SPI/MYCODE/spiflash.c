#include "spiflash.h"

/*
 * W25Q128 SPI Flash驱动程序
 * STM32F407平台
 * 
 * 引脚说明：
 *   SPI1接口
 *   SCK  -> PB3  (SPI1_SCK)
 *   MISO -> PB4  (SPI1_MISO)
 *   MOSI -> PB5  (SPI1_MOSI)
 *   CS   -> PB14 (普通GPIO输出，软件控制片选)
 * 
 * W25Q128规格：
 *   容量：128Mbit = 16MB
 *   页大小：256字节
 *   扇区大小：4KB
 *   块大小：64KB/128KB
 *   支持模式：Standard SPI、Dual SPI、Quad SPI
 */

/**
 * @brief  初始化SPI1和相关GPIO引脚
 * @note   使用SPI1主机模式，CPOL=Low, CPHA=1Edge（SPI模式0）
 * @param  无
 * @retval 无
 */
void Spiflash_Init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	SPI_InitTypeDef		SPI_InitStruct;
	
	// 使能GPIOB和SPI1时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	// 配置SPI引脚为复用功能模式
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// 配置CS引脚为普通输出模式
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// 配置引脚复用映射到SPI1
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
	
	// SPI参数配置
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;	// 波特率预分频
	SPI_InitStruct.SPI_CPHA		= SPI_CPHA_1Edge;						// 时钟相位：第一个边沿采样
	SPI_InitStruct.SPI_CPOL		= SPI_CPOL_Low;							// 时钟极性：空闲时低电平
	SPI_InitStruct.SPI_DataSize	= SPI_DataSize_8b;						// 数据宽度：8位
	SPI_InitStruct.SPI_Direction= SPI_Direction_2Lines_FullDuplex;		// 全双工模式
	SPI_InitStruct.SPI_FirstBit	= SPI_FirstBit_MSB;						// 高位在前
	SPI_InitStruct.SPI_Mode		= SPI_Mode_Master;						// 主机模式
	SPI_InitStruct.SPI_NSS		= SPI_NSS_Soft;							// 软件控制片选
	SPI_InitStruct.SPI_CRCPolynomial= 7;								// CRC多项式（未使用）
	
	// 初始化SPI1
	SPI_Init(SPI1, &SPI_InitStruct);
	
	// 使能SPI1
	SPI_Cmd(SPI1, ENABLE);
	
	// 初始状态：CS拉高，不选中芯片
	F_CS = 1;
}

/**
 * @brief  SPI1发送并接收一个字节（全双工）
 * @note   轮询方式发送和接收数据
 * @param  txdata: 要发送的字节
 * @retval 接收到的字节
 */
u8 Spi1_Send_Recv_Byte(u8 txdata)
{
	u8 rxdata = 0x00;
	
	// 等待发送缓冲区为空
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	
	// 将数据写入发送缓冲区
	SPI_I2S_SendData(SPI1, txdata);
	
	// 等待接收缓冲区非空
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	
	// 读取接收缓冲区数据
	rxdata = SPI_I2S_ReceiveData(SPI1);
	
	return rxdata;
}

/**
 * @brief  读取W25Q128的JEDEC ID（制造商ID和设备ID）
 * @note   使用命令0x90，地址0x000000
 * @param  无
 * @retval 16位ID：高8位为制造商ID，低8位为设备ID
 *         正常返回：0xEF18（Winbond + W25Q128）
 */
u16 W25q128_Id(void)
{
	u16 id = 0x00;
	
	// 拉低CS，选中芯片
	F_CS = 0;
	
	// 发送读ID命令 0x90
	Spi1_Send_Recv_Byte(0x90);
	
	// 发送地址 0x000000（24位地址，分3次发送）
	Spi1_Send_Recv_Byte(0x00);
	Spi1_Send_Recv_Byte(0x00);
	Spi1_Send_Recv_Byte(0x00);
	
	// 接收制造商ID（高8位）和设备ID（低8位）
	id |= Spi1_Send_Recv_Byte(0xAA) << 8;
	id |= Spi1_Send_Recv_Byte(0x66);
	
	// 拉高CS，取消选中
	F_CS = 1;
	
	return id;
}

/**
 * @brief  发送写使能命令
 * @note   在执行页编程、扇区擦除、块擦除等写操作前必须调用此函数
 *         使用命令0x06
 * @param  无
 * @retval 无
 */
void Write_Enable(void)
{
	F_CS = 0;
	Spi1_Send_Recv_Byte(0x06);
	F_CS = 1;
}

/**
 * @brief  等待Flash操作完成（忙等待）
 * @note   通过读取状态寄存器1的BUSY位（Bit0）判断
 *         BUSY=1表示正在忙，BUSY=0表示操作完成
 *         使用命令0x05读状态寄存器
 * @param  无
 * @retval 无
 */
void Wait_Busy(void)
{
	F_CS = 0;
	
	// 发送读状态寄存器命令 0x05
	Spi1_Send_Recv_Byte(0x05);
	
	// 循环等待BUSY位变为0
	while((Spi1_Send_Recv_Byte(0xFF) & 0x01) == 0x01);
	
	F_CS = 1;
}

/**
 * @brief  擦除一个扇区（4KB）
 * @note   使用命令0x20，擦除时间约0.1~0.5秒
 *         擦除后扇区内所有字节变为0xFF
 *         执行前需先调用Write_Enable()
 * @param  addr: 扇区起始地址（24位）
 * @retval 无
 */
void Erase_Sector(u32 addr)
{
	// 发送写使能命令
	Write_Enable();
	
	F_CS = 0;
	
	// 发送扇区擦除命令 0x20
	Spi1_Send_Recv_Byte(0x20);
	
	// 发送24位地址（分3次，每次8位）
	Spi1_Send_Recv_Byte(addr >> 16);
	Spi1_Send_Recv_Byte(addr >> 8);
	Spi1_Send_Recv_Byte(addr);
	
	F_CS = 1;
	
	// 等待擦除完成
	Wait_Busy();
}

/**
 * @brief  页编程（写入一页数据）
 * @note   使用命令0x02，单次写入不能超过256字节（一页）
 *         如果写入数据跨越页边界，超出部分会覆盖到页起始位置
 *         写入前必须先擦除对应扇区，且执行前需调用Write_Enable()
 * @param  addr: 写入起始地址（24位）
 * @param  write_buff: 待写入数据缓冲区指针
 * @param  len: 写入字节数（最大256）
 * @retval 无
 */
void Page_Write(u32 addr, u8 *write_buff, u32 len)
{
	// 发送写使能命令
	Write_Enable();
	
	F_CS = 0;
	
	// 发送页编程命令 0x02
	Spi1_Send_Recv_Byte(0x02);
	
	// 发送24位地址
	Spi1_Send_Recv_Byte(addr >> 16);
	Spi1_Send_Recv_Byte(addr >> 8);
	Spi1_Send_Recv_Byte(addr);
	
	// 逐字节写入数据
	while(len--)
	{
		Spi1_Send_Recv_Byte(*write_buff);
		write_buff++;
	}
	
	F_CS = 1;
	
	// 等待写入完成
	Wait_Busy();
}

/**
 * @brief  读取Flash数据（标准SPI模式）
 * @note   使用命令0x03，支持任意地址和长度的读取
 * @param  addr: 读取起始地址（24位）
 * @param  read_buff: 接收数据缓冲区指针
 * @param  len: 读取字节数
 * @retval 无
 */
void Read_Data(u32 addr, u8 *read_buff, u32 len)
{
	F_CS = 0;
	
	// 发送读数据命令 0x03
	Spi1_Send_Recv_Byte(0x03);
	
	// 发送24位地址
	Spi1_Send_Recv_Byte(addr >> 16);
	Spi1_Send_Recv_Byte(addr >> 8);
	Spi1_Send_Recv_Byte(addr);
	
	// 逐字节读取数据
	while(len--)
	{
		*read_buff = Spi1_Send_Recv_Byte(0x11);
		read_buff++;
	}
	
	F_CS = 1;
}

/**
 * @brief  跨页写入数据（自动处理页边界）
 * @note   W25Q128每页256字节，此函数自动将跨页的数据分成多次页写入
 *         写入前必须先擦除对应扇区
 * @param  addr: 写入起始地址（24位）
 * @param  write_buff: 待写入数据缓冲区指针
 * @param  len: 写入字节数（无限制）
 * @retval 无
 */
void Cross_Page_Write(u32 addr, u8 *write_buff, u32 len)
{
	// 计算当前地址到页边界的剩余字节数
	// addr & 0xFF 得到地址的低8位（页内偏移）
	// 256 - 页内偏移 = 当前页剩余空间
	u32 page_remaining = 256 - (addr & 0xFF);
	
	// 如果数据长度不超过当前页剩余空间，直接写入
	if(len <= page_remaining)
	{
		Page_Write(addr, write_buff, len);
		return;
	}
	
	// 先写满当前页
	Page_Write(addr, write_buff, page_remaining);
	
	// 更新地址、缓冲区指针和剩余长度
	len -= page_remaining;
	write_buff += page_remaining;
	addr += page_remaining;
	
	// 循环写入完整的256字节页面
	while(len >= 256)
	{
		Page_Write(addr, write_buff, 256);
		len -= 256;
		write_buff += 256;
		addr += 256;
	}
	
	// 写入最后不足一页的数据
	if(len > 0)
	{
		Page_Write(addr, write_buff, len);
	}
}

/**
 * @brief  将GPIO引脚切换回SPI外设模式
 * @note   Dual SPI操作完成后需要调用此函数恢复SPI功能
 *         将PB3/PB4/PB5重新配置为SPI1复用功能
 * @param  无
 * @retval 无
 */
static void GPIO_SPI_Mode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 配置PB3/PB4/PB5为复用功能模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// 配置引脚复用映射到SPI1
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
	
	// 使能SPI1外设
	SPI_Cmd(SPI1, ENABLE);
}

/**
 * @brief  将GPIO引脚配置为位操作输入模式（用于Dual SPI读取）
 * @note   STM32F407无硬件Dual SPI，需通过GPIO位操作模拟
 *         PB3(SCK) -> 输出模式，手动产生时钟
 *         PB4(MISO) -> 输入模式，读取IO0数据
 *         PB5(MOSI) -> 输入模式，读取IO1数据
 * @param  无
 * @retval 无
 */
static void GPIO_Bitbang_Input_Mode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 先禁用SPI1外设，避免冲突
	SPI_Cmd(SPI1, DISABLE);
	
	// 配置PB3(SCK)为推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// 配置PB4(MISO)和PB5(MOSI)为上拉输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// SCK初始化为低电平
	GPIOB->BSRRH = GPIO_Pin_3;
}

/**
 * @brief  将GPIO引脚配置为位操作输出模式（用于Dual SPI写入）
 * @note   PB3(SCK) -> 输出模式，手动产生时钟
 *         PB4(MISO) -> 输出模式，输出IO0数据
 *         PB5(MOSI) -> 输出模式，输出IO1数据
 * @param  无
 * @retval 无
 */
static void GPIO_Bitbang_Output_Mode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 先禁用SPI1外设，避免冲突
	SPI_Cmd(SPI1, DISABLE);
	
	// 配置PB3(SCK)为推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// 配置PB4和PB5为推挽输出（用于Dual SPI数据输出）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// SCK初始化为低电平
	GPIOB->BSRRH = GPIO_Pin_3;
}

/**
 * @brief  Dual SPI读取数据（双输出模式）
 * @note   使用命令0xBB（Dual Output Fast Read）
 *         命令和地址通过SPI外设发送，数据通过GPIO位操作读取
 *         每个时钟周期读取2位数据（IO0和IO1同时输出）
 *         不需要设置QE位
 * @param  addr: 读取起始地址（24位）
 * @param  read_buff: 接收数据缓冲区指针
 * @param  len: 读取字节数
 * @retval 无
 */
void Dual_SPI_Read(u32 addr, u8 *read_buff, u32 len)
{
	u32 i;
	u8 byte;
	
	F_CS = 0;
	
	// 发送Dual Output Fast Read命令 0xBB
	Spi1_Send_Recv_Byte(0xBB);
	
	// 发送24位地址
	Spi1_Send_Recv_Byte(addr >> 16);
	Spi1_Send_Recv_Byte(addr >> 8);
	Spi1_Send_Recv_Byte(addr);
	
	// 发送1个dummy字节（命令要求）
	Spi1_Send_Recv_Byte(0xFF);
	
	// 切换到GPIO位操作输入模式
	GPIO_Bitbang_Input_Mode();
	
	// 逐字节读取数据（每个字节需要4个时钟周期，每个周期读取2位）
	for(i = 0; i < len; i++)
	{
		byte = 0;
		
		// 第1个时钟：读取Bit7和Bit6
		GPIOB->BSRRL = GPIO_Pin_3;			// SCK置高
		GPIOB->BSRRH = GPIO_Pin_3;			// SCK拉低
		byte |= ((GPIOB->IDR & GPIO_Pin_4) ? 0x80 : 0x00);	// PB4 -> Bit7
		byte |= ((GPIOB->IDR & GPIO_Pin_5) ? 0x40 : 0x00);	// PB5 -> Bit6
		
		// 第2个时钟：读取Bit5和Bit4
		GPIOB->BSRRL = GPIO_Pin_3;
		GPIOB->BSRRH = GPIO_Pin_3;
		byte |= ((GPIOB->IDR & GPIO_Pin_4) ? 0x20 : 0x00);	// PB4 -> Bit5
		byte |= ((GPIOB->IDR & GPIO_Pin_5) ? 0x10 : 0x00);	// PB5 -> Bit4
		
		// 第3个时钟：读取Bit3和Bit2
		GPIOB->BSRRL = GPIO_Pin_3;
		GPIOB->BSRRH = GPIO_Pin_3;
		byte |= ((GPIOB->IDR & GPIO_Pin_4) ? 0x08 : 0x00);	// PB4 -> Bit3
		byte |= ((GPIOB->IDR & GPIO_Pin_5) ? 0x04 : 0x00);	// PB5 -> Bit2
		
		// 第4个时钟：读取Bit1和Bit0
		GPIOB->BSRRL = GPIO_Pin_3;
		GPIOB->BSRRH = GPIO_Pin_3;
		byte |= ((GPIOB->IDR & GPIO_Pin_4) ? 0x02 : 0x00);	// PB4 -> Bit1
		byte |= ((GPIOB->IDR & GPIO_Pin_5) ? 0x01 : 0x00);	// PB5 -> Bit0
		
		// 存储读取的字节
		read_buff[i] = byte;
	}
	
	F_CS = 1;
	
	// 恢复SPI外设模式
	GPIO_SPI_Mode();
}

/**
 * @brief  Dual SPI写入数据（双输入模式）
 * @note   使用命令0xA3（Dual Input Page Program）
 *         命令和地址通过SPI外设发送，数据通过GPIO位操作写入
 *         每个时钟周期写入2位数据（IO0和IO1同时输入）
 *         单次写入不能超过256字节（一页限制）
 *         不需要设置QE位
 * @param  addr: 写入起始地址（24位）
 * @param  write_buff: 待写入数据缓冲区指针
 * @param  len: 写入字节数（最大256）
 * @retval 无
 */
void Dual_SPI_Write(u32 addr, u8 *write_buff, u32 len)
{
	u32 i;
	u8 byte;
	
	// 发送写使能命令
	Write_Enable();
	
	F_CS = 0;
	
	// 发送Dual Input Page Program命令 0xA3
	Spi1_Send_Recv_Byte(0xA3);
	
	// 发送24位地址
	Spi1_Send_Recv_Byte(addr >> 16);
	Spi1_Send_Recv_Byte(addr >> 8);
	Spi1_Send_Recv_Byte(addr);
	
	// 切换到GPIO位操作输出模式
	GPIO_Bitbang_Output_Mode();
	
	// 逐字节写入数据（每个字节需要4个时钟周期，每个周期写入2位）
	for(i = 0; i < len; i++)
	{
		byte = write_buff[i];
		
		// 第1个时钟：写入Bit7和Bit6
		if(byte & 0x80) GPIOB->BSRRL = GPIO_Pin_4; else GPIOB->BSRRH = GPIO_Pin_4;	// PB4输出Bit7
		if(byte & 0x40) GPIOB->BSRRL = GPIO_Pin_5; else GPIOB->BSRRH = GPIO_Pin_5;	// PB5输出Bit6
		GPIOB->BSRRL = GPIO_Pin_3;		// SCK置高
		GPIOB->BSRRH = GPIO_Pin_3;		// SCK拉低
		
		// 第2个时钟：写入Bit5和Bit4
		if(byte & 0x20) GPIOB->BSRRL = GPIO_Pin_4; else GPIOB->BSRRH = GPIO_Pin_4;	// PB4输出Bit5
		if(byte & 0x10) GPIOB->BSRRL = GPIO_Pin_5; else GPIOB->BSRRH = GPIO_Pin_5;	// PB5输出Bit4
		GPIOB->BSRRL = GPIO_Pin_3;
		GPIOB->BSRRH = GPIO_Pin_3;
		
		// 第3个时钟：写入Bit3和Bit2
		if(byte & 0x08) GPIOB->BSRRL = GPIO_Pin_4; else GPIOB->BSRRH = GPIO_Pin_4;	// PB4输出Bit3
		if(byte & 0x04) GPIOB->BSRRL = GPIO_Pin_5; else GPIOB->BSRRH = GPIO_Pin_5;	// PB5输出Bit2
		GPIOB->BSRRL = GPIO_Pin_3;
		GPIOB->BSRRH = GPIO_Pin_3;
		
		// 第4个时钟：写入Bit1和Bit0
		if(byte & 0x02) GPIOB->BSRRL = GPIO_Pin_4; else GPIOB->BSRRH = GPIO_Pin_4;	// PB4输出Bit1
		if(byte & 0x01) GPIOB->BSRRL = GPIO_Pin_5; else GPIOB->BSRRH = GPIO_Pin_5;	// PB5输出Bit0
		GPIOB->BSRRL = GPIO_Pin_3;
		GPIOB->BSRRH = GPIO_Pin_3;
	}
	
	F_CS = 1;
	
	// 恢复SPI外设模式
	GPIO_SPI_Mode();
	
	// 等待写入完成
	Wait_Busy();
}