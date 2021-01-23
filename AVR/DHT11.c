/*
 * DHT11.c
 *
 *  Created on: 12 lip 2020
 *      Author: Daniel
 */

#include "DHT11.h"
#include <util/delay.h>
#include <string.h>

#define DHT11_SET_LOW DHT11_PORT &= ~DHT11;
#define DHT11_SET_HIGH DHT11_PORT |= DHT11;
#define DHT11_SET_INPUT DHT11_DDR &= ~DHT11;
#define DHT11_SET_OUTPUT DHT11_DDR |= DHT11;

uint8_t _data[5];

uint8_t _dht11_waitForHigh(uint8_t timeOutUs){
	uint8_t counter = 0;
	while(!(DHT11_PIN & DHT11)){
		if(counter > timeOutUs)
			return 1;
		_delay_us(1);
		counter++;
	}
	return 0;
}

uint8_t _dht11_waitForLow(uint8_t timeOutUs){
	uint8_t counter = 0;
	while((DHT11_PIN & DHT11)){
		if(counter > timeOutUs)
			return 1;
		_delay_us(1);
		counter++;
	}
	return 0;
}

uint8_t _dht11_readBit(){
	if(_dht11_waitForHigh(100))
		return 0;
	_delay_us(30);
	if(DHT11_PIN & DHT11){
		if(_dht11_waitForLow(100))
			return 0;
		return 1;
	}
	return 0;
}

uint8_t _dht11_readByte(){
	DHT11_SET_LOW;
	DHT11_SET_INPUT;
	uint8_t bit = 0;
	memset(_data, 0, 5);
	for(uint8_t j=0; j<5; j++){
		uint8_t byte = 0;
		for(uint8_t i=0; i<8; i++){
			bit = _dht11_readBit();
			byte |= (bit << (7-i));
		}
	_data[j] = byte;
	}
	return 0;
}

uint8_t _dht11_start(){
	DHT11_SET_OUTPUT;
	DHT11_SET_HIGH;
	DHT11_SET_LOW;
	_delay_ms(18);
	DHT11_SET_HIGH;
	_delay_us(30);
	DHT11_SET_LOW;
	DHT11_SET_INPUT;

	if(_dht11_waitForHigh(100))
		return 0;

	if(_dht11_waitForLow(100))
		return 0;
	return 1;
}

void dht11_init(){
	DHT11_SET_LOW;
	DHT11_SET_INPUT;
}

void dht11_getData(struct dht11_data *data){
	_dht11_start();
	_dht11_readByte();
	data->rhInteger = _data[0];
	data->rhFraction = _data[1];
	data->tempInteger = _data[2];
	data->tempFraction = _data[3];
}
