#include "stm32f4xx.h"
#include "led.h"
#include "delay.h" 
#include "tim.h"
#include "usart.h"
#include "string.h"
#include "mb.h"
#include "mbport.h"

// 十路输入寄存器
#define REG_INPUT_SIZE  10
uint16_t REG_INPUT_BUF[REG_INPUT_SIZE];


// 十路保持寄存器
#define REG_HOLD_SIZE   10
uint16_t REG_HOLD_BUF[REG_HOLD_SIZE];


// 十路线圈
#define REG_COILS_SIZE 10
uint8_t REG_COILS_BUF[REG_COILS_SIZE];


// 十路离散量
#define REG_DISC_SIZE  10
uint8_t REG_DISC_BUF[10];







int main(void)
{
	//NVIC分组一个工程只能设置一次
	//设置NVIC分组为第二分组； 抢占优先级取值范围:0~3, 响应优先级取值范围:0~3
	int i = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Delay_Init();
	Led_Init();

	Usart1_Init(115200);

	
	//printf("helloworld\r\n");
	
	
	eMBInit(MB_RTU, 0x01, 3, 9600, MB_PAR_NONE);
	
	/* Enable the Modbus Protocol Stack. */
	eMBEnable();	
	
	while(1)
	{
		(void)eMBPoll();
	}

	return 0;
}


// CMD4
eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    USHORT usRegIndex = usAddress - 1; 

    // 非法检测
    if((usRegIndex + usNRegs) > REG_INPUT_SIZE)
    {
        return MB_ENOREG;
    }

    // 循环读取
    while( usNRegs > 0 )
    {
        *pucRegBuffer++ = ( unsigned char )( REG_INPUT_BUF[usRegIndex] >> 8 );
        *pucRegBuffer++ = ( unsigned char )( REG_INPUT_BUF[usRegIndex] & 0xFF );
        usRegIndex++;
        usNRegs--;
    }

    // 模拟输入寄存器被改变
    for(usRegIndex = 0; usRegIndex < REG_INPUT_SIZE; usRegIndex++)
    {
        REG_INPUT_BUF[usRegIndex]++;
    }

    return MB_ENOERR;
}

// CMD6、3、16
eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    USHORT usRegIndex = usAddress - 1;  
	
    // 非法检测
    if((usRegIndex + usNRegs) > REG_HOLD_SIZE)
    {
        return MB_ENOREG;
    }
	
	// 写寄存器
    if(eMode == MB_REG_WRITE)
    {
		//判断寄存器，用于控制灯
		if(pucRegBuffer[1] ==0)
		{
			GPIO_SetBits(GPIOF, GPIO_Pin_9);
			GPIO_SetBits(GPIOF, GPIO_Pin_10);
			GPIO_SetBits(GPIOE, GPIO_Pin_13);			
			
		}	
		if(pucRegBuffer[1] ==1)
		{
			GPIO_ResetBits(GPIOF, GPIO_Pin_9);
		}
		if(pucRegBuffer[1] ==2)
		{
			GPIO_ResetBits(GPIOF, GPIO_Pin_10);
		}		
		if(pucRegBuffer[1] ==3)
		{
			GPIO_ResetBits(GPIOE, GPIO_Pin_13);
		}		
				
		
        while( usNRegs > 0 )
        {
            REG_HOLD_BUF[usRegIndex] = (pucRegBuffer[0] << 8) | pucRegBuffer[1];
            pucRegBuffer += 2;
            usRegIndex++;
            usNRegs--;
        }
    }
	
	// 读寄存器
    else
    {
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ = ( unsigned char )( REG_HOLD_BUF[usRegIndex] >> 8 );
            *pucRegBuffer++ = ( unsigned char )( REG_HOLD_BUF[usRegIndex] & 0xFF );
            usRegIndex++;
            usNRegs--;
        }
    }

    return MB_ENOERR;
}

// CMD1、5、15
eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
    USHORT usRegIndex   = usAddress - 1;
    USHORT usCoilGroups = ((usNCoils - 1) / 8 + 1);
    UCHAR  ucStatus     = 0;
    UCHAR  ucBits       = 0;
    UCHAR  ucDisp       = 0;

    // 非法检测
    if((usRegIndex + usNCoils) > REG_COILS_SIZE)
    {
        return MB_ENOREG;
    }

    // 写线圈
    if(eMode == MB_REG_WRITE)
    {
        while(usCoilGroups--)
        {
            ucStatus = *pucRegBuffer++;
            ucBits   = 8;
            while((usNCoils--) != 0 && (ucBits--) != 0)
            {
                REG_COILS_BUF[usRegIndex++] = ucStatus & 0X01;
                ucStatus >>= 1;
            }
        }
    }

    // 读线圈
    else
    {
        while(usCoilGroups--)
        {
            ucDisp = 0;
            ucBits = 8;
            while((usNCoils--) != 0 && (ucBits--) != 0)
            {
                ucStatus |= (REG_COILS_BUF[usRegIndex++] << (ucDisp++));
            }
            *pucRegBuffer++ = ucStatus;
        }
    }
    return MB_ENOERR;
}


// CMD4
eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    USHORT usRegIndex   = usAddress - 1;
    USHORT usCoilGroups = ((usNDiscrete - 1) / 8 + 1);
    UCHAR  ucStatus     = 0;
    UCHAR  ucBits       = 0;
    UCHAR  ucDisp       = 0;

    // 非法检测
    if((usRegIndex + usNDiscrete) > REG_DISC_SIZE)
    {
        return MB_ENOREG;
    }

	// 读离散输入
	while(usCoilGroups--)
	{
		ucDisp = 0;
		ucBits = 8;
		while((usNDiscrete--) != 0 && (ucBits--) != 0)
		{
			if(REG_DISC_BUF[usRegIndex])
			{
				ucStatus |= (1 << ucDisp);
			}
			ucDisp++;
		}
		*pucRegBuffer++ = ucStatus;
	}

    // 模拟改变
    for(usRegIndex = 0; usRegIndex < REG_DISC_SIZE; usRegIndex++)
    {
        REG_DISC_BUF[usRegIndex] = !REG_DISC_BUF[usRegIndex];
    }

    return MB_ENOERR;
}


#ifdef  USE_FULL_ASSERT  
/**  
  * @brief  Reports the name of the source file and the source line number  
  *         where the assert_param error has occurred.  
  * @param  file: pointer to the source file name  
  * @param  line: assert_param error line source number  
  * @retval None  
  */    
void assert_failed(uint8_t* file, uint32_t line)    
{    
  /* User can add his own implementation to report the file name and line number,  
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */    
     
  /* Infinite loop */    
  while (1)    
  {    
  }    
}  
#else  
void __aeabi_assert(const char * x1, const char * x2, int x3)  
{  
}  
#endif  

