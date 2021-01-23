/*
   CommunicationServer.cpp

    Created on: 27 gru 2020
        Author: Daniel
*/

#include "CommunicationServer.h"
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

extern "C" {
#include "checksum.h"
}
using namespace std;

uint8_t decodedMsgBuff[16];
SoftwareSerial softSerial(5, 4); //RX TX

//Init soft serial, because hardware is used for printing to console
void initCommunication() {
  pinMode(CONFIRM_RES_PIN, OUTPUT);
  digitalWrite(CONFIRM_RES_PIN, LOW);
  softSerial.begin(9600);
}

//Send sinal to AVR when sending response is finished
void confirmResponse() {
  digitalWrite(CONFIRM_RES_PIN, HIGH);
  delay(10);
  digitalWrite(CONFIRM_RES_PIN, LOW);
}

//Serialize weather structure to byte array
int serializeWeather(char weatherData[182], weatherDataPacketStruct *wdps) {
  int counter = 0;

  weatherData[counter++] = wdps->currentWeatherData.feelsLikeTemp;
  weatherData[counter++] = wdps->currentWeatherData.windSpeed;
  weatherData[counter++] = wdps->currentWeatherData.humidity;
  memcpy(&weatherData[counter], wdps->currentWeatherData.windDirection, 8);
  counter += 8;
  weatherData[counter++] = wdps->currentWeatherData.sunriseHour;
  weatherData[counter++] = wdps->currentWeatherData.sunriseMinutes;
  weatherData[counter++] = wdps->currentWeatherData.sunsetHour;
  weatherData[counter++] = wdps->currentWeatherData.sunsetMinutes;

  for (int i = 0; i < 4; i++) {
    weatherData[counter++] = wdps->hourlyWeatherData[i].id;
    weatherData[counter++] = wdps->hourlyWeatherData[i].temperature;
    memcpy(&weatherData[counter], wdps->hourlyWeatherData[i].description,
           12);
    counter += 12;
  }

  for (int i = 0; i < 4; i++) {
    weatherData[counter++] = wdps->dailyWeatherData[i].id;
    weatherData[counter++] = wdps->dailyWeatherData[i].temperature;
    weatherData[counter++] = wdps->dailyWeatherData[i].temperatureNight;
    memcpy(&weatherData[counter], wdps->dailyWeatherData[i].description,
           12);
    counter += 12;
  }
  return counter;
}

//Serialize time structure to byte array
int serializeTime(char *timeData, timeDataStruct *tds) {
  int counter = 0;

  timeData[counter++] = tds->hour;
  timeData[counter++] = tds->minute;
  timeData[counter++] = tds->second;
  timeData[counter++] = tds->day;
  timeData[counter++] = tds->month;
  timeData[counter++] = tds->year;
  timeData[counter++] = tds->dayOfWeek;

  return counter;
}

//Serialize error struct to byte array
int serializeError(char *errorData, struct errorStruct *error) {
  errorData[0] = error->code;
  strncpy(&errorData[1], error->description, 50);
  return 51;
}

//Encode response bytes
int encodeResponse(struct frame *frame, uint8_t *buff) {
  int i = 0;
  int j = 0;
  buff[i++] = FRAME_DELIM;    //Start frame byte
  buff[i++] = frame->code;    //Message code
  buff[i++] = 0;              //Initial length, will be updated at the end of encoding
  buff[i++] = 0;              //

  while (j < frame->len) {
    char byte = frame->message[j];
    if (byte == FRAME_DELIM || byte == ESCAPE) {  //Check if data byte is used as start byte or escape value
      buff[i] = ESCAPE;     //Add escape character before the data byte
      byte ^= XOR_VALUE;    //XOR the data byte
      i++;
    }
    buff[i] = byte;
    j++;
    i++;
  }
  buff[2] = (uint16_t) (i + 2) >> 8;    //Set length of message
  buff[3] = i + 2;                      //+ 2 because of CRC 16 at the end
  uint16_t crc = crc_16(buff, i);       //Compute CRC
  buff[i++] = crc;                      //Set CRC
  buff[i++] = (uint16_t) crc >> 8;

  return i;   //Return length of encoded message
}

