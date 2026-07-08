#include "tft.h"
#include "touch.h"  
#include "stm32f4xx.h"
#include "delay.h"
#include "stdlib.h"

uint32_t g_lcd_width =LCD_WIDTH;
uint32_t g_lcd_height=LCD_HEIGHT;
uint32_t g_lcd_direction=0;



// SPI发送一个字节（支持软件SPI和硬件SPI两种模式）
// byte: 要发送的字节数据
void spi1_send_byte(uint8_t byte)
{
#if LCD_SOFT_SPI_ENABLE
  unsigned char counter;

  for (counter = 0; counter < 8; counter++)
  {
    SPI_SCK_0;
    if ((byte & 0x80) == 0)
    {
      SPI_SDA_0;
    }
    else
      SPI_SDA_1;
    byte = byte << 1;
    SPI_SCK_1;
  }
  SPI_SCK_0;	

#else
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
		;
	SPI_I2S_SendData(SPI1, byte);

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
		;
	SPI_I2S_ReceiveData(SPI1);	

#endif

}

// 通过DMA发送缓冲区数据到SPI1（轮询方式等待传输完成）
// buf: 数据缓冲区指针
// len: 要发送的数据长度（字节）
void spi1_dma_send_buffer(const uint8_t *buf, uint32_t len)
{
	if (len == 0) return;
	
	// 等待DMA流完全停止
	while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE);
	
	// 清除所有DMA标志位
	DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_TCIF3 | DMA_FLAG_HTIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_FEIF3);
	
	// 设置DMA传输长度
	DMA_SetCurrDataCounter(DMA2_Stream3, len);
	// 设置DMA内存地址（直接操作寄存器）
	DMA2_Stream3->M0AR = (uint32_t)buf;
	
	// 启用SPI的TX DMA请求
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
	
	// 启动DMA传输
	DMA_Cmd(DMA2_Stream3, ENABLE);
	
	// 轮询等待DMA传输完成
	while (DMA_GetFlagStatus(DMA2_Stream3, DMA_FLAG_TCIF3) == RESET);
	
	// 清除传输完成标志
	DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_TCIF3);
	
	// 等待SPI发送完成
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	
	// 禁用SPI的TX DMA请求和DMA通道
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
	DMA_Cmd(DMA2_Stream3, DISABLE);
	
	// 读取接收缓冲区，清除溢出标志
	if (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == SET)
	{
		SPI_I2S_ReceiveData(SPI1);
	}
}

// 向LCD发送命令
// cmd: 命令字节
void lcd_send_cmd(uint8_t cmd)
{
	SPI_CS_0;
	LCD_DC_0; //命令模式
	spi1_send_byte(cmd);
	SPI_CS_1;
}

// 向LCD发送数据
// dat: 数据字节
void lcd_send_data(uint8_t dat)
{
	SPI_CS_0;
	LCD_DC_1; //数据模式
	spi1_send_byte(dat);
	SPI_CS_1;
}

// 设置LCD显示区域（矩形区域）
// x_s, y_s: 起始坐标
// x_e, y_e: 结束坐标
void lcd_addr_set(uint32_t x_s, uint32_t y_s, uint32_t x_e, uint32_t y_e)
{
	// 根据屏幕方向调整偏移量
	if(lcd_get_direction()==0 || lcd_get_direction()==2)
	{
		y_s=y_s+Y_OFFSET;
		y_e=y_e+Y_OFFSET;		
	}
	
	if(lcd_get_direction()==1 || lcd_get_direction()==3)
	{
		x_s=x_s+X_OFFSET;
		x_e=x_e+X_OFFSET;	
		
	}		

	// 设置列地址范围
	lcd_send_cmd(0x2a);	 		// 列地址设置命令
	lcd_send_data(x_s>> 8); 	// 起始列地址高8位
	lcd_send_data(x_s);			// 起始列地址低8位
	lcd_send_data(x_e >> 8); 	// 结束列地址高8位
	lcd_send_data(x_e);			// 结束列地址低8位

	// 设置行地址范围
	lcd_send_cmd(0x2b);	 		// 行地址设置命令
	lcd_send_data(y_s>> 8); 	// 起始行地址高8位
	lcd_send_data(y_s);			// 起始行地址低8位
	lcd_send_data(y_e >> 8); 	// 结束行地址高8位
	lcd_send_data(y_e);			// 结束行地址低8位
	
	lcd_send_cmd(0x2C); 		// 写显存命令（之后发送的数据将写入显存）
}

