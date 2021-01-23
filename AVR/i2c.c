/*
 * i2c.c
 *
 *  Created on: 15 lip 2020
 *      Author: Daniel
 */

#include "i2c.h"
#include <avr/io.h>
#include <util/twi.h>


//TODO reset TWI on fail
uint8_t waitForTransmission(uint8_t timeOutUs){
	uint8_t counter = 0;
	while(!(TWCR & (1 << TWINT))){	//Wait for the end of the transmission
		if(++counter > timeOutUs)
			return 1;
	}
	return 0;
}


uint8_t i2c_start(uint8_t address, uint8_t mode){
	TWCR = 0;	//Clear control register

	TWCR = (1 << TWEN) | (1 << TWSTA) | (1 << TWINT);	//Send start condition
	while(!(TWCR & (1 << TWINT)));	//Wait for the end of the transmission

	//Mask prescaler bits to 0 and check whether start condition was successful
	if((TWSR & 0xF8) != TW_START)
		return 1;

	TWDR = address + mode;	//Load slave address + write/read bit
	//TWCR &= ~(1 << TWSTA);	//Clear TWSTA
	TWCR = (1 << TWINT) | (1 << TWEN);	//Start address transmission

	while(!(TWCR & (1 << TWINT)));	//Wait for the end of the transmission
	uint8_t status = TW_STATUS;//(TWSR & 0xF8); //Read status register

	//Check if the SLA+W/SLA+R has been transmitted and ACK received
	if((status != TW_MT_SLA_ACK) && (status != TW_MR_SLA_ACK))
		return 1;

	return 0;
}

uint8_t i2c_writeByte(uint8_t byte){
	TWDR = byte;	//Load byte into register
	TWCR = (1 << TWINT) | (1 << TWEN);	//Start transmission

	while(!(TWCR & (1 << TWINT)));	//Wait for the end of the transmission

	//Check if data was transmitted and ACK received
	if((TWSR & 0xF8) != TW_MT_SLA_ACK)
		return 1;

	return 0;
}

uint8_t i2c_readByteWithAck(){
	//Start receiving data and send ACK when received
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	waitForTransmission(150);	//Wait for the end of the transmission

	//Check if data was received and ACK transmitted
	if((TWSR & 0xF8) != TW_MR_DATA_ACK)
		return 1;

	return TWDR;
}

uint8_t i2c_readByteWithoutAck(){
	TWCR = (1 << TWINT) |(1 << TWEN);	//Start receiving data

	waitForTransmission(150);	//Wait for the end of the transmission

	//Check if data was received
	if((TWSR & 0xF8) != TW_MR_DATA_NACK)
		return 1;

	return TWDR;
}

void i2c_stop(){
	TWCR = (1 << TWINT) |(1 << TWEN) | (1 << TWSTO);	//Send stop condition
}

