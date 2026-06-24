#ifndef __USART_H
#define __USART_H

#include "stm32f4xx.h"
#include "sys.h"
#include "esp8266.h"
#include "esp8266_mqtt.h"



extern volatile uint8_t  g_usart1_rx_buf[1024];
extern volatile uint32_t g_usart1_rx_cnt;
extern volatile uint32_t g_usart1_rx_end;

void Usart1_init(int BaudRate);
void Usart3_init(int BaudRate);
void usart3_send_str(char *str);
void ble_set_config(void);
void usart3_send_bytes(uint8_t *buf,uint32_t len);


#endif
