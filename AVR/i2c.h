/*
 * i2c.h
 *
 *  Created on: 3 maj 2020
 *      Author: Daniel
 *
 *      I2C communication
 */

#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>

#define I2C_WRITE_MODE 0
#define I2C_READ_MODE 1

uint8_t i2c_start(uint8_t addres, uint8_t mode);
uint8_t i2c_writeByte(uint8_t byte);
uint8_t i2c_readByteWithAck();
uint8_t i2c_readByteWithoutAck();
void i2c_stop();


#endif /* I2C_H_ */
