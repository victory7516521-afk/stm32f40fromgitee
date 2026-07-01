#include "tft.h"

#include "touch.h"  
#include "stm32f4xx.h"
#include "delay.h"

/*使用到的结构体*/
static NVIC_InitTypeDef NVIC_InitStructure;       // 中断控制器配置结构体
static EXTI_InitTypeDef  EXTI_InitStructure;      // 外部中断配置结构体
static GPIO_InitTypeDef GPIO_InitStructure;       // GPIO配置结构体

/* 全局变量： */
volatile uint32_t g_tp_event=0;     // 触摸屏事件标志，=1表示有触摸事件
uint16_t g_tp_x,g_tp_y;             // 触摸坐标（原始值）
uint8_t  g_tp_finger_num=0;         // 触摸点数量（0表示没有触摸）

/**
 * @brief  设置触摸屏SDA引脚的模式（输入或输出）
 * @note   SDA引脚是I2C双向数据线，需要在发送和接收之间切换方向
 * @param  pin_mode: GPIO_Mode_OUT（输出）或 GPIO_Mode_IN（输入）
 * @retval None
 */
void tp_sda_pin_mode(GPIOMode_TypeDef pin_mode)
{
#if TP_PIN_DEF == TP_PIN_DEF_1		
	// 引脚定义1：SDA使用PD14
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;		//选择PD14引脚
	GPIO_InitStructure.GPIO_Mode = pin_mode;	//设置为输入或输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	//开漏模式（I2C必须用开漏）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//IO速度100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//不使用上下拉
	GPIO_Init(GPIOD, &GPIO_InitStructure);	
	
#endif 
	
	
#if TP_PIN_DEF == TP_PIN_DEF_2	
	// 引脚定义2：SDA使用PD7
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;		//选择PD7引脚
	GPIO_InitStructure.GPIO_Mode = pin_mode;	//设置为输入或输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	//开漏模式（I2C必须用开漏）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//IO速度100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//不使用上下拉
	GPIO_Init(GPIOD, &GPIO_InitStructure);	
	
#endif

}

/**
 * @brief  I2C起始信号
 * @note   I2C协议规定：SCL为高电平时，SDA从高变低表示开始
 * @retval None
 */
void tp_i2c_start(void)
{
	//确保SDA引脚为输出模式
	tp_sda_pin_mode(GPIO_Mode_OUT);
	
	TP_SCL_W=1;    // SCL拉高
	TP_SDA_W=1;    // SDA拉高
	delay_us(1);   // 短暂延时
	
	TP_SDA_W=0;    // SDA拉低（关键：SCL高时SDA下降沿=起始信号）
	delay_us(1);

	TP_SCL_W=0;    // SCL拉低，准备发送数据
	delay_us(1);
}


/**
 * @brief  I2C停止信号
 * @note   I2C协议规定：SCL为高电平时，SDA从低变高表示结束
 * @retval None
 */
void tp_i2c_stop(void)
{
	//确保SDA引脚为输出模式
	tp_sda_pin_mode(GPIO_Mode_OUT);
	
	TP_SCL_W=1;    // SCL拉高
	TP_SDA_W=0;    // SDA拉低
	delay_us(1);
	
	TP_SDA_W=1;    // SDA拉高（关键：SCL高时SDA上升沿=停止信号）
	delay_us(1);
}

/**
 * @brief  通过I2C发送一个字节
 * @note   I2C发送数据时，SCL低电平准备数据，SCL高电平采样数据
 * @param  byte: 要发送的字节
 * @retval None
 */
void tp_i2c_send_byte(uint8_t byte)
{
	int32_t i;
	
	//确保SDA引脚为输出模式
	tp_sda_pin_mode(GPIO_Mode_OUT);
	
	TP_SCL_W=0;    // SCL拉低
	TP_SDA_W=0;    // SDA拉低
	delay_us(1);
	
	// 从高位到低位依次发送8位数据
	for(i=7; i>=0; i--)
	{
		if(byte & (1<<i))       // 判断当前位是1还是0
			TP_SDA_W=1;         // 是1就拉高SDA
		else
			TP_SDA_W=0;         // 是0就拉低SDA
	
		delay_us(1);
	
		TP_SCL_W=1;             // SCL拉高，从机在这时采样数据
		delay_us(1);
		
		TP_SCL_W=0;             // SCL拉低，准备下一位数据
		delay_us(1);		
	}
}

