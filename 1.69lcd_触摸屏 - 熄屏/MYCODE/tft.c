#include "tft.h"
#include "touch.h"  
#include "stm32f4xx.h"
#include "delay.h"

/* 全局变量说明： */
uint32_t g_lcd_width =LCD_WIDTH;      // LCD屏幕宽度，默认240像素
uint32_t g_lcd_height=LCD_HEIGHT;     // LCD屏幕高度，默认280像素
uint32_t g_lcd_direction=0;          // LCD显示方向：0=正常，1=90度，2=180度，3=270度

/**
 * @brief  向LCD发送一个字节数据
 * @note   支持两种模式：软件模拟SPI（LCD_SOFT_SPI_ENABLE=1）和硬件SPI（默认）
 * @param  byte: 要发送的字节数据
 * @retval None
 */
void spi1_send_byte(uint8_t byte)
{
#if LCD_SOFT_SPI_ENABLE
  // 如果开启软件模拟SPI，用GPIO模拟时序
  unsigned char counter;

  for (counter = 0; counter < 8; counter++)
  {
    SPI_SCK_0;                           // 时钟线拉低
    if ((byte & 0x80) == 0)              // 判断最高位是0还是1
    {
      SPI_SDA_0;                         // 数据为0，数据线拉低
    }
    else
      SPI_SDA_1;                         // 数据为1，数据线拉高
    byte = byte << 1;                    // 左移一位，准备发送下一位
    SPI_SCK_1;                           // 时钟线拉高，产生上升沿，发送数据
  }
  SPI_SCK_0;	                           // 发送完成，时钟线拉低
	
#else
  // 默认使用硬件SPI，速度更快更稳定
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
		;                                   // 等待发送缓冲区为空
	SPI_I2S_SendData(SPI1, byte);         // 发送数据

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
		;                                   // 等待接收完成（SPI是全双工，必须读走接收的数据）
	SPI_I2S_ReceiveData(SPI1);            // 读取接收寄存器，清空标志位
	
#endif

}

/**
 * @brief  向LCD发送一条命令
 * @note   LCD的命令和数据是通过DC引脚区分的，DC=0表示命令，DC=1表示数据
 * @param  cmd: 命令字节
 * @retval None
 */
void lcd_send_cmd(uint8_t cmd)
{
	SPI_CS_0;                             // 拉低片选，选中LCD芯片
	LCD_DC_0; //数据/命令控制，0表示命令
	spi1_send_byte(cmd);                  // 发送命令
	SPI_CS_1;                             // 拉高片选，结束通信
}

// 写一个8位数据
void lcd_send_data(uint8_t dat)
{
	
	SPI_CS_0;                             // 拉低片选，选中LCD芯片
	
	LCD_DC_1; //数据/命令控制，1表示数据
	spi1_send_byte(dat);                  // 发送数据
	SPI_CS_1;                             // 拉高片选，结束通信
}

/**
 * @brief  设置LCD的显示区域（矩形框）
 * @note   LCD屏幕实际显示区域和驱动IC的显存区域不完全对应，需要偏移量修正
 * @param  x_s: 起始X坐标
 * @param  y_s: 起始Y坐标
 * @param  x_e: 结束X坐标
 * @param  y_e: 结束Y坐标
 * @retval None
 */
