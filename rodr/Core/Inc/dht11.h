/*
 * dht11.h
 *
 *  Created on: Mar 21, 2025
 *      Author: Mateusz Drogowski
 */

#ifndef DHT11_H_
#define DHT11_H_

//change lib below if needed
#include "stm32f7xx_hal.h"

struct DHT11
{
	GPIO_TypeDef* gpio_port;
	uint16_t gpio_pin;
	uint16_t* temperature;
	uint16_t* humidity;
	TIM_HandleTypeDef* htim;
};

void DHT11_ReadData(struct DHT11* dht);
#endif /* DHT11_H_ */