/**
 * @brief  I2C应答信号（主机发送给从机）
 * @param  ack: 0=应答（ACK），1=非应答（NACK）
 * @retval None
 */
void tp_i2c_ack(uint8_t ack)
{
	//确保SDA引脚为输出模式
	tp_sda_pin_mode(GPIO_Mode_OUT);
	
	TP_SCL_W=0;
	TP_SDA_W=0;
	delay_us(1);
	
	if(ack)
		TP_SDA_W=1;        // NACK：SDA=1
	else
		TP_SDA_W=0;        // ACK：SDA=0

	delay_us(1);

	TP_SCL_W=1;            // SCL拉高，从机采样
	delay_us(1);
	
	TP_SCL_W=0;            // SCL拉低
	delay_us(1);		
}

/**
 * @brief  等待I2C应答信号（从机发送给主机）
 * @note   主机接收完一个字节后，需要等待从机的ACK信号
 * @retval 0=收到ACK（从机应答），1=收到NACK（从机不应答）
 */
uint8_t tp_i2c_wait_ack(void)
{
	uint8_t ack;
	//确保SDA引脚为输入模式（等待从机拉低）
	tp_sda_pin_mode(GPIO_Mode_IN);

	//主机释放总线，等待从机应答
	TP_SCL_W=1;
	delay_us(1);
	
	if(TP_SDA_R)            // 读取SDA引脚状态
		ack=1;              // SDA=1，表示NACK
	else
		ack=0;              // SDA=0，表示ACK
	
	//SCL拉低，结束应答周期
	TP_SCL_W=0;
	delay_us(1);

	return ack;
}

/**
 * @brief  通过I2C接收一个字节
 * @note   主机接收数据时，SCL高电平期间读取SDA
 * @retval 收到的字节数据
 */
uint8_t tp_i2c_recv_byte(void)
{
	uint8_t d=0;
	int32_t i;
	
	//确保SDA引脚为输入模式
	tp_sda_pin_mode(GPIO_Mode_IN);

	// 从高位到低位依次接收8位数据
	for(i=7; i>=0; i--)
	{
		TP_SCL_W=1;         // SCL拉高，从机在这时输出数据
		delay_us(1);
		
		if(TP_SDA_R)        // 读取SDA引脚状态
			d|=1<<i;        // 如果是1，设置对应的位
		
		//SCL拉低，准备接收下一位
		TP_SCL_W=0;
		delay_us(1);	
	}

	return d;
}


/**
 * @brief  触摸屏底层硬件初始化
 * @note   配置GPIO、时钟、外部中断等硬件资源
 * @retval None
 */
void tp_lowlevel_init(void)
{
	
#if TP_PIN_DEF == TP_PIN_DEF_1	
	// 引脚定义1：使用PD0(SCL)、PD14(SDA)、PD4(RST)、PF12(IRQ)
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);  // 使能GPIOD时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);  // 使能GPIOF时钟
	
	//使能系统配置的硬件时钟（用于外部中断）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// 配置PD0、PD4、PD14为开漏输出（SCL、RST、SDA）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_4|GPIO_Pin_14;		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;				//输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;				//开漏模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//高速
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//无上下拉
	GPIO_Init(GPIOD, &GPIO_InitStructure);


	//I2C总线空闲时，SCL和SDA都要拉高
	TP_SCL_W=1;
	TP_SDA_W=1;	
	
	// 配置PF12为输入（IRQ中断引脚），上拉模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOF , &GPIO_InitStructure);
	
	/* 配置外部中断12号引脚 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line12;	//外部中断12号
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //下降沿触发（触摸时IRQ变低）
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//启用
	EXTI_Init(&EXTI_InitStructure);
	
	/* NVIC配置外部中断12号通道 */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;//中断号（EXTI10-15共用一个中断）
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x06;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//开启通道
	NVIC_Init(&NVIC_InitStructure);	
	//复位完成
	TP_RST=1;
	
#endif