// 填充矩形区域（使用DMA传输，分块发送）
// x_s, y_s: 起始坐标
// x_len, y_len: 区域宽度和高度
// color: 填充颜色（RGB565格式）
void lcd_fill(uint32_t x_s, uint32_t y_s, uint32_t x_len, uint32_t y_len,uint32_t color)
{
	uint32_t total_pixels = x_len * y_len;			// 总像素数
	uint32_t total_bytes = total_pixels * 2;		// 总字节数（每个像素2字节）
	uint32_t block_size = 65534;					// 分块大小（65534是偶数，保持像素对齐）
	uint32_t remaining_len = total_bytes;			// 剩余字节数
	
	uint8_t fill_buf[2];							// 颜色缓冲区
	fill_buf[0] = color >> 8;						// 颜色高字节
	fill_buf[1] = color;							// 颜色低字节
	
	// 设置填充区域
	lcd_addr_set(x_s, y_s, x_s + x_len - 1, y_s + y_len - 1);
	
	SPI_CS_0;										// 拉低CS，选中LCD
	LCD_DC_1;										// 设置为数据模式
	
	// 分块发送数据
	while (remaining_len > 0)
	{
		// 计算当前块大小（不超过剩余长度）
		uint32_t current_len = (remaining_len > block_size) ? block_size : remaining_len;
		
		uint32_t repeat_count = current_len / 2;	// 需要重复的颜色次数
		static uint8_t dma_buf[65534];				// 静态缓冲区（避免动态内存分配失败）
		
		// 填充颜色数据到缓冲区
		for (uint32_t i = 0; i < repeat_count; i++)
		{
			dma_buf[i * 2] = fill_buf[0];
			dma_buf[i * 2 + 1] = fill_buf[1];
		}
		
		// 使用DMA发送当前块数据
		spi1_dma_send_buffer(dma_buf, current_len);
		
		// 更新剩余长度
		remaining_len -= current_len;
	}
	
	SPI_CS_1;										// 拉高CS，取消选中
}

// 清屏（填充整个屏幕）
// color: 清屏颜色（RGB565格式）
void lcd_clear(uint32_t color)
{
	lcd_fill(0,0,g_lcd_width,g_lcd_height,color);
}

// 显示图片（使用DMA传输，分块发送）
// x_s, y_s: 显示起始坐标
// width, height: 图片宽度和高度
// pic: 图片数据指针（RGB565格式，每个像素2字节）
void lcd_draw_picture(uint32_t x_s, uint32_t y_s, uint32_t width, uint32_t height, const uint8_t *pic)
{
	uint32_t total_len = width * height * 2;		// 总字节数
	uint32_t block_size = 65534;					// 分块大小（保持像素对齐）
	uint32_t remaining_len = total_len;			// 剩余字节数
	uint32_t offset = 0;							// 当前偏移量
	
	// 设置显示区域
	lcd_addr_set(x_s, y_s, x_s + width - 1, y_s + height - 1);
	
	SPI_CS_0;										// 拉低CS，选中LCD
	LCD_DC_1;										// 设置为数据模式
	
	// 分块发送图片数据
	while (remaining_len > 0)
	{
		// 计算当前块大小
		uint32_t current_len = (remaining_len > block_size) ? block_size : remaining_len;
		
		// 使用DMA发送当前块数据
		spi1_dma_send_buffer(pic + offset, current_len);
		
		// 更新偏移量和剩余长度
		offset += current_len;
		remaining_len -= current_len;
	}
	
	SPI_CS_1;										// 拉高CS，取消选中
	
	// 等待SPI发送完成
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
	
	// 读取接收缓冲区，清除溢出标志
	if (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == SET)
	{
		SPI_I2S_ReceiveData(SPI1);
	}
}

// 显示图片（先清屏再显示，避免旧图片残留）
// x_s, y_s: 显示起始坐标
// width, height: 图片宽度和高度
// pic: 图片数据指针
// bg_color: 背景颜色（清屏颜色）
void lcd_draw_picture_ex(uint32_t x_s, uint32_t y_s, uint32_t width, uint32_t height, const uint8_t *pic, uint32_t bg_color)
{
	lcd_clear(bg_color);							// 先清屏
	lcd_draw_picture(x_s, y_s, width, height, pic);	// 再显示图片
}

