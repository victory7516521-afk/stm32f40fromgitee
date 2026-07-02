#include "flash.h"



static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }


  return sector;
}


//flash是内部
//void Flash_Write(0x08020000, u8 *write_buff, u32 len)
void Flash_Write(u32 addr, u8 *write_buff, u32 len)
{
	u32 FLASH_USER_START_ADDR, FLASH_USER_END_ADDR;
    uint32_t uwStartSector = 0;
    uint32_t uwEndSector = 0;
    uint32_t uwAddress = 0;
    uint32_t uwSectorCounter = 0;
    
	
	//解锁
	FLASH_Unlock();
	
	/* Erase the user Flash area ************************************************/
	/* area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR */
	
	/* Clear pending flags (if any) */  
	//清空标志位
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
					FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	
	    //计算开始地址与结束地址  
	//方便知道数据写入在哪个扇区（写入要擦除扇区）
    FLASH_USER_START_ADDR = addr;
    FLASH_USER_END_ADDR = FLASH_USER_START_ADDR+len;
    
	
    /* Get the number of the start and end sectors */
    //计算开始与结束扇区
    uwStartSector = GetSector(FLASH_USER_START_ADDR);
    uwEndSector   = GetSector(FLASH_USER_END_ADDR);
	
    /* Strat the erase operation */
    //开始扇区赋值给 uwSectorCounter
    uwSectorCounter = uwStartSector;
    //循环擦除扇区
	while (uwSectorCounter <= uwEndSector) 
	{
		/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
			be done by word */ 
		//擦除扇区函数
        if (FLASH_EraseSector(uwSectorCounter, VoltageRange_3) != FLASH_COMPLETE)
        { 
            /* Error occurred while sector erase. 
            User can add here some code to deal with this error  */
            //错误处理
            printf("error\r\n");
            return;
        }
		
		/* jump to the next sector */
		//换下一个扇区，需要加8
		uwSectorCounter += 8;
		
	}
	
	/* Program the user Flash area word by word ********************************/
	/* area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR */
	
    /* area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR */
    //开始地址赋值给uwAddress
    uwAddress = FLASH_USER_START_ADDR;
    //循环写入数据
    while (uwAddress < FLASH_USER_END_ADDR)
    {
        //*(__IO uint8_t*)uwAddress = *write_buff  0x0802000
        if (FLASH_ProgramByte(uwAddress, *write_buff) == FLASH_COMPLETE)
        {
            //FLASH地址加1
            uwAddress = uwAddress + 1;
            
            write_buff++; //指针指向下一个数据空间
        }
        else
        { 
            //错误处理
            printf("error\r\n");
            return;
        }
    }
    
	
	/* Lock the Flash to disable the flash control register access (recommended
	to protect the FLASH memory against possible unwanted operation) */
	FLASH_Lock();  
    
}

//FLASH读数据
void Flash_Read(u32 addr, u8 *read_buff, u32 len)
{
    u32 FLASH_USER_START_ADDR, FLASH_USER_END_ADDR;
    uint32_t uwAddress = 0;

    
        //计算开始地址与结束地址
    FLASH_USER_START_ADDR = addr;
    FLASH_USER_END_ADDR = FLASH_USER_START_ADDR+len;
    
    
    uwAddress = FLASH_USER_START_ADDR;
    
    
    while (uwAddress < FLASH_USER_END_ADDR)
    {
		//解引用，访问FLASH空间
        *read_buff = *(__IO uint8_t*)uwAddress;
   
        uwAddress = uwAddress + 1;
        read_buff++;
    }  

}