#if TP_PIN_DEF == TP_PIN_DEF_2
	// 引脚定义2：使用PD6(SCL)、PD7(SDA)、PC6(RST)、PC8(IRQ)

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);  // 使能GPIOC时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);  // 使能GPIOD时钟
	
	//使能系统配置的硬件时钟（用于外部中断）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// 配置PD6、PD7为开漏输出（SCL、SDA）
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6|GPIO_Pin_7;		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;				//输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;				//开漏模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//高速
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//无上下拉
	GPIO_Init(GPIOD, &GPIO_InitStructure);


	//I2C总线空闲时，SCL和SDA都要拉高
	TP_SCL_W=1;
	TP_SDA_W=1;	

	// 配置PC6为推挽输出（RST复位引脚）
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOC , &GPIO_InitStructure);
	
	// 配置PC8为输入（IRQ中断引脚），上拉模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOC , &GPIO_InitStructure);
	
	// 将PC8映射到外部中断线8
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource8);
	
	/* 配置外部中断8号引脚 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;	//外部中断8号
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //下降沿触发（触摸时IRQ变低）
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//启用
	EXTI_Init(&EXTI_InitStructure);
	
	/* NVIC配置外部中断8号通道 */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//中断号（EXTI5-9共用一个中断）
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x06;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//开启通道
	NVIC_Init(&NVIC_InitStructure);
	
	//复位完成
	TP_RST=1;
	
#endif
}

/**
 * @brief  向触摸屏寄存器写入一个字节
 * @note   触摸屏芯片地址是0x2A（写），0x2B（读）
 * @param  addr: 寄存器地址
 * @param  data: 指向要写入数据的指针
 * @retval None
 */
void tp_send_byte(uint8_t addr,uint8_t* data)
{
	tp_i2c_start();                    // 起始信号
	tp_i2c_send_byte(0x2A);tp_i2c_wait_ack();  // 发送写地址0x2A，等待ACK
	tp_i2c_send_byte(addr);tp_i2c_wait_ack();  // 发送寄存器地址，等待ACK
	tp_i2c_send_byte(*data);tp_i2c_wait_ack(); // 发送数据，等待ACK
	tp_i2c_stop();                     // 停止信号
}

/**
 * @brief  从触摸屏寄存器读取一个字节
 * @note   触摸屏芯片地址是0x2A（写），0x2B（读）
 * @param  addr: 寄存器地址
 * @param  data: 指向存储数据的指针
 * @retval None
 */
void tp_recv_byte(uint8_t addr,uint8_t* data)
{
	tp_i2c_start();                    // 起始信号
	tp_i2c_send_byte(0x2A);tp_i2c_wait_ack();  // 发送写地址0x2A，等待ACK
	tp_i2c_send_byte(addr);tp_i2c_wait_ack();  // 发送寄存器地址，等待ACK
	tp_i2c_start();                    // 重复起始信号（切换到读模式）
	tp_i2c_send_byte(0x2B);tp_i2c_wait_ack();  // 发送读地址0x2B，等待ACK
	*data=tp_i2c_recv_byte();          // 接收一个字节
	tp_i2c_ack(1);                     // 发送NACK（表示不再接收）
	tp_i2c_stop();                     // 停止信号
}

/**
 * @brief  从触摸屏寄存器读取多个字节
 * @param  addr: 起始寄存器地址
 * @param  data: 指向存储数据缓冲区的指针
 * @param  len: 要读取的字节数
 * @retval None
 * 
 * I2C读取多个字节的流程说明：
 * 
 * 触摸屏芯片的地址：
 * - 写地址：0x2A（表示主机要向从机写数据）
 * - 读地址：0x2B（表示主机要从从机读数据）
 * 
 * 读取流程（共4步）：
 * 1. 发送起始信号，告诉从机开始通信
 * 2. 发送写地址0x2A，告诉从机"我要写一个寄存器地址给你"
 * 3. 发送要读取的寄存器地址，告诉从机"我想读这个地址的数据"
 * 4. 发送重复起始信号，然后发送读地址0x2B，告诉从机"现在开始，你把数据发给我"
 * 5. 连续接收数据，每个字节后发送ACK，最后一个字节发送NACK表示"我不要了"
 * 6. 发送停止信号，结束通信
 * 
 * ACK和NACK的含义：
 * - ACK（应答）：主机收到字节后，拉低SDA，表示"我收到了，继续发"
 * - NACK（非应答）：主机收到最后一个字节后，不拉低SDA，表示"我不要了，停止吧"
 */
