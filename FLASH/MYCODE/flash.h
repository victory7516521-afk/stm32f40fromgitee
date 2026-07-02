#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f4xx.h"

#define  ADDR_FLASH_SECTOR_0   ((uint32_t)0x08000000) 
#define  ADDR_FLASH_SECTOR_1   ((uint32_t)0x08004000) 
#define  ADDR_FLASH_SECTOR_2   ((uint32_t)0x08008000) 
#define  ADDR_FLASH_SECTOR_3   ((uint32_t)0x0800C000) 
#define  ADDR_FLASH_SECTOR_4   ((uint32_t)0x08010000) 
#define  ADDR_FLASH_SECTOR_5   ((uint32_t)0x08020000) 
#define  ADDR_FLASH_SECTOR_6   ((uint32_t)0x08040000) 
#define  ADDR_FLASH_SECTOR_7   ((uint32_t)0x08060000) 
#define  ADDR_FLASH_SECTOR_8   ((uint32_t)0x08080000) 


void Flash_Write(u32 addr, u8 *write_buff, u32 len);
void Flash_Read(u32 addr, u8 *read_buff, u32 len);
#endif 