void lcd_addr_set(uint32_t x_s, uint32_t y_s, uint32_t x_e, uint32_t y_e)
{
	/* TFT屏幕需要偏移量 */
	
	// 如果是正常方向(0)或180度方向(2)，Y坐标需要加偏移
	if(lcd_get_direction()==0 || lcd_get_direction()==2)
	{
		y_s=y_s+Y_OFFSET;
		y_e=y_e+Y_OFFSET;		
	}
	
	// 如果是90度(1)或270度(3)方向，X坐标需要加偏移
	if(lcd_get_direction()==1 || lcd_get_direction()==3)
	{
		x_s=x_s+X_OFFSET;
		x_e=x_e+X_OFFSET;	
		
	}		

	// 发送列地址设置命令(0x2a)
	lcd_send_cmd(0x2a);	 		
	lcd_send_data(x_s>> 8); 	// 起始列高8位
	lcd_send_data(x_s);      // 起始列低8位
	lcd_send_data(x_e >> 8); // 结束列高8位
	lcd_send_data(x_e);      // 结束列低8位

	// 发送行地址设置命令(0x2b)
	lcd_send_cmd(0x2b);	 		
	lcd_send_data(y_s>> 8); 	// 起始行高8位
	lcd_send_data(y_s);      // 起始行低8位
	lcd_send_data(y_e >> 8); // 结束行高8位
	lcd_send_data(y_e);      // 结束行低8位
	
	lcd_send_cmd(0x2C); 		// 发送写数据命令，之后就可以连续写入像素数据了
}

//填充矩形区域
// 参数说明：
// x_s: 起始X坐标（左上角）
// y_s: 起始Y坐标（左上角）
// x_len: 区域宽度（多少个像素）
// y_len: 区域高度（多少个像素）
// color: 填充颜色（RGB565格式，比如RED=0xF800，GREEN=0x07E0，BLUE=0x001F）
void lcd_fill(uint32_t x_s, uint32_t y_s, uint32_t x_len, uint32_t y_len,uint32_t color)
{
	uint32_t x, y;
	
	// 设置填充区域，终点坐标 = 起点坐标 + 长度 - 1
	// 比如起点(0,0)，宽度10，终点就是9（因为包含0到9共10个点）
	lcd_addr_set(x_s,y_s,x_s+x_len-1,y_s+y_len - 1);  

	// 外层循环：从上到下，遍历每一行
	for (y = y_s; y < y_s+y_len; y++) 
	{
		// 内层循环：从左到右，遍历每一列
		for (x = x_s; x < x_s+x_len; x++) 
		{
			lcd_send_data(color >> 8);   // 发送颜色高8位（RGB的高5位+绿的高3位）
			lcd_send_data(color);        // 发送颜色低8位（绿的低3位+蓝的低5位）
		}
	}
}

/**
 * @brief  清屏函数，用指定颜色填充整个屏幕
 * @param  color: 清屏颜色，如WHITE、BLACK、RED等
 * @retval None
 */
void lcd_clear(uint32_t color)
{
	lcd_fill(0,0,g_lcd_width,g_lcd_height,color);  // 填充整个屏幕区域
}

//显示图片
// 参数说明：
// x_s: 图片左上角X坐标
// y_s: 图片左上角Y坐标
// width: 图片宽度（像素）
// height: 图片高度（像素）
// pic: 图片数据指针（RGB565格式，每个像素占2个字节）
// 
// 图片数据格式说明：
// RGB565格式：用16位表示一个颜色，高5位是红色(R)，中间6位是绿色(G)，低5位是蓝色(B)
// 比如红色是0xF800，绿色是0x07E0，蓝色是0x001F，白色是0xFFFF，黑色是0x0000
// 图片数据总大小 = width * height * 2 字节
void lcd_draw_picture(uint32_t x_s, uint32_t y_s, uint32_t width, uint32_t height, const uint8_t *pic)
{
	const uint8_t *p = pic;  // 指向图片数据的指针，从第一个像素开始
	uint32_t i = 0;          // 循环计数器
	
	// 设置图片显示区域，告诉LCD要在哪个位置显示图片
	lcd_addr_set(x_s, y_s, x_s+width-1, y_s+height-1);  

	// 循环发送所有像素数据
	// 每个像素占2个字节，所以每次循环发送2个字节，i每次加2
	// 总共有 width*height 个像素，所以总字节数是 width*height*2
	for (i = 0; i <width*height*2; i += 2) 
	{
		lcd_send_data(p[i]);	      // 发送第一个字节（颜色高8位）
		lcd_send_data(p[i + 1]);   // 发送第二个字节（颜色低8位）
	}
}