// 显示图片（先清屏再显示，功能与lcd_draw_picture_ex相同）
// x_s, y_s: 显示起始坐标
// width, height: 图片宽度和高度
// pic: 图片数据指针
// bg_color: 背景颜色（清屏颜色）
void lcd_draw_picture_clear_bg(uint32_t x_s, uint32_t y_s, uint32_t width, uint32_t height, const uint8_t *pic, uint32_t bg_color)
{
	lcd_fill(0, 0, g_lcd_width, g_lcd_height, bg_color);
	lcd_draw_picture(x_s, y_s, width, height, pic);
}

// 显示图片（DMA方式，与lcd_draw_picture功能相同）
// x_s, y_s: 显示起始坐标
// width, height: 图片宽度和高度
// pic: 图片数据指针
void lcd_draw_picture_dma(uint32_t x_s, uint32_t y_s, uint32_t width, uint32_t height, const uint8_t *pic)
{
	lcd_draw_picture(x_s, y_s, width, height, pic);
}

// 显示ASCII字符（支持DMA传输和点绘制两种模式）
// x, y: 显示起始坐标
// ch: 要显示的字符（ASCII码）
// fc: 前景色
// bc: 背景色
// font_size: 字体大小（12/16/24/32）
// mode: 显示模式（0=非叠加模式，1=叠加模式）
void lcd_show_char(uint32_t x, uint32_t y,uint8_t ch,uint32_t fc,uint32_t bc,uint32_t font_size,uint32_t mode)
{
	u8 temp,sizex,t,m=0;
	u16 i,TypefaceNum;
	
	u16 x0=x;
	
	sizex=font_size/2;								// 字符宽度（字体高度的一半）
	
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*font_size;	// 字符点阵数据长度
	
	ch=ch-' ';										// 计算字符在字库中的偏移
	
	lcd_addr_set(x,y,x+sizex-1,y+font_size-1);		// 设置字符显示区域
	
	if(!mode)										// 非叠加模式（使用DMA传输）
	{
		uint32_t total_pixels = sizex * font_size;	// 总像素数
		uint32_t total_bytes = total_pixels * 2;	// 总字节数
		uint8_t *dma_buf = (uint8_t *)malloc(total_bytes);
		if (dma_buf == NULL) return;
		
		uint32_t buf_idx = 0;
		
		// 生成字符点阵数据
		for(i=0;i<TypefaceNum;i++)
		{
			// 根据字体大小选择字库
			if(font_size==12)temp=ascii_1206[ch][i];
			else if(font_size==16)temp=ascii_1608[ch][i];
			else if(font_size==24)temp=ascii_2412[ch][i];
			else if(font_size==32)temp=ascii_3216[ch][i];
			else { free(dma_buf); return; }
			
			// 逐位处理点阵数据
			for(t=0;t<8;t++)
			{
				if(temp&(0x01<<t))					// 该位为1，显示前景色
				{
					dma_buf[buf_idx++] = fc >> 8;
					dma_buf[buf_idx++] = fc;
				}
				else 								// 该位为0，显示背景色
				{
					dma_buf[buf_idx++] = bc >> 8;
					dma_buf[buf_idx++] = bc;
				}
				m++;
				if(m%sizex==0)						// 一行结束
				{
					m=0;
					break;
				}
			}
		}
		
		// 使用DMA发送字符数据
		SPI_CS_0;
		LCD_DC_1;
		spi1_dma_send_buffer(dma_buf, total_bytes);
		SPI_CS_1;
		
		free(dma_buf);
	}
	else											// 叠加模式（逐点绘制）
	{
		for(i=0;i<TypefaceNum;i++)
		{
			if(font_size==12)temp=ascii_1206[ch][i];
			else if(font_size==16)temp=ascii_1608[ch][i];
			else if(font_size==24)temp=ascii_2412[ch][i];
			else if(font_size==32)temp=ascii_3216[ch][i];
			else return;
			
			for(t=0;t<8;t++)
			{
				if(temp&(0x01<<t))lcd_draw_point(x,y,fc);	// 该位为1，绘制前景色点
				x++;
				if((x-x0)==sizex)							// 一行结束
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}
}

/******************************************************************************
*函数说明：显示字符串
*参数：
u16 x:显示字符串的起始x坐标
u16 y:显示字符串的起始y坐标
const u8 *p:字符串的地址
u16 fc:字符串前景色
u16 bc:字符串背景色
u8 font_size:字体大小
u8 mode:是否叠加模式(是否需要填充背景色)

*返回值：无
******************************************************************************/

void lcd_show_string(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 font_size,u8 mode)
{         
	while(*p!='\0')									// 遍历字符串直到结束
	{       
		lcd_show_char(x,y,*p,fc,bc,font_size,mode);	// 显示单个字符
		x+=font_size/2;								// 移动到下一个字符位置
		p++;
	}  
}

/******************************************************************************
*函数说明：计算幂次方
*参数：m为底数，n为指数
*返回值：m^n的结果
******************************************************************************/
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;
	return result;
}


/******************************************************************************
*函数说明：显示整数
*参数：x,y为显示坐标
	num 要显示的整数
	len 要显示的位数
	fc 数字颜色
	bc 背景颜色
	font_size 字体大小
*返回值：无
******************************************************************************/
void lcd_show_integer(uint32_t x,uint32_t y,uint32_t num,uint32_t len,uint32_t fc,uint32_t bc,uint32_t font_size)
{         	
	u8 t,temp;
	u8 enshow=0;									// 是否开始显示有效数字
	u8 sizex=font_size/2;							// 字符宽度
	
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;			// 提取当前位数字
		
		if(enshow==0&&t<(len-1))					// 还没开始显示且不是最后一位
		{
			if(temp==0)								// 当前位是0，显示空格
			{
				lcd_show_char(x+t*sizex,y,' ',fc,bc,font_size,0);
				continue;
			}else enshow=1; 						// 遇到非零数字，开始显示
		 	 
		}
	 	lcd_show_char(x+t*sizex,y,temp+48,fc,bc,font_size,0);	// 显示数字字符
	}
} 