void tp_recv(uint8_t addr,uint8_t* data,uint32_t len)
{
	uint8_t *p=data;  // 指向数据缓冲区的指针，用来存储读取的数据
	
	// 第一步：发送起始信号，开始I2C通信
	tp_i2c_start();                   
	
	// 第二步：发送写地址0x2A，等待从机应答
	// 告诉从机："我是主机，我要给你写数据"
	tp_i2c_send_byte(0x2A);tp_i2c_wait_ack();  
	
	// 第三步：发送寄存器地址，等待从机应答
	// 告诉从机："我想读取这个地址开始的数据"
	tp_i2c_send_byte(addr);tp_i2c_wait_ack();  
	
	// 第四步：发送重复起始信号，切换到读模式
	// I2C协议规定：从写模式切换到读模式需要发送一个重复的起始信号
	tp_i2c_start();                   
	
	// 第五步：发送读地址0x2B，等待从机应答
	// 告诉从机："现在开始，你把数据发给我"
	tp_i2c_send_byte(0x2B);tp_i2c_wait_ack();  
	
	len=len-1;  // 最后一个字节要发送NACK（表示不再接收），所以先减1
	
	// 第六步：循环读取前面的字节（除了最后一个）
	// 每读完一个字节，发送ACK告诉从机继续发
	while(len--)
	{
		*p=tp_i2c_recv_byte();        // 接收一个字节数据，保存到缓冲区
		tp_i2c_ack(0);                // 发送ACK（0=应答），告诉从机"收到了，继续发"
		p++;                          // 指针指向下一个位置，准备接收下一个字节
	}
	
	// 第七步：读取最后一个字节
	*p=tp_i2c_recv_byte();            // 接收最后一个字节
	tp_i2c_ack(1);                    // 发送NACK（1=非应答），告诉从机"我不要了"
	
	// 第八步：发送停止信号，结束通信
	tp_i2c_stop();                   
}

/**
 * @brief  触摸屏硬件复位
 * @note   通过拉低复位引脚10ms，然后拉高等待60ms让芯片初始化
 * @retval None
 */
void tp_reset(void)
{
	TP_RST=0;           // 拉低复位引脚
	delay_ms(10);       // 等待10ms
	
	TP_RST=1;           // 拉高复位引脚
	delay_ms(60);       // 等待60ms，芯片完成初始化
}

/**
 * @brief  触摸屏初始化
 * @note   初始化硬件、复位芯片、读取芯片ID验证通信是否正常
 * @retval None
 */
void tp_init(void)
{
	uint8_t ChipID=0;
	uint8_t FwVersion=0;
	
	tp_lowlevel_init();          // 初始化底层硬件
	
	tp_reset();                  // 硬件复位
	tp_recv_byte(0xa7,&ChipID);  // 读取芯片ID（地址0xA7）
	tp_recv_byte(0xa9,&FwVersion); // 读取固件版本（地址0xA9）
	
	printf("ChipID:%02X\r\n",ChipID);  // 串口打印芯片ID
	
	/*	
		常见触摸芯片ID
		CST716 : 0x20
		CST816S : 0xB4
		CST816T : 0xB5
		CST816D : 0xB6	
	*/
	
	printf("FwVersion:%02X\r\n",FwVersion);  // 串口打印固件版本
}


/**
 * @brief  获取当前触摸点数量
 * @retval 触摸点数量（0=无触摸，1=单点触摸）
 */
uint8_t tp_finger_num_get(void)
{
	return g_tp_finger_num;
}

/**
 * @brief  读取触摸坐标并处理显示方向
 * @note   触摸芯片返回的原始坐标需要根据屏幕方向进行转换
 * @param  screen_x: 指向存储屏幕X坐标的指针
 * @param  screen_y: 指向存储屏幕Y坐标的指针
 * @retval 手势码（0x00=按下，0x01=抬起，0x02=上滑，0x03=下滑等）
 */
