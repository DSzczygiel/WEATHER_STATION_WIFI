#include "CommunicationClient.h"
#include "checksum.h"
#include "string.h"
#include "uart.h"
#include <util/delay.h>

#define BUFF_SIZE 160
struct weatherDataPacketStruct weatherData;
struct timeDataStruct timeData;
struct errorStruct errorData;
unsigned char decodedMsgBuff[BUFF_SIZE];

//Init UART, ports and interrupts
void initCommunication() {
	uart0_init(UART_BAUD_SELECT(9600, 8000000L));
	CONFIRM_REQ_DDR |= CONFIRM_REQ;
	CONFIRM_REQ_PORT &= ~CONFIRM_REQ;

	GOT_RES_DDR &= ~GOT_RES;
	GOT_RES_PORT &= ~GOT_RES;
	EICRA |= (1 << ISC01) || (1 << ISC00);	// Interrupt on rising edge on int0
	EIMSK |= (1 << INT0); 				// Enable int0 interrupt
}

void confirmRequest() {
	CONFIRM_REQ_PORT |= CONFIRM_REQ;
	_delay_ms(100);
	CONFIRM_REQ_PORT &= ~CONFIRM_REQ;
}

//Send request to NodeMCU
void sendRequest(unsigned char *data, uint16_t len) {
	for (uint8_t i = 0; i < len; i++)
		uart0_putc(data[i]);
	confirmRequest();
}

//Encode frame to byte array
int encodeRequest(struct frame *frame, uint8_t *buff) {
	int i = 0;
	int j = 0;
	buff[i++] = FRAME_DELIM;	//Frame start
	buff[i++] = frame->code;	//Message code
	buff[i++] = 0;			//Initial length. Will be updated at the end
	buff[i++] = 0;

	while (j < frame->len) {	//Iterate through message body
		char byte = frame->message[j];
		if (byte == FRAME_DELIM || byte == ESCAPE) {//Check if data byte is reserved character
			buff[i] = ESCAPE;	//Add escape character before data
			byte ^= XOR_VALUE;	//Xor data to change it's value
			i++;
		}
		buff[i] = byte;
		j++;
		i++;
	}
	buff[2] = (uint16_t) (i + 2) >> 8;	//Length of encode message
	buff[3] = i + 2;			//+2 because of CRC-16

	uint16_t crc = crc_16(buff, i);	//Compute and save CRC
	buff[i++] = crc;
	buff[i++] = (uint16_t) crc >> 8;

	return i;	//Return encoded message length
}

//Deserialize received weather bytes to weather struct
void deserializeWeatherData(uint8_t *bytes,
		struct weatherDataPacketStruct *wdps) {
	uint16_t i = 0;
	wdps->currentWeatherData.feelsLikeTemp = bytes[i++];
	wdps->currentWeatherData.windSpeed = bytes[i++];
	wdps->currentWeatherData.humidity = bytes[i++];
	memcpy(wdps->currentWeatherData.windDirection, &bytes[i], 8);
	i += 8;
	wdps->currentWeatherData.sunriseHour = bytes[i++];
	wdps->currentWeatherData.sunriseMinutes = bytes[i++];
	wdps->currentWeatherData.sunsetHour = bytes[i++];
	wdps->currentWeatherData.sunsetMinutes = bytes[i++];

	for (uint8_t j = 0; j < 4; j++) {
		wdps->hourlyWeatherData[j].id = bytes[i++];
		wdps->hourlyWeatherData[j].temperature = bytes[i++];
		memcpy(wdps->hourlyWeatherData[j].description, &bytes[i], 12);
		i += 12;
	}
	for (uint8_t j = 0; j < 4; j++) {
		wdps->dailyWeatherData[j].id = bytes[i++];
		wdps->dailyWeatherData[j].temperature = bytes[i++];
		wdps->dailyWeatherData[j].temperatureNight = bytes[i++];
		memcpy(wdps->dailyWeatherData[j].description, &bytes[i], 12);
		i += 12;
	}
}

//Deserialize time bytes to time struct
void deserializeTimeData(uint8_t *bytes, struct timeDataStruct *tds) {
	uint16_t i = 0;
	tds->hour = bytes[i++];
	tds->minute = bytes[i++];
	tds->second = bytes[i++];
	tds->day = bytes[i++];
	tds->month = bytes[i++];
	tds->year = bytes[i++];
	tds->dayOfWeek = bytes[i++];
}

//Deserialize error bytes to error struct
void deserializeError(uint8_t *bytes, struct errorStruct *err) {
	err->code = (int8_t) bytes[0];
	memcpy(err->description, &bytes[1], 50);
}

int8_t decodeResponse(uint8_t *bytes) {
	uint8_t foundStart = 0;
	uint8_t startIndex = 0;
	for (uint8_t i = 0; i < BUFF_SIZE; i++) {
		if (bytes[i] == FRAME_DELIM) { 		//Find start of frame (for now, should be always at the beginning because of flushed Serial)
			foundStart = 1;
			startIndex = i;
			break;
		}
	}

	if (foundStart == 0) //Check if start byte is found
		return -1;

	uint16_t len = (uint16_t) bytes[3] | (uint16_t) (bytes[2] << 8);
	uint16_t crc = crc_16(&bytes[startIndex], len);	//Compute CRC of received request

	if (crc != 0)
		return -1;

	uint16_t j = 0;
	for (uint16_t i = 4; i < len - 2; i++) {	//Iterate through message body
		uint8_t byte = bytes[i];
		if (byte == ESCAPE) {			//Check if data byte is escape charactter
			i++;				//Skip escape character
			byte = bytes[i] ^ XOR_VALUE;	//XOR next byte to gets it original value
		}
		decodedMsgBuff[j] = byte;
		j++;
	}

	int8_t code = bytes[1];
	//Return decoded request code
	switch (code) {
	case CODE_WEATHER_RES:
		deserializeWeatherData(decodedMsgBuff, &weatherData);
		return code;
	case CODE_TIME_RES:
		deserializeTimeData(decodedMsgBuff, &timeData);
		return code;
	case CODE_ERROR_RES:
		deserializeError(decodedMsgBuff, &errorData);
		return code;
	default:
		return -1;
	}
	return -1;
}

//Read response bytes from UART and save in buffer
uint8_t getResponse() {
	unsigned char buff[BUFF_SIZE];
	uint16_t data;

	for (uint8_t i = 0; i < BUFF_SIZE; i++) {
		data = uart0_getc();
		if (((data >> 8) & 0x00FF) == UART_NO_DATA)
			break;
		buff[i] = data;
	}
	uart0_flush();
	return decodeResponse(buff);
}

void sendTimeRequest() {
	uint8_t requestBuff[16];
	struct frame f;
	f.code = CODE_TIME_REQ;
	f.len = 0;
	uint16_t len = encodeRequest(&f, requestBuff);

	sendRequest(requestBuff, len);
}

void sendWeatherRequest() {
	uint8_t requestBuff[16];
	struct frame f;
	f.code = CODE_WEATHER_REQ;
	f.len = 0;
	uint16_t len = encodeRequest(&f, requestBuff);

	sendRequest(requestBuff, len);
}