//显示字符
// 参数说明：
// x: 字符左上角X坐标
// y: 字符左上角Y坐标
// ch: 要显示的字符（ASCII码，比如'A'、'1'、'@'等）
// fc: 前景色（字符颜色）
// bc: 背景色（字符后面的颜色）
// font_size: 字体大小（支持12/16/24/32号）
// mode: 显示模式（0=非叠加模式，需要画背景色；1=叠加模式，只画字符，不改变背景）
//
// 字模原理说明：
// 每个字符由一个点阵组成，比如16号字体是8x16的点阵（宽8像素，高16像素）
// 字模数据用字节存储，每个字节表示一行中的8个点
// 字节中的每一位代表一个像素：1=显示前景色，0=显示背景色
// 比如字节0b00010000表示中间那个点是亮的，其他是暗的
void lcd_show_char(uint32_t x, uint32_t y,uint8_t ch,uint32_t fc,uint32_t bc,uint32_t font_size,uint32_t mode)
{
	u8 temp,sizex,t,m=0;
	u16 i,TypefaceNum;						//一个字符所占的字节大小
	
	u16 x0=x;  // 保存起始X坐标，用于换行时恢复
	
	sizex=font_size/2; //字符宽度，字体大小的一半。比如16号字体宽度是8像素
	
	// 计算一个字符占用的字节数
	// (sizex/8 + 是否有余数) * font_size
	// 比如16号字体：(8/8+0)*16=16字节；24号字体：(12/8+1)*24=48字节
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*font_size;  
	
	ch=ch-' ';    						//获取偏移后的值，空格(' ')是第一个字符(索引0)
										//比如显示'A'，ASCII码是65，' '是32，所以65-32=33，就是第33个字符
	
	lcd_addr_set(x,y,x+sizex-1,y+font_size-1);  //设置字符显示区域
	
	// 外层循环：遍历字符的每一个字节（从上到下）
	for(i=0;i<TypefaceNum;i++)
	{ 
		// 根据字体大小选择对应的字模数组
		// ascii_1206: 6x12字体（宽6像素，高12像素）
		// ascii_1608: 8x16字体（宽8像素，高16像素）
		// ascii_2412: 12x24字体（宽12像素，高24像素）
		// ascii_3216: 16x32字体（宽16像素，高32像素）
		if(font_size==12)temp=ascii_1206[ch][i];		      //调用6x12字体
		else if(font_size==16)temp=ascii_1608[ch][i];		 //调用8x16字体
		else if(font_size==24)temp=ascii_2412[ch][i];		 //调用12x24字体
		else if(font_size==32)temp=ascii_3216[ch][i];		 //调用16x32字体
		else return;
		
		// 内层循环：遍历字节中的每一位（从低位到高位，对应从左到右）
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式，需要同时处理前景色和背景色
			{
				if(temp&(0x01<<t))  // 如果当前位是1，显示前景色（字符颜色）
				{
					lcd_send_data(fc>>8);
					lcd_send_data(fc);
				}
				else                 // 如果当前位是0，显示背景色（字符后面的颜色）
				{
					lcd_send_data(bc>>8);
					lcd_send_data(bc);
				}
				m++;              // 统计当前行已经显示了多少个点
				if(m%sizex==0)    // 一行显示完了（m是sizex的倍数），跳到下一行
				{
					m=0;          // 重置计数器
					break;        // 跳出内层循环，处理下一个字节
				}
			}
			else//叠加模式，只在1的位置画点，其他位置保持原来的颜色
			{
				if(temp&(0x01<<t))lcd_draw_point(x,y,fc);//当前位是1，画一个点
				x++;              // X坐标右移一位
				if((x-x0)==sizex) // 一行显示完了，跳到下一行
				{
					x=x0;         // 恢复X坐标到起始位置
					y++;          // Y坐标下移一位
					break;        // 跳出内层循环，处理下一个字节
				}
			}
		}
	}  
}

