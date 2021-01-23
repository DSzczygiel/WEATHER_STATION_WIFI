/*
 * DHT11.h
 *
 *  Created on: 12 lip 2020
 *      Author: Daniel
 *
 *      DHT-11 temperature and humidity sensor
 */

#ifndef DHT11_H_
#define DHT11_H_

#include <avr/io.h>

#define DHT11_DDR DDRC
#define DHT11_PORT PORTC
#define DHT11_PIN PINC
#define DHT11 (1 << PC1)

struct dht11_data{
	uint8_t tempInteger;
	uint8_t tempFraction;
	uint8_t rhInteger;
	uint8_t rhFraction;
};

void dht11_init();
void dht11_getData(struct dht11_data *data);
#endif /* DHT11_H_ */
