/*
 * BMP280.c
 *
 *  Created on: 17 lip 2020
 *      Author: Daniel
 */

#include "BMP280.h"
#include "i2c.h"

uint16_t dig_T1;
int16_t dig_T2;
int16_t dig_T3;

uint16_t dig_P1;
int16_t dig_P2;
int16_t dig_P3;
int16_t dig_P4;
int16_t dig_P5;
int16_t dig_P6;
int16_t dig_P7;
int16_t dig_P8;
int16_t dig_P9;

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
BMP280_S32_t t_fine;
BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T) {
	BMP280_S32_t var1, var2, T;
	var1 = ((((adc_T >> 3) - ((BMP280_S32_t) dig_T1 << 1)))
			* ((BMP280_S32_t) dig_T2)) >> 11;
	var2 = (((((adc_T >> 4) - ((BMP280_S32_t) dig_T1))
			* ((adc_T >> 4) - ((BMP280_S32_t) dig_T1))) >> 12)
			* ((BMP280_S32_t) dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
BMP280_U32_t bmp280_compensate_P_int32(BMP280_S32_t adc_P) {
	BMP280_S32_t var1, var2;
	BMP280_U32_t p;
	var1 = (((BMP280_S32_t) t_fine) >> 1) - (BMP280_S32_t) 64000;
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((BMP280_S32_t) dig_P6);
	var2 = var2 + ((var1 * ((BMP280_S32_t) dig_P5)) << 1);
	var2 = (var2 >> 2) + (((BMP280_S32_t) dig_P4) << 16);
	var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3)
			+ ((((BMP280_S32_t) dig_P2) * var1) >> 1)) >> 18;
	var1 = ((((32768 + var1)) * ((BMP280_S32_t) dig_P1)) >> 15);
	if (var1 == 0) {
		return 0; // avoid exception caused by division by zero
	}
	p = (((BMP280_U32_t) (((BMP280_S32_t) 1048576) - adc_P) - (var2 >> 12)))
			* 3125;
	if (p < 0x80000000) {
		p = (p << 1) / ((BMP280_U32_t) var1);
	} else {
		p = (p / (BMP280_U32_t) var1) * 2;
	}
	var1 = (((BMP280_S32_t) dig_P9)
			* ((BMP280_S32_t) (((p >> 3) * (p >> 3)) >> 13))) >> 12;
	var2 = (((BMP280_S32_t) (p >> 2)) * ((BMP280_S32_t) dig_P8)) >> 13;
	p = (BMP280_U32_t) ((BMP280_S32_t) p + ((var1 + var2 + dig_P7) >> 4));
	return p;
}

void _bmp280_getCalibrationData() {
	int16_t data[12];
	int16_t tmp;
	i2c_start(BMP280_ADDRESS, I2C_WRITE_MODE);
	i2c_writeByte(BMP280_CALIB_DATA_REGISTER);
	i2c_start(BMP280_ADDRESS, I2C_READ_MODE);

	for (uint8_t i = 0; i < 11; i++) {
		tmp = i2c_readByteWithAck();
		data[i] = tmp;
		tmp = i2c_readByteWithAck();
		data[i] |= (tmp << 8);
	}
	tmp = i2c_readByteWithAck();
	data[11] = tmp;
	tmp = i2c_readByteWithoutAck();
	data[11] |= (tmp << 8);
	i2c_stop();

	dig_T1 = (uint16_t) data[0];
	dig_T2 = data[1];
	dig_T3 = data[2];

	dig_P1 = (uint16_t) data[3];
	dig_P2 = data[4];
	dig_P3 = data[5];
	dig_P4 = data[6];
	dig_P5 = data[7];
	dig_P6 = data[8];
	dig_P7 = data[9];
	dig_P8 = data[10];
	dig_P9 = data[11];
}

void bmp280_init() {
	uint8_t cfg = 0;
	uint8_t ctrl_meas = 0;
	cfg |= BMP280_FILTER_16 | BMP280_STANDBY_4000MS; //filter coefficient 16x, standby time 4s
	ctrl_meas |= BMP280_T_OVERSAMPLING_2X | BMP280_P_OVERSAMPLING_16X | BMP280_MODE_NORMAL;
	i2c_start(BMP280_ADDRESS, I2C_WRITE_MODE);
	i2c_writeByte(BMP280_CONFIG_REGISTER);
	i2c_writeByte(cfg);
	i2c_start(BMP280_ADDRESS, I2C_WRITE_MODE);
	i2c_writeByte(BMP280_CTRL_MEAS_REGISTER);
	i2c_writeByte(ctrl_meas);
	i2c_stop();

	_bmp280_getCalibrationData();
}

BMP280_S32_t bmp280_getData(struct bmp280_data *data) {
	BMP280_S32_t tempAdc = 0;
	BMP280_S32_t pressAdc = 0;

	i2c_start(BMP280_ADDRESS, I2C_WRITE_MODE);
	i2c_writeByte(BMP280_PRESSURE_REGISTER);
	i2c_start(BMP280_ADDRESS, I2C_READ_MODE);

	uint32_t tmp = i2c_readByteWithAck();
	pressAdc |= (tmp << 12);
	tmp = i2c_readByteWithAck();
	pressAdc |= (tmp << 4);
	tmp = i2c_readByteWithAck();
	pressAdc |= (tmp >> 4);

	tmp = i2c_readByteWithAck();
	tempAdc |= (tmp << 12);
	tmp = i2c_readByteWithAck();
	tempAdc |= (tmp << 4);
	tmp = i2c_readByteWithoutAck();
	tempAdc |= (tmp >> 4);
	i2c_stop();

	BMP280_S32_t temp = bmp280_compensate_T_int32(tempAdc);
	BMP280_U32_t press = bmp280_compensate_P_int32(pressAdc);

	data->tempInteger = temp / 100;
	data->tempFraction = temp % 100;
	data->pressureInteger = press / 100;
	data->pressureFraction = press % 100;
	return 0;
}
