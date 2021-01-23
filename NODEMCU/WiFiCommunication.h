/*
 * Communication with time and weather API.
 * Parsing json responses and saving it
 * to appropriate struct.
 * 
 */

#ifndef WIFICOMMUNICATION_H_
#define WIFICOMMUNICATION_H_

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "CommunicationData.h"

//#define COMMUNICATION_WIFI_DEBUG

extern int wifiStatusPin; //D7
extern int timeErrPin; //D3
extern int weatherErrPin; //D4

void initWifi(char *ssid, char *password);
int fetchTimeJson(struct timeDataStruct *tds);
int fetchWeatherJson(struct weatherDataPacketStruct *wdps);

#endif /* WIFICOMMUNICATION_H_ */