/******************************************************************************
*函数说明：显示字符串
*参数：
u16 x:显示字符串起始x
u16 y:显示字符串起始y
const u8 *p:字符串的地址
u16 fc:字符串的颜色
u16 bc:字符串的背景颜色
u8 font_size:字体大小
u8 mode:是否叠加模式(是否需要背景色)

*返回值： 无
******************************************************************************/

void lcd_show_string(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 font_size,u8 mode)
{         
	while(*p!='\0')              // 循环直到遇到字符串结束符
	{       
		lcd_show_char(x,y,*p,fc,bc,font_size,mode);  // 显示当前字符
		x+=font_size/2;           // 移动到下一个字符的位置
		p++;                      // 指针指向下一个字符
	}  
}

/******************************************************************************
*函数说明：求幂运算
*输入参数：m底数 n指数
*返回值： 无
******************************************************************************/
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;         // 循环n次，result = result * m
	return result;
}


/******************************************************************************
*函数说明：显示无符号整数
*输入参数：x,y显示位置
	num 要显示的无符号整数
	len 要显示的位数
	fc 数字颜色
	bc 背景颜色
	font_size 字号
*返回值： 无
******************************************************************************/
void lcd_show_integer(uint32_t x,uint32_t y,uint32_t num,uint32_t len,uint32_t fc,uint32_t bc,uint32_t font_size)
{         	
	u8 t,temp;
	u8 enshow=0;                // 是否开始显示有效数字（跳过前导0）
	u8 sizex=font_size/2;
	
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;  // 提取当前位的数字
		
		// 如果还没开始显示有效数字，且不是最后一位
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)                    // 如果当前位是0，显示空格
			{
				lcd_show_char(x+t*sizex,y,' ',fc,bc,font_size,0);
				continue;
			}else enshow=1;                // 遇到非0数字，开始显示
			 
		}
	 	lcd_show_char(x+t*sizex,y,temp+48,fc,bc,font_size,0);  // 显示数字字符(48是'0'的ASCII码)
	}
} 


/******************************************************************************
*函数说明：显示两位小数函数
*输入参数：x,y显示位置
		num 要显示小数的数值
		len 要显示的位数
		fc 数值颜色
		bc 背景颜色
		font_size 字号
*返回值： 无
******************************************************************************/
void lcd_show_float(uint32_t x,uint32_t y,float num,uint32_t len,uint32_t fc,uint32_t bc,uint32_t font_size)
{         	
	uint32_t t,temp,sizex;
	uint32_t num1;
	sizex=font_size/2;
	num1=num*100;               // 将小数转为整数，保留两位小数
	
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;  // 提取当前位数字
		
		if(t==(len-2))           // 在倒数第二位时插入小数点
		{
			lcd_show_char(x+(len-2)*sizex,y,'.',fc,bc,font_size,0);
			t++;
			len+=1;              // 长度加1，因为多了一个小数点
		}
	 	lcd_show_char(x+t*sizex,y,temp+48,fc,bc,font_size,0);
	}
}




/**
 * @brief  LCD初始化函数，配置硬件和发送初始化序列
 * @note   驱动芯片是ST7789V2，需要发送一系列命令来配置显示参数
 * @param  None
 * @retval None
 */
