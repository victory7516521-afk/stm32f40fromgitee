#ifndef __ES8266_MQTT_H
#define __ES8266_MQTT_H

#include "stm32f4xx.h"


#define WEATHER_URL  		"api.seniverse.com"
#define WEATHER_PORT  	80

extern int32_t esp8266_mqtt_init(void);


#endif