/******************************************************************************
*函数说明：显示两位小数
*参数：x,y为显示坐标
		num 要显示的小数
		len 要显示的位数
		fc 数字颜色
		bc 背景颜色
		font_size 字体大小
*返回值：无
******************************************************************************/
void lcd_show_float(uint32_t x,uint32_t y,float num,uint32_t len,uint32_t fc,uint32_t bc,uint32_t font_size)
{         	
	uint32_t t,temp,sizex;
	uint32_t num1;
	sizex=font_size/2;								// 字符宽度
	num1=num*100;									// 转换为整数处理
	
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;			// 提取当前位数字
		
		if(t==(len-2))								// 在小数点位置显示小数点
		{
			lcd_show_char(x+(len-2)*sizex,y,'.',fc,bc,font_size,0);
			t++;
			len+=1;
		}
	 	lcd_show_char(x+t*sizex,y,temp+48,fc,bc,font_size,0);	// 显示数字字符
	}
}



// LCD初始化（基于ST7789V2芯片）
void lcd_init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	SPI_InitTypeDef  	SPI_InitStructure;
	
#if LCD_SOFT_SPI_ENABLE
	// 软件SPI模式：启用GPIOD和GPIOE时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);

	// 背光引脚配置（PD9）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // 输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 速度设置为100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 不使用上下拉电阻
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	// SPI引脚配置（PE7/9/11/13/15）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7| GPIO_Pin_9|GPIO_Pin_11|GPIO_Pin_13|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // 输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 速度设置为100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 不使用上下拉电阻
	GPIO_Init(GPIOE, &GPIO_InitStructure);

#else
	// 硬件SPI模式：启用GPIOB、GPIOG和SPI1时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	// SPI1引脚配置：SCK=PB3, MOSI=PB5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		 // 复用功能模式
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;   // 高速模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 // 推挽输出模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 不使用上下拉电阻
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// SPI1 CS引脚配置：PB4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		 // 输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;   // 高速模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 // 推挽输出模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 不使用上下拉电阻
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// LCD控制引脚配置：DC=PG6, RST=PG7, BLK=PG8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		 // 输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;   // 高速模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 // 推挽输出模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 不使用上下拉电阻
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	// 将PB3和PB5连接到SPI1硬件
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

	// 关闭SPI1，准备重新配置
	SPI_Cmd(SPI1, DISABLE);

	// 初始化SPI1配置
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // 全双工模式
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;					   // 主机模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;				   // 8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;						   // 时钟空闲时为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;					   // 第二个时钟边沿采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;						   // CS由软件控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; // 预分频为2分频
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   // 高位先发送
	SPI_InitStructure.SPI_CRCPolynomial = 0;						   // 不使用CRC
	SPI_Init(SPI1, &SPI_InitStructure);								   // 初始化SPI1

	// 开启SPI1
	SPI_Cmd(SPI1, ENABLE);
	
	// 初始化SPI1 TX DMA通道
	spi1_tx_dma_init();
	
