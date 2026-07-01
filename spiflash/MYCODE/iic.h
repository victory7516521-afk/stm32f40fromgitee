#ifndef __IIC_H
#define __IIC_H

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
/*******************************
Òý½ÅËµÃ÷
SCL -- PB8
SDA -- PB9
********************************/

#define AT24C02_ADD_W  0x0A0
#define AT24C02_ADD_R  0x0A1

#define SCL  	PBout(8)
#define SDA_IN	PBin(9)
#define SDA_OUT	PBout(9)

void Iic_Init(void);
void At24c02_Write_Page(u8 addr, u8 *write_buff, u8 len);
void At24c02_Write_Data(u8 addr, u8 *write_buff, u8 len);
void At24c02_Read_Data(u8 addr, u8 *read_buff, u8 len);

#endif