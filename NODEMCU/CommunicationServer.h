/*
 * CommunicationServer.h
 *
 *  Created on: 27 gru 2020
 *      Author: Daniel
 *      
 *      Communication with AVR using simple frame based protocol.
 *      Frame consist of header containg info about message, 
 *      fixed length (depending on message type) message body,
 *      and CRC-16 value;
 *      Communication is done using UART. Structures reresenting
 *      frame and messages are serialized/deserialized to/from byte arrays
 *      before sending via UART.
 *      Protocol need a lot of improvements like variable length messages
 *      or queuing requests. For now it just works.
 */

#ifndef COMMUNICATIONSERVER_H_
#define COMMUNICATIONSERVER_H_

#include "CommunicationData.h"

//#define COMMUNICATION_S_DEBUG

#define GOT_REQ_PIN 14 	//D5
#define CONFIRM_RES_PIN 12 	//D6

void initCommunication();
void sendTimeResponse(struct timeDataStruct *tds);
void sendWeatherResponse(struct weatherDataPacketStruct *wdps);
void sendErrorResponse(struct errorStruct *es);
int getRequest();

#endif /* COMMUNICATIONSERVER_H_ */
