#ifndef __SPIFLASH_H
#define __SPIFLASH_H


#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"

#define F_CS    PBout(14)


#define SCK		PBout(3)
#define MOSI	PBout(5)
#define MISO	PBin(4)


void Spiflash_Init(void);
u16 W25q128_id(void);

void Erase_Sector(u32 addr);
void Page_Write(u32 addr, u8 *write_buff, u32 len);
void Read_Data(u32 addr, u8 *read_buff, u32 len);

#endif