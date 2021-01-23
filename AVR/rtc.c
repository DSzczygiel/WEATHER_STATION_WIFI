/*
 * rtc.c
 *
 *  Created on: 19 lip 2020
 *      Author: Daniel
 */

#include "rtc.h"
#include "i2c.h"

char days[7][15] = {"Nie", "Pon", "Wto", "Sr", "Czw", "Pt", "Sob"};

uint8_t bcdToDecimal(uint8_t bcd){
	return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t decimalToBcd(uint8_t decimal){
	return ((decimal / 10) << 4) + (decimal % 10);
}

/*
 * Start clock by setting CH bit and set SCK clock to 100kHz
 */
void rtc_Init() {
	TWBR = 32;
	//i2c_init();
	i2c_start(DS1307_ADDR, I2C_WRITE_MODE);
	i2c_writeByte(SECOND_ADDR);
	i2c_start(DS1307_ADDR, I2C_READ_MODE);

	uint8_t tmp = i2c_readByteWithoutAck();
	i2c_start(DS1307_ADDR, I2C_WRITE_MODE);
	i2c_writeByte(SECOND_ADDR);
	i2c_writeByte(tmp & 0b01111111);
}

/*
 *Read time from DS1307 and save in given struct
 */
void rtc_getTime(struct rtc_data *time) {
	i2c_start(DS1307_ADDR, I2C_WRITE_MODE);
	i2c_writeByte(SECOND_ADDR);
	i2c_start(DS1307_ADDR, I2C_READ_MODE);

	uint8_t tmp = i2c_readByteWithAck();
	time->second = bcdToDecimal(tmp);

	tmp = i2c_readByteWithAck();
	time->minute = bcdToDecimal(tmp);

	tmp = i2c_readByteWithAck();
	time->hour = bcdToDecimal(tmp);

	tmp = i2c_readByteWithAck();
	time->dayOfWeek = bcdToDecimal(tmp);
	time->dayOfWeekName = days[bcdToDecimal(tmp)-1];

	tmp = i2c_readByteWithAck();
	time->day = bcdToDecimal(tmp);

	tmp = i2c_readByteWithAck();
	time->month = bcdToDecimal(tmp);

	tmp = i2c_readByteWithoutAck();
	time->year = bcdToDecimal(tmp);

	i2c_stop();
}

void rtcSetTime(struct rtc_data *time) {
	i2c_start(DS1307_ADDR, I2C_WRITE_MODE);
	i2c_writeByte(SECOND_ADDR);

	i2c_writeByte(decimalToBcd(time->second));
	i2c_writeByte(decimalToBcd(time->minute));
	i2c_writeByte(decimalToBcd(time->hour));
	i2c_writeByte(decimalToBcd(time->dayOfWeek));
	i2c_writeByte(decimalToBcd(time->day));
	i2c_writeByte(decimalToBcd(time->month));
	i2c_writeByte(decimalToBcd(time->year));

	i2c_stop();
}