#endif

	// 将1.69英寸屏幕背光设为高电平开启
	LCD_BLK_1;
	
	SPI_SCK_0;

	SPI_SCK_1; // 关闭时钟信号
	
	// 硬件复位LCD
	LCD_RST_0;
	delay_ms(100);
	LCD_RST_1;
	delay_ms(100);
	
	// 退出睡眠模式
	lcd_send_cmd(0x11); // Sleep Out
	delay_ms(120);		// 延时120ms等待LCD稳定
	
	// 设置颜色模式为65K色
	lcd_send_cmd(0x3A); // Interface Pixel Format
	lcd_send_data(0x05);
	
	// 设置VCOM电压
	lcd_send_cmd(0xC5); // VCOM Setting
	lcd_send_data(0x1A);
	
	// 设置屏幕显示方向
	lcd_send_cmd(0x36); // Memory Data Access Control
	lcd_send_data(0x00);
	
	// 设置Porch参数
	lcd_send_cmd(0xb2); // Porch Setting
	lcd_send_data(0x05);
	lcd_send_data(0x05);
	lcd_send_data(0x00);
	lcd_send_data(0x33);
	lcd_send_data(0x33);

	// 设置Gate控制
	lcd_send_cmd(0xb7);	 // Gate Control
	lcd_send_data(0x05); // 12.2v   -10.43v
	
	// 设置VCOM电压
	lcd_send_cmd(0xBB); // VCOM Setting
	lcd_send_data(0x3F);

	// 设置电源控制1
	lcd_send_cmd(0xC0); // Power Control 1
	lcd_send_data(0x2c);

	// 启用VDV和VRH命令
	lcd_send_cmd(0xC2); // VDV and VRH Command Enable
	lcd_send_data(0x01);

	// 设置VRH值
	lcd_send_cmd(0xC3);	 // VRH Set
	lcd_send_data(0x0F); // 4.3+( vcom+vcom offset+vdv)

	// 设置VDV值
	lcd_send_cmd(0xC4);	 // VDV Set
	lcd_send_data(0x20); // 0v

	// 设置帧率控制
	lcd_send_cmd(0xC6);	 // Frame Rate Control in Normal Mode
	lcd_send_data(0X01); // 111Hz

	// 设置电源控制1
	lcd_send_cmd(0xd0); // Power Control 1
	lcd_send_data(0xa4);
	lcd_send_data(0xa1);

	// 设置电源控制2
	lcd_send_cmd(0xE8); // Power Control 2
	lcd_send_data(0x03);

	// 设置均衡时间控制
	lcd_send_cmd(0xE9); // Equalize Time Control
	lcd_send_data(0x09);
	lcd_send_data(0x09);
	lcd_send_data(0x08);
	
	// 设置正gamma校正
	lcd_send_cmd(0xE0); // Set Gamma
	lcd_send_data(0xD0);
	lcd_send_data(0x05);
	lcd_send_data(0x09);
	lcd_send_data(0x09);
	lcd_send_data(0x08);
	lcd_send_data(0x14);
	lcd_send_data(0x28);
	lcd_send_data(0x33);
	lcd_send_data(0x3F);
	lcd_send_data(0x07);
	lcd_send_data(0x13);
	lcd_send_data(0x14);
	lcd_send_data(0x28);
	lcd_send_data(0x30);

	// 设置负gamma校正
	lcd_send_cmd(0XE1); // Set Gamma
	lcd_send_data(0xD0);
	lcd_send_data(0x05);
	lcd_send_data(0x09);
	lcd_send_data(0x09);
	lcd_send_data(0x08);
	lcd_send_data(0x03);
	lcd_send_data(0x24);
	lcd_send_data(0x32);
	lcd_send_data(0x32);
	lcd_send_data(0x3B);
	lcd_send_data(0x14);
	lcd_send_data(0x13);
	lcd_send_data(0x28);
	lcd_send_data(0x2F);

	lcd_send_cmd(0x21); // 反色设置

	lcd_send_cmd(0x29); // 开启显示
	
}

