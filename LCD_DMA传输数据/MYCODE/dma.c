#include "dma.h"																	   	  
#include "delay.h"	


volatile u8  g_dma_send_flag = 0;

// DMA是什么？
// DMA是Direct Memory Access（直接内存访问）的缩写
// 简单说就是：让硬件自动搬运数据，不需要CPU插手
// 
// 打个比方：
// - 没有DMA：CPU要把数据从内存搬到串口，得自己一个字节一个字节地搬
// - 有了DMA：CPU告诉DMA"从内存地址A搬N个字节到串口"，然后CPU就去干别的了
//   DMA搬完后会发个中断告诉CPU"我搬完了"
// 
// 这个函数的作用：配置DMA通道（从内存到外设模式）
// 
// 参数说明：
// DMA_Streamx: DMA流选择，比如DMA2_Stream2
// chx: DMA通道选择，比如DMA_Channel_4
// par: 外设地址，比如串口的数据寄存器地址 &USART1->DR
// mar: 内存地址，比如数组SendBuff的地址
// ndtr: 要传输的数据数量（多少个字节）
// 
// 使用例子：
// MYDMA_Config(DMA2_Stream7, DMA_Channel_4, (u32)&USART1->DR, (u32)SendBuff, strlen(SendBuff));
void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx, u32 chx, u32 par, u32 mar, u16 ndtr)
{ 
	DMA_InitTypeDef		DMA_InitStruct;  // DMA配置结构体
	NVIC_InitTypeDef    NVIC_InitStructure;  // 中断控制器配置结构体
	

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);//1. 开启DMA时钟（就像打开电源开关）

	DMA_DeInit(DMA_Streamx);//2. 复位DMA流的配置（恢复出厂设置）

	DMA_InitStruct.DMA_Channel = chx;	//3. 配置DMA通道的各项参数

	DMA_InitStruct.DMA_PeripheralBaseAddr = par;// 比如串口1的数据寄存器地址 &USART1->DR

	DMA_InitStruct.DMA_Memory0BaseAddr = mar;// 设置内存地址（数据从哪里来）
	
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;// 设置传输方向

	DMA_InitStruct.DMA_BufferSize = ndtr;// 设置要传输的数据数量（多少个字节）

	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;// 外设地址是否自动增加？否（Disable）

	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;// 内存地址是否自动增加？是（Enable）

	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;// 外设数据宽度：字节（Byte），也就是8位

	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;// 内存数据宽度：字节（Byte），和外设保持一致

	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;// DMA模式：普通模式（Normal）

	DMA_InitStruct.DMA_Priority = DMA_Priority_High;// DMA优先级：高（High）

	DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;// 内存突发模式：单次（Single）

	DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;// 外设突发模式：单次（Single）
	
	// 是否启用FIFO模式：禁用（Disable）
	// FIFO是一个数据缓冲区，可以先存一些数据再一次性发送
	// 禁用后数据直接传输，不经过FIFO
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	
	// 应用上述配置到DMA流
	DMA_Init(DMA_Streamx, &DMA_InitStruct);

	//配置中断（NVIC）
	// 当DMA传输完成时，会产生中断，告诉CPU"我搬完了"

	// 选择中断通道：DMA2_Stream2对应的中断号
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream2_IRQn;
	
	// 设置抢占优先级：1（数值越小优先级越高）
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	
	// 设置子优先级：2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
	
	// 启用这个中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
	// 应用中断配置
	NVIC_Init(&NVIC_InitStructure);

	//5. 启用DMA传输完成中断
	// DMA_IT_TC = Transfer Complete（传输完成）
	DMA_ITConfig(DMA_Streamx, DMA_IT_TC, ENABLE);
	
	//6. 启用DMA流，开始工作
	DMA_Cmd(DMA_Streamx, ENABLE);
} 


// DMA2_Stream2中断处理函数
// 当DMA2的Stream2传输完成时，会自动调用这个函数
// 
// 中断处理流程：
// 1. 判断是否是"传输完成"中断
// 2. 如果是，执行传输完成后的操作
// 3. 清理中断标志，告诉CPU已经处理完这个中断
void DMA2_Stream2_IRQHandler(void)
{
	// 判断是否是"传输完成"中断（DMA_IT_TCIF7 = Transfer Complete Interrupt Flag）
	if(DMA_GetITStatus(DMA2_Stream2, DMA_IT_TCIF2) == SET)
	{
		// 调试用：翻转PF9引脚的电平（可以接个LED看传输是否完成）
		GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
		
		// 1. 禁用串口的DMA发送请求
		// 告诉串口："不要再让DMA给你接收数据了"
		USART_DMACmd(USART1, USART_DMAReq_Rx, DISABLE);
		
		// 2. 禁用DMA2_Stream7
		// 停止DMA工作，因为数据已经传输完了
		DMA_Cmd(DMA2_Stream2, DISABLE);
	
		// 3. 设置传输完成标志
		// 主程序可以通过检查这个标志来判断传输是否完成
		g_dma_send_flag = 1;
		
		// 4. 清除中断标志位
		// 告诉CPU："我已经处理完这个中断了，可以接受下一个中断了"
		DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_TCIF2);
	}	
}

