uint8_t tp_read(uint16_t *screen_x,uint16_t *screen_y)
{
	uint8_t buf[7];       // 存储7字节触摸数据（触摸芯片一次返回7个字节）
	uint16_t x=0,y=0,tmp;
	
	//读取当前触摸点的全部数据（从地址0开始，读取7个字节）
	tp_recv(0,buf,7);
	
	//触摸芯片返回的数据格式（7个字节）：
	// buf[0]: 头部数据（固定为0x03）
	// buf[1]: 手势码（0x00=按下，0x01=抬起，0x02=上滑等）
	// buf[2]: 触摸点数量（低4位有效）
	// buf[3]: X坐标高4位 + 保留位
	// buf[4]: X坐标低8位
	// buf[5]: Y坐标高4位 + 保留位
	// buf[6]: Y坐标低8位
	
	//X坐标计算：buf[3]高4位 + buf[4]低8位
	// buf[3]&0x0F 提取低4位（实际上是X坐标的高4位）
	// <<8 左移8位，变成高8位
	// | buf[4] 和低8位合并，得到完整的12位X坐标
	x=(uint16_t)((buf[3]&0x0F)<<8)|buf[4];
	
	//Y坐标计算：buf[5]高4位 + buf[6]低8位
	// 原理和X坐标一样
	y=(uint16_t)((buf[5]&0x0F)<<8)|buf[6];	

	//触摸点数量（buf[2]的低4位）
	g_tp_finger_num = buf[2];

	// 检查坐标是否在屏幕范围内（防止触摸边缘时出现异常值）
	if((x<g_lcd_width) && (y<g_lcd_height))
	{
		// 根据屏幕显示方向转换坐标
		// 为什么需要转换？因为触摸芯片的坐标是固定的（以屏幕某个角落为原点）
		// 但LCD屏幕可以旋转显示（0度、90度、180度、270度）
		// 所以需要把触摸芯片返回的原始坐标转换为当前屏幕方向下的坐标
		
		if(lcd_get_direction()==1)     // 90度方向（横屏）
		{
		   tmp= x;              // 保存原始X坐标
			x = y;               // 旋转后，原来的Y变成了新的X
			y = g_lcd_height-tmp; // 旋转后，原来的X变成了新的Y，但需要反向
		}			
		
		if(lcd_get_direction()==2)     // 180度方向（倒置）
		{
			x = g_lcd_width-x;    // X坐标反向（左边变成右边）
			y = g_lcd_height-y;   // Y坐标反向（上边变成下边）
		}	

		if(lcd_get_direction()==3)     // 270度方向（另一种横屏）
		{
		   tmp= y;              // 保存原始Y坐标
			y = x;               // 旋转后，原来的X变成了新的Y
			x = g_lcd_width-tmp; // 旋转后，原来的Y变成了新的X，但需要反向
		}			
		
		*screen_x=x;                   // 输出转换后的X坐标（写入到调用者提供的变量中）
		*screen_y=y;                   // 输出转换后的Y坐标（写入到调用者提供的变量中）
		
		/*
			手势码说明：
			0x00=按下（手指刚碰到屏幕）
			0x01=抬起（手指离开屏幕）
			0x02=上滑（手指向上移动）
			0x03=下滑（手指向下移动）
			0x04=左滑（手指向左移动）
			0x05=右滑（手指向右移动）
			0x0B=双击（快速按两次）
			0x0C=长按（按住不放超过一定时间）		
		*/

		return buf[1];  // 返回手势码
	}  	
	
	return 0;  // 坐标超出范围，返回0（表示无效触摸）
}


#if TP_PIN_DEF == TP_PIN_DEF_1
/**
 * @brief  EXTI15_10中断处理函数（引脚定义1的触摸中断）
 * @note   当触摸发生时，IRQ引脚产生下降沿中断
 * @retval None
 */
void EXTI15_10_IRQHandler(void)
{
	//获取外部中断12是否触发
	if(EXTI_GetITStatus(EXTI_Line12) != RESET)
	{
		/*触摸事件发生*/
		g_tp_event=1;   // 设置触摸事件标志
		
		/* tp_read是I2C通信，速度较慢，不建议在中断中调用 */
		//tp_read(&g_tp_x,&g_tp_y);
		  
		/*清除外部中断12的标志位，表示CPU已经处理*/
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
}
#endif

#if TP_PIN_DEF == TP_PIN_DEF_2
/**
 * @brief  EXTI9_5中断处理函数（引脚定义2的触摸中断）
 * @note   当触摸发生时，IRQ引脚产生下降沿中断
 * @retval None
 */
void EXTI9_5_IRQHandler(void)
{
	//获取外部中断8是否触发
	if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		/*触摸事件发生*/
		
		g_tp_event=1;   // 设置触摸事件标志
		
		/* tp_read是I2C通信，速度较慢，不建议在中断中调用 */
		//tp_read(&g_tp_x,&g_tp_y);
		  
		/*清除外部中断8的标志位，表示CPU已经处理*/
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
}
#endif
