/*
 * CommunicationData.h
 *
 *  Created on: 27 gru 2020
 *      Author: Daniel
 *      
 *      Constants and structs used in communication
 */

#ifndef COMMUNICATIONDATA_H_
#define COMMUNICATIONDATA_H_

#include <inttypes.h>

#define ESCAPE 0x7D
#define FRAME_DELIM 0x7E
#define XOR_VALUE 0x20

#define CODE_WEATHER_REQ 1
#define CODE_TIME_REQ 0
#define CODE_TIME_RES 2
#define CODE_WEATHER_RES 3
#define CODE_ERROR_RES 4

#define ERROR_FETCH_WEATHER -10
#define ERROR_FETCH_TIME -20

struct frame{
	uint8_t code;
	uint16_t len;
	char *message;
	uint16_t crc;
};

struct currentData {
	int8_t feelsLikeTemp;
	uint8_t windSpeed;
	uint8_t humidity;
	char windDirection[8];
	uint8_t sunriseHour;
	uint8_t sunriseMinutes;
	uint8_t sunsetHour;
	uint8_t sunsetMinutes;
};

struct weatherData {
	uint8_t id;
	int8_t temperature;
	int8_t temperatureNight;
	char description[12];
};
struct weatherDataPacketStruct {
	struct currentData currentWeatherData;
	struct weatherData dailyWeatherData[4];
	struct weatherData hourlyWeatherData[4];
};

struct errorStruct {
  int8_t code;
  char description[50];
};

struct timeDataStruct {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t dayOfWeek;
};

#endif /* COMMUNICATIONDATA_H_ */
