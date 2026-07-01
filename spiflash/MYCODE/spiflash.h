#ifndef __SPIFLASH_H
#define __SPIFLASH_H

#include "stm32f4xx.h"
#include "sys.h"

#define F_CS PBout(14)

void Spiflash_Init(void);
u16 W25q128_Id(void);
void Erase_Sector(u32 addr);
void Page_Write(u32 addr, u8 *write_buff, u32 len);
//读数据
void Read_Data(u32 addr, u8 *read_buff, u32 len);
#endif
