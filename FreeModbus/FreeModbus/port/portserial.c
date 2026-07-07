/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "stm32f4xx.h"	//添加STM32头文件
#include "usart.h"		//添加串口头文件

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
	//STM32串口 接收中断使能  
	if(xRxEnable==TRUE)   
	{   
		//使能接收和接收中断  
		 USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);  
	}   
	else if(xRxEnable == FALSE)  
	{   
		 //禁止接收和接收中断    
		 USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);  
	}  
	
	//STM32串口 发送中断使能  
	if(xTxEnable==TRUE)   
	{  
		//使能发送完成中断  
		USART_ITConfig(USART2, USART_IT_TXE, ENABLE);  
	}   
	else if(xTxEnable == FALSE)   
	{  
		//禁止发送完成中断  
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);  
	}  
	else if(xTxEnable == FALSE)   
	{  
		 //MODBUS_RECIEVE();  
		 USART_ITConfig(USART2, USART_IT_TC, DISABLE);  
	}	
	
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
	
	//串口初始化，只需要传输串口波特率
	Usart2_Init((uint16_t)ulBaudRate);	
	
    return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
	//发送数据
	USART_SendData(USART2, ucByte);  
	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){};	
	
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
	//接收数据
	*pucByte = USART_ReceiveData(USART2);    
    return TRUE;  
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}


//串口中断处理函数  
void USART2_IRQHandler(void)  
{  
	if(USART_GetITStatus(USART2, USART_IT_TXE) == SET)   
	{  
		prvvUARTTxReadyISR();  
		USART_ClearITPendingBit(USART2, USART_IT_TXE);  
	}  
   
	if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)   
	{  
		prvvUARTRxISR();  
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);  
	}  
}  





