#ifndef __DMA_H
#define	__DMA_H	   
#include "sys.h"

extern volatile u8  g_dma_send_flag;

void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr);//配置DMAx_CHx
void MYDMA_Enable(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr);	//使能一次DMA传输		   
#endif






























