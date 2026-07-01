#ifndef __I2C_EE_H
#define	__I2C_EE_H

#include "stm32f4xx.h"


void I2C_GPIO_Config(void);
uint32_t I2C_EE_PageWrite(u8 WriteAddr, u8* pBuffer, u8 NumByteToWrite);
uint32_t I2C_EE_BufferRead(u8 ReadAddr, u8* pBuffer, u16 NumByteToRead);

#endif /* __I2C_EE_H */
