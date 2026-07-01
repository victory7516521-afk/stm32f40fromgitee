#ifndef __SPIFLASH_H
#define __SPIFLASH_H

#include "stm32f4xx.h"
#include "sys.h"

#define F_CS PBout(14)

void Spiflash_Init(void);
u16 W25q128_Id(void);
void Write_Enable(void);
void Wait_Busy(void);
void Erase_Sector(u32 addr);
void Page_Write(u32 addr, u8 *write_buff, u32 len);
void Cross_Page_Write(u32 addr, u8 *write_buff, u32 len);
void Read_Data(u32 addr, u8 *read_buff, u32 len);
void Dual_SPI_Read(u32 addr, u8 *read_buff, u32 len);
void Dual_SPI_Write(u32 addr, u8 *write_buff, u32 len);
#endif
