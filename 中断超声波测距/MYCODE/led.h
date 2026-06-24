#ifndef __LED_H
#define __LED_H

#include "stm32f4xx.h"

#define CONNECT_MQTT_LED(x)		PEout(14)=(x)?0:1

void Led_Init(void);



#endif