void lcd_init(void) ////ST7789V2
{
	
	GPIO_InitTypeDef	GPIO_InitStructure;
	SPI_InitTypeDef  	SPI_InitStructure;
	
#if LCD_SOFT_SPI_ENABLE
	// 如果使用软件模拟SPI，配置GPIOD和GPIOE
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // 输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 速度设置为高速
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 不使用上下拉电阻
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7| GPIO_Pin_9|GPIO_Pin_11|GPIO_Pin_13|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;      // 输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 速度设置为高速
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 不使用上下拉电阻
	GPIO_Init(GPIOE, &GPIO_InitStructure);

#else
	// 默认使用硬件SPI，配置GPIOB、GPIOG和SPI1
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  // 使能GPIOB时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);  // 使能GPIOG时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);   // 使能SPI1时钟

	// SCK=PB3, MOSI=PB5，配置为复用功能
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	 //复用功能模式
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed; //高速模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //无上下拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// PB4作为LCD背光控制，配置为普通输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	 //输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed; //高速模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //无上下拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// PG6=CS, PG7=DC, PG8=RST，配置为普通输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	 //输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed; //高速模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //无上下拉
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	// 将PB3、PB5映射到SPI1硬件功能
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

	//先关闭SPI1
	SPI_Cmd(SPI1, DISABLE);

	//配置SPI参数
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //全双工模式
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;					   //主设备模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;				   //8位数据帧
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;						   //空闲时时钟为高
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;					   //第二个时钟沿采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;						   //软件控制CS
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; //2分频，最高速
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   //高位先发送
	SPI_InitStructure.SPI_CRCPolynomial = 0;						   //不使用CRC校验
	SPI_Init(SPI1, &SPI_InitStructure);								   //初始化SPI1

	//启用SPI1
	SPI_Cmd(SPI1, ENABLE);
	
#endif

	//打开1.69英寸LCD背光（默认高电平点亮）
	LCD_BLK_1;
	
	SPI_SCK_0;

	SPI_SCK_1; // 复位操作！
	LCD_RST_0;                // 拉低复位引脚
	delay_ms(100);            // 等待100毫秒
	LCD_RST_1;                // 拉高复位引脚
	delay_ms(100);            // 等待100毫秒，让LCD稳定
	
	//退出睡眠模式
	lcd_send_cmd(0x11); // Sleep Out
	delay_ms(120);		// 必须等待120毫秒
	
	//-----------------------ST7789V帧率设置-----------------//
	lcd_send_cmd(0x3A); // 设置颜色模式为65K色
	lcd_send_data(0x05);
	lcd_send_cmd(0xC5); // 设置VCOM电压
	lcd_send_data(0x1A);
	lcd_send_cmd(0x36); // 设置LCD显示方向
	lcd_send_data(0x00);
	
	//-------------ST7789V帧速率设置-----------//
	lcd_send_cmd(0xb2); // Porch Setting（前后廊设置）
	lcd_send_data(0x05);
	lcd_send_data(0x05);
	lcd_send_data(0x00);
	lcd_send_data(0x33);
	lcd_send_data(0x33);

	lcd_send_cmd(0xb7);	 // Gate Control（栅极控制）
	lcd_send_data(0x05); // 12.2v   -10.43v
	
	//--------------ST7789V电源设置---------------//
	lcd_send_cmd(0xBB); // VCOM设置
	lcd_send_data(0x3F);

	lcd_send_cmd(0xC0); // Power control（电源控制）
	lcd_send_data(0x2c);

	lcd_send_cmd(0xC2); // VDV and VRH Command Enable
	lcd_send_data(0x01);

	lcd_send_cmd(0xC3);	 // VRH Set（电压参考设置）
	lcd_send_data(0x0F); // 4.3+( vcom+vcom offset+vdv)

	lcd_send_cmd(0xC4);	 // VDV Set（电压偏差设置）
	lcd_send_data(0x20); // 0v

	lcd_send_cmd(0xC6);	 // Frame Rate Control（帧率控制）
	lcd_send_data(0X01); // 111Hz

	lcd_send_cmd(0xd0); // Power Control 1（电源控制1）
	lcd_send_data(0xa4);
	lcd_send_data(0xa1);

	lcd_send_cmd(0xE8); // Power Control（电源控制）
	lcd_send_data(0x03);

	lcd_send_cmd(0xE9); // Equalize time control（均衡时间控制）
	lcd_send_data(0x09);
	lcd_send_data(0x09);
	lcd_send_data(0x08);
	
	//---------------ST7789V伽马校正设置-------------//
	lcd_send_cmd(0xE0); // Set Gamma（设置伽马曲线）
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

	lcd_send_cmd(0XE1); // Set Gamma（设置伽马曲线）
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

	lcd_send_cmd(0x21); // 反色显示（这里启用反色）

	lcd_send_cmd(0x29); // 开启显示
	
}

