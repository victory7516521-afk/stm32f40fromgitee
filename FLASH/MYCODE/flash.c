#include "flash.h"



// 根据地址找到对应的Flash扇区编号
// 参数：Address - Flash地址（比如0x08020000）
// 返回：扇区编号（0~7，对应Sector_0到Sector_7）
// 
// STM32F4的Flash扇区分布说明：
// Flash就像一个大仓库，被分成了8个小房间（扇区），每个房间大小不一样：
// 
// 扇区编号 | 起始地址    | 结束地址    | 大小
// --------|------------|------------|------
// Sector 0 | 0x08000000 | 0x08003FFF | 16KB
// Sector 1 | 0x08004000 | 0x08007FFF | 16KB  
// Sector 2 | 0x08008000 | 0x0800BFFF | 16KB
// Sector 3 | 0x0800C000 | 0x0800FFFF | 16KB
// Sector 4 | 0x08010000 | 0x0801FFFF | 64KB
// Sector 5 | 0x08020000 | 0x0803FFFF | 128KB
// Sector 6 | 0x08040000 | 0x0805FFFF | 128KB
// Sector 7 | 0x08060000 | 0x0807FFFF | 128KB
// 
// 为什么需要这个函数？
// 因为写Flash之前必须先擦除扇区，而擦除是按扇区来的，不能只擦除一个字节。
// 所以要先知道地址在哪个扇区，才能正确擦除。
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;  // 扇区编号，默认初始化为0
  
  // 判断地址在哪个扇区范围之内
  // 每个if语句检查地址是否在某个扇区的起始地址和下一个扇区起始地址之间
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  // 地址在0x08000000 ~ 0x08003FFF之间，属于扇区0
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  // 地址在0x08004000 ~ 0x08007FFF之间，属于扇区1
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  // 地址在0x08008000 ~ 0x0800BFFF之间，属于扇区2
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  // 地址在0x0800C000 ~ 0x0800FFFF之间，属于扇区3
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  // 地址在0x08010000 ~ 0x0801FFFF之间，属于扇区4
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  // 地址在0x08020000 ~ 0x0803FFFF之间，属于扇区5
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  // 地址在0x08040000 ~ 0x0805FFFF之间，属于扇区6
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  // 地址在0x08060000 ~ 0x0807FFFF之间，属于扇区7
  }

  // 返回对应的扇区编号
  return sector;
}


