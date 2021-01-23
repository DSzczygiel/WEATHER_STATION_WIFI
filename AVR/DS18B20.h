/*
 * DS18B20.h
 *
 *  Created on: 3 maj 2020
 *      Author: Daniel
 *
 *      DS18B20 1-wire temperature sensor
 */

#ifndef DS18B20_H_
#define DS18B20_H_

#include <avr/io.h>

struct ds18b20_data{
	int8_t tempInteger;
	int8_t tempFraction;
};

#define DS18B20_DDR DDRC
#define DS18B20_PORT PORTC
#define DS18B20_PIN PINC
#define DS18B20 (1 << PC2)


void ds18b20_init();
void ds18b20_getTemperature(struct ds18b20_data *data);

#endif /* DS18B20_H_ */