/**
 * @brief  显示中文字符
 * @note   使用字模表显示，需要在lcd_font.h中定义中文字模数组
 * @param  x: 起始X坐标
 * @param  y: 起始Y坐标
 * @param  no: 中文字符在字模表中的索引
 * @param  fc: 前景色
 * @param  bc: 背景色
 * @param  font_size: 字体大小（16/24/32）
 * @retval None
 */
void lcd_show_chn(uint32_t x, uint32_t y,uint8_t no, uint32_t fc, uint32_t bc,uint32_t font_size)
{
	uint32_t i,j;
	uint8_t tmp;

	lcd_addr_set(x, y, x + font_size-1, y + font_size-1);  // 设置显示区域

	// 计算中文字符占用的字节数：(字体大小 * 字体大小) / 8
	for (i = 0; i < (font_size*font_size/8); i++) 
	{
		// 根据字体大小选择对应的字模表
		if(font_size==16)tmp = chinese_tbl_16[no][i];
		if(font_size==24)tmp = chinese_tbl_24[no][i];	
		if(font_size==32)tmp = chinese_tbl_32[no][i];
		
		// 逐位处理字模数据
		for (j = 0;j < 8; j++)
		{
			if (tmp & (1<<j))  // 当前位是1，显示前景色
			{
				lcd_send_data(fc >> 8);
				lcd_send_data(fc);
			}
			else               // 当前位是0，显示背景色
			{
				lcd_send_data(bc);
				lcd_send_data(bc);
			}
		}
	}
}

/**
 * @brief  在指定位置画一个像素点
 * @param  x: X坐标
 * @param  y: Y坐标
 * @param  color: 颜色
 * @retval None
 */
void lcd_draw_point(uint32_t x, uint32_t y, uint32_t color)
{
	lcd_addr_set(x, y, x, y);  // 设置单个像素的区域

	lcd_send_data(color >> 8); // 发送颜色高8位
	lcd_send_data(color);      // 发送颜色低8位
}


/**
 * @brief  画一条直线（使用Bresenham算法）
 * @param  x1: 起点X坐标
 * @param  y1: 起点Y坐标
 * @param  x2: 终点X坐标
 * @param  y2: 终点Y坐标
 * @param  color: 线条颜色
 * @retval None
 */
