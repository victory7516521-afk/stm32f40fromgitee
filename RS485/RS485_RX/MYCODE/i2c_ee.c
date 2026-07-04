#include "i2c_ee.h"
#include "usart.h"


/*******************************************
I2C1_SCL --- PB8
I2C1_SDA -- PB9
使用I2C1
 *****************************************/
void I2C_GPIO_Config(void)
{

  GPIO_InitTypeDef  GPIO_InitStructure; 
  I2C_InitTypeDef  I2C_InitStructure; 


  //GPIOB初始化
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  //IIC时钟使能
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

  /* Connect PXx to I2C_SCL*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_I2C1);
  /* Connect PXx to I2C_SDA*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);  
  
  /*!< Configure EEPROM_I2C pins: SCL */   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		//复用
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	//开漏 这里一定写开漏
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  

  /* I2C 配置 */
  I2C_InitStructure.I2C_Mode 		= I2C_Mode_I2C;			//I2C模式
  I2C_InitStructure.I2C_DutyCycle 	= I2C_DutyCycle_2;		/* 高电平数据稳定，低电平数据变化 SCL 时钟线的占空比 */
  I2C_InitStructure.I2C_OwnAddress1 = 0x11; 				//指定自身的I2C设备地址 主机的地址是不能与从机相同即可
  I2C_InitStructure.I2C_Ack 		= I2C_Ack_Enable ;		//Ack使能		
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;	/* I2C的寻址模式 */
  I2C_InitStructure.I2C_ClockSpeed 		= 400000;	                            /* 通信速率  最大只能是400Kbps  */
  
  I2C_Init(I2C1, &I2C_InitStructure);	                                      /* I2C1 初始化 */
  
  
  I2C_Cmd(I2C1, ENABLE);  	                                                /* 使能 I2C1 */


}

//AT24C02写数据
uint32_t I2C_EE_PageWrite(u8 WriteAddr, u8* pBuffer,  u8 NumByteToWrite)
{
	
	//等待总线空闲
  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));  


  //启动信号
  I2C_GenerateSTART(I2C1, ENABLE);
	//等待启动正常(等待起始位发送)
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));/*EV5,主模式*/
 
  /* Send EEPROM address for write */
  //发送设备地址，并执行写操作
  I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Transmitter);
  /* Test on EV6 and clear it */ //地址发送结束
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); 

  /* Send the EEPROM's internal address to write to */    
	//发送写数据的起始地址
  I2C_SendData(I2C1, WriteAddr);  
  /* Test on EV8 and clear it */ //判断数据寄存器为空
  while(! I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 

  /* While there is data to be written */
  while(NumByteToWrite--)  
  {
    /* Send the current byte */
    I2C_SendData(I2C1, *pBuffer); 
    /* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	  
    /* Point to the next byte to be written */
    pBuffer++; 	  

  }

  /* Send STOP condition */
  I2C_GenerateSTOP(I2C1, ENABLE);
  
  return 1;
}

/**
  * @brief   从EEPROM里面读取一块数据 
  * @param   
  *		@arg pBuffer:存放从EEPROM读取的数据的缓冲区指针
  *		@arg WriteAddr:接收数据的EEPROM的地址
  *     @arg NumByteToWrite:要从EEPROM读取的字节数
  * @retval  无
  */
uint32_t I2C_EE_BufferRead(u8 ReadAddr, u8* pBuffer, u16 NumByteToRead)
{  


  //等待总线空闲
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));   

  /* Send START condition */
  I2C_GenerateSTART(I2C1, ENABLE);
  /* Test on EV5 and clear it */
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));


  /* Send EEPROM address for write */
  I2C_Send7bitAddress(I2C1, 0xA0, I2C_Direction_Transmitter);
  /* Test on EV6 and clear it */
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); 
  /* Clear EV6 by setting again the PE bit */

  //主机发送读数据的起始地址
  I2C_SendData(I2C1, ReadAddr);  
  /* Test on EV8 and clear it */
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
  
  //启动信号
  /* Send STRAT condition a second time */  
  I2C_GenerateSTART(I2C1, ENABLE);
  /* Test on EV5 and clear it */
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
  

//发送设备地址，执行读操作
  I2C_Send7bitAddress(I2C1, 0xA1, I2C_Direction_Receiver);
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
  
  
  /* While there is data to be read */
  while(NumByteToRead--)  
  {


	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)==0);  	
	/* Read a byte from the device */
	*pBuffer = I2C_ReceiveData(I2C1);

	/* Point to the next location where the byte read will be saved */
	pBuffer++; 


	
    if(NumByteToRead > 0)
    {
		//发送有效信号
		  I2C_AcknowledgeConfig(I2C1, ENABLE);
		 
    }	
			
				
  }

  /* Enable Acknowledgement to be ready for another reception */
  //发送无效应答
  I2C_AcknowledgeConfig(I2C1, DISABLE);
  
  /* Send STOP Condition */
  I2C_GenerateSTOP(I2C1, ENABLE);  
  
  return 0;
}

