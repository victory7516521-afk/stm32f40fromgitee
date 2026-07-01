#ifndef __IIC_H
#define __IIC_H

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"

#define AT24C02_WR_ADD  0xA0
#define AT24C02_RD_ADD  0xA1

/*******************************
Òý½ÅËµÃ÷
SCL -- PB8
SDA -- PB9
********************************/


#define SCL  	PBout(8)
#define SDA_IN	PBin(9)
#define SDA_OUT	PBout(9)

void Iic_Init(void);
void At24c02_Write_Page(u8 addr, u8 *write_buf, u8 len);
void At24c02_Read_Data(u8 addr, u8 *read_buff, u8 len);
void At24c02_Write_Addr(u8 addr, u8 *write_buff, u8 len);


#endif