void lcd_draw_line(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	
	delta_x=x2-x1; //计算坐标差值
	delta_y=y2-y1;
	uRow=x1;       //起点坐标
	uCol=y1;
	
	//确定X方向的步进
	if(delta_x>0)incx=1;           //向右走
	else if (delta_x==0)incx=0;    //竖直方向
	else {incx=-1;delta_x=-delta_x;} //向左走
	
	//确定Y方向的步进
	if(delta_y>0)incy=1;           //向下走
	else if (delta_y==0)incy=0;    //水平方向
	else {incy=-1;delta_y=-delta_y;} //向上走
	
	//选择最大的差值作为循环次数
	if(delta_x>delta_y)distance=delta_x;
	else distance=delta_y;
	
	//Bresenham算法核心：逐点绘制直线
	for(t=0;t<distance+1;t++)
	{
		lcd_draw_point(uRow,uCol,color); //画点
		xerr+=delta_x;
		yerr+=delta_y;
		
		if(xerr>distance)               //X方向需要步进
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)               //Y方向需要步进
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

/**
 * @brief  画一个矩形框
 * @param  x1: 左上角X坐标
 * @param  y1: 左上角Y坐标
 * @param  x2: 右下角X坐标
 * @param  y2: 右下角Y坐标
 * @param  color: 边框颜色
 * @retval None
 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	lcd_draw_line(x1,y1,x2,y1,color); // 上边
	lcd_draw_line(x1,y1,x1,y2,color); // 左边
	lcd_draw_line(x1,y2,x2,y2,color); // 下边
	lcd_draw_line(x2,y1,x2,y2,color); // 右边
}

/**
 * @brief  画一个圆形（使用中点画圆算法）
 * @param  x0: 圆心X坐标
 * @param  y0: 圆心Y坐标
 * @param  r: 半径（多少个像素）
 * @param  color: 圆的颜色
 * @retval None
 * 
 * 算法原理：
 * 画圆的巧妙方法是利用圆的对称性。一个圆可以分成8个对称的部分（八分之一圆）
 * 只要算出一个部分的点，就能通过对称得到其他7个部分的点，这样可以大大减少计算量。
 * 
 * 举例说明：
 * 假设圆心在(100,100)，半径是50，算出一个点(150,100)（右边最远处）
 * 那么对称的点就是：
 * - 左边：(50,100)
 * - 上边：(100,50)
 * - 下边：(100,150)
 * - 右上：(150,50)
 * - 左上：(50,50)
 * - 右下：(150,150)
 * - 左下：(50,150)
 * 
 * 程序中的方法：
 * 从a=0（起点）开始，b=r（半径），逐步增加a，减少b
 * 用勾股定理判断：a² + b² 是否大于 r²
 * 如果大于，说明点在圆外面，需要把b减小一点
 */
void lcd_draw_circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color)
{
	int a,b;  // a是X方向的偏移，b是Y方向的偏移
	
	a=0;      // 从最右边开始（X偏移为0）
	b=r;      // Y偏移等于半径
	
	// 循环条件：a <= b，表示只需要计算八分之一圆
	// 当a超过b时，就重复计算了
	while(a<=b)
	{
		// 神奇的地方：同时绘制圆上对称的8个点！
		// 这样画一个圆只需要计算1/8的点，效率很高
		
		lcd_draw_point(x0-b,y0-a,color);             // 左上区域
		lcd_draw_point(x0+b,y0-a,color);             // 右上区域
		lcd_draw_point(x0-a,y0+b,color);             // 左下区域
		lcd_draw_point(x0-a,y0-b,color);             // 左上区域（另一个对称点）
		lcd_draw_point(x0+b,y0+a,color);             // 右下区域
		lcd_draw_point(x0+a,y0-b,color);             // 右上区域（另一个对称点）
		lcd_draw_point(x0+a,y0+b,color);             // 右下区域（另一个对称点）
		lcd_draw_point(x0-b,y0+a,color);             // 左下区域（另一个对称点）
		
		a++;  // X方向偏移加1，向右移动一点
		
		// 判断：如果当前点(a,b)到圆心的距离平方大于半径平方
		// 说明这个点在圆外面，需要把Y方向偏移减小一点
		if((a*a+b*b)>(r*r))
		{
			b--;  // Y方向偏移减1，向上移动一点
		}
	}
}


/**
 * @brief  设置LCD显示方向
 * @note   0=正常(0度), 1=90度(横屏), 2=180度(倒置), 3=270度
 * @param  dir: 方向值(0/1/2/3)
 * @retval None
 */
void lcd_set_direction(uint32_t dir)
{
	g_lcd_direction = dir;
	
	/* 0度 */
	if(dir==0)
	{
		lcd_send_cmd(0x36);
		lcd_send_data(0x00);
		g_lcd_width=LCD_WIDTH;
		g_lcd_height=LCD_HEIGHT;
	}
	
	/* 90度 */
	if(dir==1)
	{
		lcd_send_cmd(0x36);
		lcd_send_data((1<<6)|(1<<5)|(1<<4));
		g_lcd_width=LCD_HEIGHT;  // 旋转后宽高互换
		g_lcd_height=LCD_WIDTH;
	
	}	
	
	/* 180度 */
	if(dir==2)
	{
		lcd_send_cmd(0x36);
		lcd_send_data((1<<7)|(1<<6));
		g_lcd_width=LCD_WIDTH;
		g_lcd_height=LCD_HEIGHT;
	
	}
	
	/* 270度 */
	if(dir==3)
	{
		lcd_send_cmd(0x36);
		lcd_send_data((1<<7)|(0<<6)|(1<<5)|(1<<4));
		g_lcd_width=LCD_HEIGHT;  // 旋转后宽高互换
		g_lcd_height=LCD_WIDTH;
	
	}		
}