// 显示中文字符（使用DMA传输）
// x, y: 显示起始坐标
// no: 中文字符在字库中的编号
// fc: 前景色
// bc: 背景色
// font_size: 字体大小（16/24/32）
void lcd_show_chn(uint32_t x, uint32_t y,uint8_t no, uint32_t fc, uint32_t bc,uint32_t font_size)
{
	uint32_t i,j;
	uint8_t tmp;
	
	uint32_t total_pixels = font_size * font_size;	// 总像素数（中文字符是正方形）
	uint32_t total_bytes = total_pixels * 2;			// 总字节数
	uint8_t *dma_buf = (uint8_t *)malloc(total_bytes);
	if (dma_buf == NULL) return;
	
	uint32_t buf_idx = 0;

	// 设置中文字符显示区域
	lcd_addr_set(x, y, x + font_size-1, y + font_size-1);

	// 生成中文字符点阵数据
	for (i = 0; i < (font_size*font_size/8); i++)
	{
		// 根据字体大小选择字库
		if(font_size==16)tmp = chinese_tbl_16[no][i];
		if(font_size==24)tmp = chinese_tbl_24[no][i];	
		if(font_size==32)tmp = chinese_tbl_32[no][i];
		
		// 逐位处理点阵数据
		for (j = 0;j < 8; j++)
		{
			if (tmp & (1<<j))						// 该位为1，显示前景色
			{
				dma_buf[buf_idx++] = fc >> 8;
				dma_buf[buf_idx++] = fc;
			}
			
			else									// 该位为0，显示背景色
			{
				dma_buf[buf_idx++] = bc >> 8;
				dma_buf[buf_idx++] = bc;
			}
		}
	}
	
	// 使用DMA发送中文字符数据
	SPI_CS_0;
	LCD_DC_1;
	spi1_dma_send_buffer(dma_buf, total_bytes);
	SPI_CS_1;
	
	free(dma_buf);
}

// 绘制单个点（使用DMA传输）
// x, y: 点的坐标
// color: 点的颜色
void lcd_draw_point(uint32_t x, uint32_t y, uint32_t color)
{
	lcd_addr_set(x, y, x, y);						// 设置显示区域为单个点
	
	uint8_t dma_buf[2];								// 颜色缓冲区
	dma_buf[0] = color >> 8;						// 颜色高字节
	dma_buf[1] = color;								// 颜色低字节
	
	SPI_CS_0;										// 拉低CS，选中LCD
	LCD_DC_1;										// 设置为数据模式
	spi1_dma_send_buffer(dma_buf, 2);				// 使用DMA发送点数据
	SPI_CS_1;										// 拉高CS，取消选中
}


// 绘制直线（使用Bresenham算法）
// x1, y1: 起点坐标
// x2, y2: 终点坐标
// color: 线的颜色
void lcd_draw_line(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	
	delta_x=x2-x1; 									// 计算x坐标差值 
	delta_y=y2-y1;									// 计算y坐标差值
	uRow=x1;											// 起点x坐标
	uCol=y1;											// 起点y坐标
	
	if(delta_x>0)incx=1; 							// 设置x步进方向为正
	else if (delta_x==0)incx=0;						// 垂直线（x不变）
	else {incx=-1;delta_x=-delta_x;}				// x步进方向为负
	
	if(delta_y>0)incy=1;							// 设置y步进方向为正
	else if (delta_y==0)incy=0;						// 水平线（y不变）
	else {incy=-1;delta_y=-delta_y;}				// y步进方向为负
	
	if(delta_x>delta_y)distance=delta_x; 			// 选取最长的距离作为循环次数
	else distance=delta_y;
	
	for(t=0;t<distance+1;t++)
	{
		lcd_draw_point(uRow,uCol,color);			// 绘制当前点
		xerr+=delta_x;								// 累加x误差
		yerr+=delta_y;								// 累加y误差
		
		if(xerr>distance)							// x误差超过距离，移动x坐标
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)							// y误差超过距离，移动y坐标
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

// 绘制矩形（通过四条直线实现）
// x1, y1: 左上角坐标
// x2, y2: 右下角坐标
// color: 矩形边框颜色
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	lcd_draw_line(x1,y1,x2,y1,color);				// 上边
	lcd_draw_line(x1,y1,x1,y2,color);				// 左边
	lcd_draw_line(x1,y2,x2,y2,color);				// 下边
	lcd_draw_line(x2,y1,x2,y2,color);				// 右边
}

