/*
    LoLin NodeMCU v3 main file

*/

#include <ArduinoJson.h>
#include <math.h>
#include "CommunicationServer.h"
#include "WiFiCommunication.h"

//#define MAIN_DEBUG

volatile bool gotRequest = false;
struct timeDataStruct timeData;
struct weatherDataPacketStruct weatherData;
struct errorStruct error;

void setup() {
  Serial.begin(115200);
  initCommunication();
  initWifi("SomeName", "SomePassword");
  pinMode(GOT_REQ_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(GOT_REQ_PIN), onRequestDetected, RISING); //Enable interrupt on rising edge
}

void loop() {
  if (WiFi.status() == WL_CONNECTED)
    digitalWrite(wifiStatusPin, LOW);
  else
    digitalWrite(wifiStatusPin, HIGH);

#ifdef MAIN_DEBUG
  Serial.println("LOOP ");
#endif

  if (gotRequest) {
#ifdef MAIN_DEBUG
    Serial.println("REQ DETECTED");
#endif
    gotRequest = false;
    int code = getRequest();
    if (code == CODE_TIME_REQ) {
      int i = fetchTimeJson(&timeData);
      if (i > 0) {
        digitalWrite(timeErrPin, LOW);
        sendTimeResponse(&timeData);
      } else {
        digitalWrite(timeErrPin, HIGH);
        error.code = ERROR_FETCH_TIME;
        strcpy(error.description, "Time error");
        sendErrorResponse(&error);
      }

    } else if (code == CODE_WEATHER_REQ) {
      int i = fetchWeatherJson(&weatherData);
      if (i > 0) {
        digitalWrite(weatherErrPin, LOW);
        sendWeatherResponse(&weatherData);
      } else {
        digitalWrite(weatherErrPin, HIGH);
        error.code = ERROR_FETCH_WEATHER;
        strcpy(error.description, "Weather error");
        sendErrorResponse(&error);
      }
    }
  }
  delay(1000);
}

ICACHE_RAM_ATTR void onRequestDetected() {
  gotRequest = true;
}