/**
 * @brief  获取当前LCD显示方向
 * @retval 当前方向值(0/1/2/3)
 */
uint32_t lcd_get_direction(void)
{
	return g_lcd_direction;
}

/**
 * @brief  SPI1发送数据的DMA初始化函数
 * @note   使用DMA可以自动发送大量数据，减轻CPU负担
 * @param  DMA_Memory0BaseAddr: 内存缓冲区地址
 * @param  DMA_BufferSize: 缓冲区大小
 * @param  DMA_MemoryDataSize: 内存数据宽度
 * @param  DMA_MemoryInc: 是否递增内存地址
 * @retval None
 */
void spi1_tx_dma_init(uint32_t DMA_Memory0BaseAddr, uint16_t DMA_BufferSize, uint32_t DMA_MemoryDataSize, uint32_t DMA_MemoryInc)
{
	NVIC_InitTypeDef 	NVIC_InitStructure;		
	DMA_InitTypeDef 		DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); // 使能DMA2时钟

	DMA_DeInit(DMA2_Stream3);  // 复位DMA2的Stream3

	// 等待DMA2_Stream3准备就绪
	while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE)
		;

	/* 配置 DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_3;							// 通道3，连接SPI1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;			// 外设地址：SPI1数据寄存器
	DMA_InitStructure.DMA_Memory0BaseAddr = DMA_Memory0BaseAddr;			// DMA内存缓冲区地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;					// 方向：内存到外设
	DMA_InitStructure.DMA_BufferSize = DMA_BufferSize;						// 数据缓冲区大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设地址不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc;						// 内存地址是否递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据宽度：8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize;				// 内存数据宽度
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// 正常模式（发送完停止）
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						// 高优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;					// 启用FIFO模式
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;			// FIFO阈值：满
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				// 内存突发：单次
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		// 外设突发：单次
	DMA_Init(DMA2_Stream3, &DMA_InitStructure);

	DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_TCIF3);  // 清除传输完成标志
	
	DMA_Cmd(DMA2_Stream3, ENABLE);  // 启用DMA2_Stream3
	
	
	/* 配置DMA中断 */		
    DMA_ITConfig(DMA2_Stream3,DMA_IT_TC,ENABLE);  // 启用传输完成中断

    // 中断初始化
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream3_IRQn;	
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;				
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;						 
    NVIC_Init(&NVIC_InitStructure);

}

/**
 * @brief  启动SPI1的DMA发送
 * @retval None
 */
void spi1_tx_dma_start(void)
{
	DMA_Cmd(DMA2_Stream3, ENABLE);
}

/**
 * @brief  停止SPI1的DMA发送
 * @retval None
 */
void spi1_tx_dma_stop(void)
{
	DMA_Cmd(DMA2_Stream3, DISABLE);
}


/**
 * @brief  DMA2_Stream3中断处理函数（SPI1 DMA发送完成）
 * @note   当DMA发送完成时，会自动拉高CS引脚结束通信
 * @retval None
 */
void DMA2_Stream3_IRQHandler(void)
{
    // 判断是否是DMA传输完成中断
    if(DMA_GetITStatus(DMA2_Stream3, DMA_IT_TCIF3))	
    {
        // 清除DMA传输完成标志
        DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TCIF3);	
		
        // 片选拉高，数据传输完成	
        SPI_CS_1;	
    }
	
}