// 绘制圆形（使用中点画圆算法）
// x0, y0: 圆心坐标
// r: 圆的半径
// color: 圆的颜色
void lcd_draw_circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color)
{
	int a,b;
	
	a=0;b=r;											// a从0开始，b从半径开始
	
	while(a<=b)
	{
		// 绘制圆上8个对称点
		lcd_draw_point(x0-b,y0-a,color);             // 第3象限
		lcd_draw_point(x0+b,y0-a,color);             // 第0象限
		lcd_draw_point(x0-a,y0+b,color);             // 第1象限
		lcd_draw_point(x0-a,y0-b,color);             // 第2象限
		lcd_draw_point(x0+b,y0+a,color);             // 第4象限
		lcd_draw_point(x0+a,y0-b,color);             // 第5象限
		lcd_draw_point(x0+a,y0+b,color);             // 第6象限
		lcd_draw_point(x0-b,y0+a,color);             // 第7象限
		
		a++;											// a递增
		if((a*a+b*b)>(r*r))							// 判断是否需要递减b
		{
			b--;
		}
	}
}


// 设置LCD显示方向（旋转屏幕）
// dir: 方向（0=0度, 1=90度, 2=180度, 3=270度）
void lcd_set_direction(uint32_t dir)
{
	g_lcd_direction = dir;
	
	// 0度方向
	if(dir==0)
	{
		lcd_send_cmd(0x36);
		lcd_send_data(0x00);
		g_lcd_width=LCD_WIDTH;
		g_lcd_height=LCD_HEIGHT;
	}
	
	// 90度方向
	if(dir==1)
	{
		lcd_send_cmd(0x36);
		lcd_send_data((1<<6)|(1<<5)|(1<<4));
		g_lcd_width=LCD_HEIGHT;
		g_lcd_height=LCD_WIDTH;
	
	}	
	
	// 180度方向
	if(dir==2)
	{
		lcd_send_cmd(0x36);
		lcd_send_data((1<<7)|(1<<6));
		g_lcd_width=LCD_WIDTH;
		g_lcd_height=LCD_HEIGHT;
	
	}
	
	// 270度方向
	if(dir==3)
	{
		lcd_send_cmd(0x36);
		lcd_send_data((1<<7)|(0<<6)|(1<<5)|(1<<4));
		g_lcd_width=LCD_HEIGHT;
		g_lcd_height=LCD_WIDTH;
	
	}		

}

// 获取当前LCD显示方向
// 返回值: 当前方向（0/1/2/3）
uint32_t lcd_get_direction(void)
{
	return g_lcd_direction;
}

// 初始化SPI1 TX DMA（配置DMA2_Stream3通道3）
// 配置为内存到外设模式，字节传输，单次模式
void spi1_tx_dma_init(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	// 启用DMA2时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	// 复位DMA2_Stream3
	DMA_DeInit(DMA2_Stream3);

	// 等待DMA2_Stream3完全停止
	while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE);

	// 配置DMA通道参数
	DMA_InitStructure.DMA_Channel = DMA_Channel_3;							// 使用通道3
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;		// 外设地址为SPI1数据寄存器
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;					// 内存到外设方向
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设地址不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// 内存地址递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	// 外设数据宽度为字节
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// 内存数据宽度为字节
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// 单次模式（非循环）
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						// 高优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;					// 禁用FIFO模式
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				// 内存单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		// 外设单次传输
	
	// 初始化DMA2_Stream3
	DMA_Init(DMA2_Stream3, &DMA_InitStructure);
}

// 启动DMA传输
void spi1_tx_dma_start(void)
{
	DMA_Cmd(DMA2_Stream3, ENABLE);
}

// 停止DMA传输
void spi1_tx_dma_stop(void)
{
	DMA_Cmd(DMA2_Stream3, DISABLE);
}
