#include "dma.h"																	   	  
#include "delay.h"	


volatile u8  g_dma_send_flag = 0;

//DMAx的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//从存储器->外设模式/8位数据宽度/存储器增量模式
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
//chx:DMA通道选择,@ref DMA_channel DMA_Channel_0~DMA_Channel_7
//par:外设地址
//mar:存储器地址
//ndtr:数据传输量  
//MYDMA_Config(DMA2_Stream7,DMA_Channel_4,(u32)&USART1->DR,(u32)SendBuff,strlen(SendBuff));
void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr)
{ 
	DMA_InitTypeDef		DMA_InitStruct;
	NVIC_InitTypeDef    NVIC_InitStructure;
	
	//1、 使能DMA时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
	//2、复位DMA寄存器
	DMA_DeInit(DMA_Streamx);
	//3、初始化DMA通道参数

	DMA_InitStruct.DMA_Channel				= chx;  //通道
	DMA_InitStruct.DMA_PeripheralBaseAddr	= par;  //外设基地址
	DMA_InitStruct.DMA_Memory0BaseAddr		= mar;  //存储器地址
	DMA_InitStruct.DMA_DIR					= DMA_DIR_MemoryToPeripheral; //传输方向 存储器到外设
	DMA_InitStruct.DMA_BufferSize			= ndtr;  //一次性数据量
	DMA_InitStruct.DMA_PeripheralInc		= DMA_PeripheralInc_Disable; //外设地址不自增
	DMA_InitStruct.DMA_MemoryInc            = DMA_MemoryInc_Enable; //存储器地址自增
	DMA_InitStruct.DMA_PeripheralDataSize   = DMA_PeripheralDataSize_Byte;//外设数据宽度  1字节
	DMA_InitStruct.DMA_MemoryDataSize       = DMA_MemoryDataSize_Byte;//存储器数据宽度 1字节
	DMA_InitStruct.DMA_Mode					= DMA_Mode_Normal; //普通模式  一次传输
	DMA_InitStruct.DMA_Priority				= DMA_Priority_High; //优先级高
	DMA_InitStruct.DMA_MemoryBurst			= DMA_MemoryBurst_Single; //单次传输
	DMA_InitStruct.DMA_PeripheralBurst		= DMA_PeripheralBurst_Single; //单次传输
	
	//此参数选择DMA_FIFOMode_Disable，那么 DMA_FIFOThreshold参数值则无用 
	DMA_InitStruct.DMA_FIFOMode			    = DMA_FIFOMode_Disable; //不使用FIFO
	DMA_InitStruct.DMA_FIFOThreshold		= DMA_FIFOThreshold_Full;
	
	DMA_Init(DMA_Streamx, &DMA_InitStruct);

	//4、配置NVIC
	NVIC_InitStructure.NVIC_IRQChannel 					 = DMA2_Stream7_IRQn; //NVIC中断通道，可在stm32fxx.h头文件中查找
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 		 = 0x02;		  //响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd 				 = ENABLE; //通道使能
	NVIC_Init(&NVIC_InitStructure);

	//5、配置中断
	DMA_ITConfig(DMA_Streamx, DMA_IT_TC, ENABLE);
	//6、使能DMA1通道，启动传输。

	DMA_Cmd(DMA_Streamx, ENABLE);




	

} 

/* DMA_STREAM 传输完成中断服务函数 */
void DMA2_Stream7_IRQHandler(void)
{

	if(DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) == SET)	/* 获取 DMA_STREAM 传输完成中断标志 */
	{
		//进入中断，灯亮
		GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
		
		USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);		/* 失能串口发送请求 */
		DMA_Cmd(DMA2_Stream7, DISABLE);		/* 失能 DMA_STREAM */
	
		g_dma_send_flag = 1;
		
		DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);	/* 清除 DMA_STREAM 传输完成中断标志位，否则程序会无限陷入中断 */
	}	
	
}

























