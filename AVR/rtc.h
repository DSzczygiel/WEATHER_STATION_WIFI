/*
 * rtc.h
 *
 *  Created on: 19 lip 2020
 *      Author: Daniel
 *
 *      DS1307 RTC module
 */

#ifndef RTC_H_
#define RTC_H_

#include <avr/io.h>

#define DS1307_ADDR 0b11010000
#define SECOND_ADDR 0x00
#define MINUTES_ADDR 0x01
#define HOUR_ADDR 0x02

extern char days[7][15];

struct rtc_data{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t day;
	uint8_t dayOfWeek;
	char* dayOfWeekName;
	uint8_t month;
	uint16_t year;
};
void rtc_Init();
void rtc_getTime(struct rtc_data *time);
void rtcSetTime(struct rtc_data *time);

#endif /* RTC_H_ */
