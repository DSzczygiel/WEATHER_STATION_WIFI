/*
 * CommunicationClient.h
 *
 *  Created on: 27 gru 2020
 *      Author: Daniel
 *
 *      Communication with ESP8266 using simple frame based protocol.
 *      Frame consist of header containg info about message,
 *      fixed length (depending on message type) message body,
 *      and CRC-16 value;
 *      Communication is done using UART. Structures reresenting
 *      frame and messages are serialized/deserialized to/from byte arrays
 *      before sending via UART.
 *      Protocol need a lot of improvements like variable length messages
 *      or queuing requests. For now it just works.
 */

#ifndef COMMUNICATIONCLIENT_H_
#define COMMUNICATIONCLIENT_H_

#include "CommunicationData.h"

#define CONFIRM_REQ_DDR DDRB
#define CONFIRM_REQ_PORT PORTB
#define CONFIRM_REQ (1 << PB5)

#define GOT_RES_DDR DDRD
#define GOT_RES_PORT PORTD
#define GOT_RES_PIN PIND
#define GOT_RES (1 << PD2)

extern struct weatherDataPacketStruct weatherData;
extern struct timeDataStruct timeData;
extern struct errorStruct errorData;

void initCommunication();
void sendTimeRequest();
void sendWeatherRequest();
uint8_t getResponse();
#endif /* COMMUNICATIONCLIENT_H_ */