//Write encoded response to Serial and send confirmation signal to AVR
void sendResponse(uint8_t *res, int len) {
  for (int i = 0; i < len; i++)
    softSerial.write(res[i]);

#ifdef COMMUNICATION_S_DEBUG
  for (int i = 0; i < len; i++) {
    Serial.print(res[i]);
    Serial.print(" ");
  }
#endif

  confirmResponse();
}

void sendTimeResponse(struct timeDataStruct *tds) {
  char timeBuff[7];
  uint8_t responseBuff[32];
  struct frame f;

  serializeTime(timeBuff, tds);
  f.code = CODE_TIME_RES;
  f.message = timeBuff;
  f.len = 7;
  int len = encodeResponse(&f, responseBuff);
#ifdef COMMUNICATION_S_DEBUG
  Serial.println(F("SENDING TIME RESPONSE"));
#endif
  sendResponse(responseBuff, len);
}

void sendWeatherResponse(struct weatherDataPacketStruct *wdps) {
  char weatherBuff[131];
  uint8_t responseBuff[160];
  struct frame f;

  serializeWeather(weatherBuff, wdps);
  f.code = CODE_WEATHER_RES;
  f.message = weatherBuff;
  f.len = 131;
  int len = encodeResponse(&f, responseBuff);
#ifdef COMMUNICATION_S_DEBUG
  Serial.println(F("SENDING WEATHER RESPONSE"));
#endif
  sendResponse(responseBuff, len);
}

void sendErrorResponse(struct errorStruct *es) {
  char errBuff[51];
  uint8_t responseBuff[100];
  struct frame f;

  serializeError(errBuff, es);
  f.code = CODE_ERROR_RES;
  f.message = errBuff;
  f.len = 51;
  int len = encodeResponse(&f, responseBuff);

#ifdef COMMUNICATION_S_DEBUG
  Serial.print(F("SENDING ERROR RESPONSE CODE: "));
  Serial.println(es->code);
#endif
  sendResponse(responseBuff, len);
}

//Decode request's byte array
int decodeRequest(uint8_t *bytes) {
  uint8_t foundStart = 0;
  uint8_t startIndex = 0;
  for (uint8_t i = 0; i < 256; i++) { //Find start of frame (for now, should be always at the beginning because of flushed Serial)
    if (bytes[i] == FRAME_DELIM) {
      foundStart = 1;
      startIndex = i;
      break;
    }
  }

  if (foundStart == 0)  //Check if start byte is found
    return -2;

  uint16_t len = (uint16_t) bytes[3] | (uint16_t) (bytes[2] << 8);
  uint16_t crc = crc_16(&bytes[startIndex], len);   //Compute CRC of received request

  if (crc != 0)   //Check if CRC is valid
    return -3;

  uint16_t j = 0;
  for (uint16_t i = 4; i < len - 2; i++) {  //Iterate through message body
    uint8_t byte = bytes[i];
    if (byte == ESCAPE) {           //Check if data byte is escape charactter
      i++;                          //Skip escape character
      byte = bytes[i] ^ XOR_VALUE;  //XOR next byte to gets it original value
    }
    decodedMsgBuff[j] = byte;
    j++;

  }

  int code = bytes[1];
  //Return decoded request code
  switch (code) {
    case CODE_TIME_REQ:
#ifdef COMMUNICATION_S_DEBUG
      Serial.println(F("RECEIVED TIME REQUEST"));
#endif
      return CODE_TIME_REQ;
    case CODE_WEATHER_REQ:
#ifdef COMMUNICATION_S_DEBUG
      Serial.println(F("RECEIVED WEATHER REQUEST"));
#endif
      return CODE_WEATHER_REQ;;
    default:
      break;
  }
}

int getRequest() {
  byte buffer[256];
  int read = softSerial.readBytes(buffer, 256); //Read bytes from Serial and save into buffer

#ifdef COMMUNICATION_S_DEBUG
  Serial.print(F("Received request bytes: "));
  for (int i = 0; i < read; i++)
    Serial.print(buffer[i]);
  Serial.println();
#endif

  if (read > 0) {   //If any bytes were read, flush the Serial and decode received bytes
    softSerial.flush();
    return decodeRequest(buffer);

  }
  return -1;
}
