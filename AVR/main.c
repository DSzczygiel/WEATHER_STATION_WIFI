/*
 * main.c
 *
 *  Created on: 3 maj 2020
 *      Author: Daniel
 *
 *		Weather station with date, time,
 *		indoor and outdoor temperature,
 *		humidity and air pressure.
 *
 *		Extended by current weather data,
 *		4 hours and 4 days forecast fetched
 *		from NodeMCU v3 module.
 *
 *		uC: Atmega328P @ 8MHz
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include "LCD.h"
#include "DS18B20.h"
#include "DHT11.h"
#include "rtc.h"
#include "BMP280.h"
#include "brightnessControl.h"
#include "CommunicationClient.h"
#include "CommunicationData.h"

#define SCREENS_NR 4
#define MAX_RETRIES 3

struct ds18b20_data externalTemp;
struct dht11_data tempHumidity;
struct rtc_data time;
struct bmp280_data tempPressure;
volatile uint8_t gotResponse = 0;
volatile uint8_t currentScreen = 0;
volatile uint8_t screenShowTime = 0;
uint8_t fetchWeatherRequests = 0;
uint8_t fetchTimeRequests = 0;

void updateSensors();
void updateTime();
void printSensorsScreen(char[24], char);
void printCurrentWeatherScreen(char[24]);
void printHourlyWeatherScreen(char[24]);
void printDailyWeatherScreen(char[24]);
void onResponse();

int main() {
	uint16_t counter = 0;
	char text[24];
	char timeColon = ' ';

	ds18b20_init();
	dht11_init();
	rtc_Init();
	bmp280_init();
	lcd_init();
	bcInit();
	initCommunication();

	DDRD &= ~(1 << PD3);	// Touch button as input
	EICRA |= (1 << ISC11);	// Interrupt on falling edge on INT1
	EIMSK |= (1 << INT1);	// Enable INT1 interrupt

	updateSensors();
	lcd_clear();

	sei();

	while (1) {
		//Blink colon every second
		if (counter % 2 == 0) {
			timeColon = ' ';
		} else {
			timeColon = ':';
		}

		if (counter % 5 == 0) {
			updateTime();
			adjustBrightness();
		}

		if (counter % 58 == 0) {
			updateSensors();
			lcd_clear();
		}

		if (counter % 599 == 0) {
			fetchWeatherRequests = 0;
			sendWeatherRequest();
		}

		if (counter >= 3600) {
			fetchTimeRequests = 0;
			sendTimeRequest();
			counter = 0;

		}

		if (gotResponse) {
			gotResponse = 0;
			onResponse();
		}

		counter++;
		
		//Turn back to home screen after 10 seconds of inactivity
		if (++screenShowTime >= 10) {
			screenShowTime = 0;
			currentScreen = 0;
		}

		switch (currentScreen) {
		case 0:
			printSensorsScreen(text, timeColon);
			break;
		case 1:
			printCurrentWeatherScreen(text);
			break;
		case 2:
			printHourlyWeatherScreen(text);
			break;
		case 3:
			printDailyWeatherScreen(text);
			break;
		}
		
		_delay_ms(1000);
	}
}

void printSensorsScreen(char text[24], char timeColon) {
	lcd_clear();
	lcd_goTo(9, 0);
	sprintf(text, "%02u%c%02u", time.hour, timeColon, time.minute);
	lcd_write(text);
	lcd_goTo(4, 1);
	sprintf(text, "%02u/%02u/20%02u, %s", time.day, time.month, time.year,
			time.dayOfWeekName);
	lcd_write(text);

	lcd_goTo(0, 2);
	sprintf(text, "%c:% 3d,%01u%cC", LCD_OUT_TEMP_CHAR,
			externalTemp.tempInteger, externalTemp.tempFraction,
			LCD_DEG_CHAR);
	lcd_write(text);

	lcd_goTo(11, 2);
	sprintf(text, "%c:%c%d,%01u%cC", LCD_IN_TEMP_CHAR,
			((tempPressure.tempInteger < 0) ? '-' : ' '),
			abs(tempPressure.tempInteger), tempPressure.tempFraction / 10,
			LCD_DEG_CHAR);

	lcd_write(text);
	lcd_goTo(0, 3);
	sprintf(text, "%c: %u%%     %c: %u,%uhPa", LCD_HUM_CHAR,
			tempHumidity.rhInteger,
			LCD_PRESS_CHAR, tempPressure.pressureInteger,
			tempPressure.pressureFraction);
	lcd_write(text);
}

void printCurrentWeatherScreen(char text[24]) {
	lcd_clear();
	lcd_goTo(10, 0);
	lcd_write("Dzis");

	lcd_goTo(0, 1);
	sprintf(text, "Wsch: %02u:%02u  Zach: %02u:%02u",
			weatherData.currentWeatherData.sunriseHour,
			weatherData.currentWeatherData.sunriseMinutes,
			weatherData.currentWeatherData.sunsetHour,
			weatherData.currentWeatherData.sunsetMinutes);
	lcd_write(text);

	lcd_goTo(0, 2);
	sprintf(text, "T.od:% 3d%cC", weatherData.currentWeatherData.feelsLikeTemp,
	LCD_DEG_CHAR);
	lcd_write(text);

	lcd_goTo(13, 2);
	sprintf(text, "Wilg: %u%%", weatherData.currentWeatherData.humidity);
	lcd_write(text);

	lcd_goTo(0, 3);
	sprintf(text, "Wiatr: %dkm/h %s", weatherData.currentWeatherData.windSpeed,
			weatherData.currentWeatherData.windDirection);
	lcd_write(text);
}

void printHourlyWeatherScreen(char text[24]) {
	lcd_clear();
	lcd_goTo(0, 0);
	int8_t temp = weatherData.hourlyWeatherData[0].temperature;
	uint8_t hour = time.hour + 1;
	sprintf(text, "%-2d: % 3d%cC %s", hour > 23 ? hour - 24 : hour, temp,
	LCD_DEG_CHAR, weatherData.hourlyWeatherData[0].description);
	lcd_write(text);
	lcd_goTo(0, 1);
	temp = weatherData.hourlyWeatherData[1].temperature;
	hour = time.hour + 2;
	sprintf(text, "%-2d: % 3d%cC %s", hour > 23 ? hour - 24 : hour, temp,
	LCD_DEG_CHAR, weatherData.hourlyWeatherData[1].description);
	lcd_write(text);
	lcd_goTo(0, 2);
	temp = weatherData.hourlyWeatherData[2].temperature;
	hour = time.hour + 3;
	sprintf(text, "%-2d: % 3d%cC %s", hour > 23 ? hour - 24 : hour, temp,
	LCD_DEG_CHAR, weatherData.hourlyWeatherData[2].description);
	lcd_write(text);
	lcd_goTo(0, 3);
	temp = weatherData.hourlyWeatherData[3].temperature;
	hour = time.hour + 4;
	sprintf(text, "%-2d: % 3d%cC %s", hour > 23 ? hour - 24 : hour, temp,
	LCD_DEG_CHAR, weatherData.hourlyWeatherData[3].description);
	lcd_write(text);
}

void printDailyWeatherScreen(char text[24]) {
	lcd_clear();
	int8_t temp = weatherData.dailyWeatherData[0].temperature;
	int8_t tempNight = weatherData.dailyWeatherData[0].temperatureNight;
	uint8_t dayNr = time.dayOfWeek + 1 - 1;
	lcd_goTo(0, 0);
	sprintf(text, "%-3s % 3d /% 3d %s",
			dayNr > 6 ? days[dayNr - 7] : days[dayNr], temp, tempNight,
			weatherData.dailyWeatherData[0].description);
	lcd_write(text);
	temp = weatherData.dailyWeatherData[1].temperature;
	tempNight = weatherData.dailyWeatherData[1].temperatureNight;
	dayNr = time.dayOfWeek + 2 - 1;
	lcd_goTo(0, 1);
	sprintf(text, "%-3s % 3d /% 3d %s",
			dayNr > 6 ? days[dayNr - 7] : days[dayNr], temp, tempNight,
			weatherData.dailyWeatherData[1].description);
	lcd_write(text);
	temp = weatherData.dailyWeatherData[2].temperature;
	tempNight = weatherData.dailyWeatherData[2].temperatureNight;
	dayNr = time.dayOfWeek + 3 - 1;
	lcd_goTo(0, 2);
	sprintf(text, "%-3s % 3d /% 3d %s",
			dayNr > 6 ? days[dayNr - 7] : days[dayNr], temp, tempNight,
			weatherData.dailyWeatherData[2].description);
	lcd_write(text);
	temp = weatherData.dailyWeatherData[3].temperature;
	tempNight = weatherData.dailyWeatherData[3].temperatureNight;
	dayNr = time.dayOfWeek + 4 - 1;
	lcd_goTo(0, 3);
	sprintf(text, "%-3s % 3d /% 3d %s",
			dayNr > 6 ? days[dayNr - 7] : days[dayNr], temp, tempNight,
			weatherData.dailyWeatherData[3].description);
	lcd_write(text);
}

//Process response from  NodeMCU and retry request on error
void onResponse() {
	int8_t code = getResponse();
	if (code == CODE_ERROR_RES) {
		if (errorData.code == ERROR_FETCH_TIME
				&& fetchTimeRequests < MAX_RETRIES) {
			fetchTimeRequests++;
			sendTimeRequest();
		} else if (errorData.code == ERROR_FETCH_WEATHER
				&& fetchWeatherRequests < MAX_RETRIES) {
				fetchWeatherRequests++;
			sendWeatherRequest();
		}
		return;
	} else if (code == CODE_TIME_RES) {
		time.minute = timeData.minute;
		time.hour = timeData.hour;
		time.second = timeData.second;
		time.day = timeData.day;
		time.month = timeData.month;
		time.year = timeData.year;
		time.dayOfWeek = timeData.dayOfWeek+1;
		time.dayOfWeekName = days[timeData.dayOfWeek];
		rtcSetTime(&time);
	}
}

void updateSensors() {
	dht11_getData(&tempHumidity);
	ds18b20_getTemperature(&externalTemp);
	bmp280_getData(&tempPressure);
}

void updateTime() {
	rtc_getTime(&time);
}

//Received response from NodeMCU
ISR(INT0_vect) {
	gotResponse = 1;
}

//Change screen on button touch
ISR(INT1_vect) {
	if (++currentScreen >= SCREENS_NR)
		currentScreen = 0;
	screenShowTime = 0;

}