//flash是内部
//void Flash_Write(0x08020000, u8 *write_buff, u32 len)
// 
// 向STM32内部Flash写入数据（按字节写入）
// 参数说明：
// addr: 写入的起始地址（必须是Flash地址，比如0x08020000，要避开程序代码区域）
// write_buff: 要写入的数据缓冲区指针（存放着要写的数据）
// len: 要写入的数据长度（多少个字节）
// 
// Flash写入规则：
// 1. 写之前必须先解锁Flash（就像开门需要钥匙）
// 2. 写之前必须先擦除扇区（Flash只能从1改成0，不能直接改，必须先全部擦成1）
// 3. 擦除只能按扇区擦，不能只擦一个字节
// 4. 写完之后要重新上锁（防止误操作）
void Flash_Write(u32 addr, u8 *write_buff, u32 len)
{
	u32 FLASH_USER_START_ADDR, FLASH_USER_END_ADDR;//flash 存储器的 起始地址和终止地址
    uint32_t uwStartSector = 0; // 起始扇区编号
    uint32_t uwEndSector = 0;   // 结束扇区编号
    uint32_t uwAddress = 0;     // 当前写入地址
    uint32_t uwSectorCounter = 0; // 扇区计数器
    
	
	//解锁Flash，允许操作Flash寄存器（默认是锁定状态，防止误写）
	FLASH_Unlock();
	

	//清空标志位
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
					FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	
	//计算开始地址与结束地址  
	//方便知道数据写入在哪个扇区（写入要擦除扇区）
    FLASH_USER_START_ADDR = addr;
    FLASH_USER_END_ADDR = FLASH_USER_START_ADDR+len;
    
	
    /* Get the number of the start and end sectors */
    //计算开始与结束扇区
    //比如数据从0x08020000开始，写1000个字节，结束地址是0x080203E8
    //开始扇区是Sector_5，结束扇区也是Sector_5，只需要擦除一个扇区
    uwStartSector = GetSector(FLASH_USER_START_ADDR);
    uwEndSector   = GetSector(FLASH_USER_END_ADDR);
	
    /* Strat the erase operation */
    //开始扇区赋值给 uwSectorCounter，作为擦除的起点
    uwSectorCounter = uwStartSector;
    
    //循环擦除扇区：从起始扇区开始，一直擦到结束扇区
    //为什么uwSectorCounter每次加8？
    //因为STM32库中扇区编号的定义是：
    // FLASH_Sector_0 = 0, FLASH_Sector_1 = 8, FLASH_Sector_2 = 16...
    // 所以每次加8就是跳到下一个扇区
	while (uwSectorCounter <= uwEndSector) 
	{
		/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
			be done by word */ 
		//擦除扇区函数
		//VoltageRange_3表示芯片电压在2.7V~3.6V之间
        if (FLASH_EraseSector(uwSectorCounter, VoltageRange_3) != FLASH_COMPLETE)
        { 
            /* Error occurred while sector erase. 
            User can add here some code to deal with this error  */
            //错误处理：擦除失败，打印error并返回
            printf("error\r\n");
            return;
        }
		
		/* jump to the next sector */
		//换下一个扇区，需要加8（因为Sector_0=0, Sector_1=8, Sector_2=16...）
		uwSectorCounter += 8;
		
	}
	
	/* Program the user Flash area word by word ********************************/
	/* area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR */
	
    /* area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR */
    //开始地址赋值给uwAddress，作为写入的起点
    uwAddress = FLASH_USER_START_ADDR;
    
    //循环写入数据：从起始地址开始，一个字节一个字节地写入
    //uwAddress从起始地址增加到结束地址
    while (uwAddress < FLASH_USER_END_ADDR)
    {
        //*(__IO uint8_t*)uwAddress = *write_buff  0x0802000
        //调用库函数写入一个字节到Flash
        //参数1：Flash地址，参数2：要写入的数据
        if (FLASH_ProgramByte(uwAddress, *write_buff) == FLASH_COMPLETE)
        {
            //写入成功，Flash地址加1，准备写入下一个字节
            uwAddress = uwAddress + 1;
            
            write_buff++; //数据指针加1，指向下一个要写入的数据
        }
        else
        { 
            //错误处理：写入失败，打印error并返回
            printf("error\r\n");
            return;
        }
    }
    
	
	/* Lock the Flash to disable the flash control register access (recommended
	to protect the FLASH memory against possible unwanted operation) */
	//写完数据后，锁定Flash，防止误操作（就像写完东西后锁上门）
	FLASH_Lock();  
    
}

//FLASH读数据
// 
// 从STM32内部Flash读取数据（按字节读取）
// 参数说明：
// addr: 读取的起始地址（必须是Flash地址，比如0x08020000）
// read_buff: 存储读取数据的缓冲区指针（读完的数据会放到这里）
// len: 要读取的数据长度（多少个字节）
// 
// Flash读取规则：
// 1. 读Flash不需要解锁，直接读就行（和写不一样）
// 2. Flash地址是内存映射的，所以可以像读普通内存一样读Flash
// 3. 读取速度很快，不需要等待
void Flash_Read(u32 addr, u8 *read_buff, u32 len)
{
    u32 FLASH_USER_START_ADDR, FLASH_USER_END_ADDR;
    uint32_t uwAddress = 0;

    
    //计算开始地址与结束地址
    FLASH_USER_START_ADDR = addr;
    FLASH_USER_END_ADDR = FLASH_USER_START_ADDR+len;
    
    
    //开始地址赋值给uwAddress，作为读取的起点
    uwAddress = FLASH_USER_START_ADDR;
    
    
    //循环读取数据：从起始地址开始，一个字节一个字节地读
    while (uwAddress < FLASH_USER_END_ADDR)
    {
		//解引用，访问FLASH空间
		//(__IO uint8_t*)uwAddress 把Flash地址转换成字节指针
		//*(__IO uint8_t*)uwAddress 读取该地址的值
		//然后把读到的值存入read_buff指向的缓冲区
        *read_buff = *(__IO uint8_t*)uwAddress;
   
        //Flash地址加1，准备读取下一个字节
        uwAddress = uwAddress + 1;
        
        //缓冲区指针加1，准备存储下一个字节
        read_buff++;
    }  

}