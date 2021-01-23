/*
 * DS18B20.c
 *
 *  Created on: 30 cze 2020
 *      Author: Daniel
 */

#include "DS18B20.h"
#include <util/delay.h>

#define DS18B20_SET_INPUT DS18B20_DDR &= ~DS18B20;
#define DS18B20_SET_OUTPUT DS18B20_DDR |= DS18B20
#define DS18B20_SET_LOW DS18B20_PORT &= ~DS18B20
#define DS18B20_SET_HIGH DS18B20_PORT |= DS18B20

#define DS18B20_SKIP_ROM 0xCC
#define DS18B20_INIT_CONVERSION 0x44
#define DS18B20_READ_SCRATCHPAD 0xBE
#define DS18B20_12_BIT_MULTIPLIER 625

void ds18b20_init(){
	DS18B20_SET_LOW;
	DS18B20_SET_OUTPUT;
}

void _ds18b20_sendOne(){
	DS18B20_SET_LOW;
	DS18B20_SET_OUTPUT;
	_delay_us(5);
	DS18B20_SET_INPUT;
	_delay_us(60);
}

void _ds18b20_sendZero(){
	DS18B20_SET_LOW;
	DS18B20_SET_OUTPUT;
	_delay_us(65);
	DS18B20_SET_INPUT;
}

void _ds18b20_sendByte(uint8_t byte){
	for(uint8_t i=0; i<8; i++){
		if(byte & (1 << i))
			_ds18b20_sendOne();
		else
			_ds18b20_sendZero();
	}
}

uint8_t _ds18b20_reset(){
	DS18B20_SET_OUTPUT;
	DS18B20_SET_LOW;
	_delay_us(480);
	DS18B20_SET_INPUT;
	_delay_us(60);

	uint8_t presence = (DS18B20_PORT & DS18B20_PIN);
	_delay_us(420);

	return presence;

}

uint8_t _ds18b20_readBit(){
	uint8_t bit = 0;
	DS18B20_SET_LOW;
	DS18B20_SET_OUTPUT;
	_delay_us(1);
	DS18B20_SET_INPUT;
	_delay_us(10);

	if(DS18B20_PIN & DS18B20)
		bit = 1;
	_delay_us(40);

	return bit;
}

uint8_t _ds18b20_readByte(){
	uint8_t byte = 0;
	for(uint8_t i=0; i<8; i++){
		byte |= (_ds18b20_readBit() << i);
	}
	return byte;
}

void ds18b20_getTemperature(struct ds18b20_data *data){
	uint8_t tempHighByte;
	uint8_t tempLowByte;

	if(_ds18b20_reset())
		return;
	_ds18b20_sendByte(DS18B20_SKIP_ROM);
	_ds18b20_sendByte(DS18B20_INIT_CONVERSION);

	while(_ds18b20_readBit() == 0);


	if(_ds18b20_reset())
		return;

	_ds18b20_sendByte(DS18B20_SKIP_ROM);
	_ds18b20_sendByte(DS18B20_READ_SCRATCHPAD);

	tempLowByte = _ds18b20_readByte();
	tempHighByte = _ds18b20_readByte();

	_ds18b20_reset();

	int8_t integer = (tempHighByte << 4) | (tempLowByte >> 4);
	uint16_t fraction = (tempLowByte & 0xF) * DS18B20_12_BIT_MULTIPLIER;
	double result = integer + fraction * 0.0001;
	data->tempInteger = result;
	data->tempFraction = (result - ((int8_t)result))*10;
}
