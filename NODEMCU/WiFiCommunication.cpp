#include "WiFiCommunication.h"
#include <ArduinoJson.h>

const size_t weatherCapacity = 57 * JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(8) + JSON_ARRAY_SIZE(48) + 11 * JSON_OBJECT_SIZE(1) + 65 * JSON_OBJECT_SIZE(4) + 8 * JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(7) + 38 * JSON_OBJECT_SIZE(12) + 10 * JSON_OBJECT_SIZE(13) + JSON_OBJECT_SIZE(14) + 8 * JSON_OBJECT_SIZE(15) + 2350;
DynamicJsonDocument timeDoc(768);
DynamicJsonDocument weatherDoc(24576);
HTTPClient http;
WiFiClient client;
char responseBuff[18000];
int wifiStatusPin = 13; 	//D7
int timeErrPin = 0; 		//D3
int weatherErrPin = 2; 	//D4

const char *timeHost = "worldtimeapi.org";
const char *timeUrl = "/api/timezone/Europe/Warsaw";
const char *weatherUrl = "/data/2.5/onecall?lat=X&lon=Y&appid=SECRETID&lang=pl&units=metric&exclude=minutely";
const char *weatherHost = "api.openweathermap.org";

bool isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

//Connect to wifi
void initWifi(char *ssid, char *password) {
  pinMode(wifiStatusPin, OUTPUT);
  pinMode(timeErrPin, OUTPUT);
  pinMode(weatherErrPin, OUTPUT);

#ifdef COMMUNICATION_WIFI_DEBUG
  Serial.println(F("Initializing WiFi..."));
#endif
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(wifiStatusPin, HIGH);
#ifdef COMMUNICATION_WIFI_DEBUG
    Serial.println(F("Connecting..."));
#endif;
    delay(1000);
  }
#ifdef COMMUNICATION_WIFI_DEBUG
  Serial.print(F("Connected to "));
  Serial.println(ssid);
#endif
  digitalWrite(wifiStatusPin, LOW);
}

//Parse timestamp
void getHourFromTimestamp(long timestamp, long timeOffset, uint8_t *hour,
                          uint8_t *minutes) {
  int offsetHours = timeOffset / 3600;
  double mHour = fmod(timestamp / 3600.0, 24);
  double mMinutes = 60.0 * (mHour - ((int) mHour));
  *hour = (int) mHour + offsetHours;
  *minutes = (int) mMinutes;
}

int mpsToKmph(double mps) {
  return mps * 3.6;
}

//Translate degree number to word direction
String getWindDirection(int degree) {
  if (degree >= 0 && degree < 22.5)
    return "Pn";
  else if (degree >= 22.5 && degree < 67.5)
    return "Pn-Wsch";
  else if (degree >= 67.5 && degree < 112.5)
    return "Wsch";
  else if (degree >= 112.5 && degree < 157.5)
    return "Pd-Wsch";
  else if (degree >= 157.5 && degree < 202.5)
    return "Pd";
  else if (degree >= 202.5 && degree < 247.5)
    return "Pd-Zach";
  else if (degree >= 247.5 && degree < 292.5)
    return "Zach";
  else if (degree >= 292.5 && degree < 337.5)
    return "Pn-Zach";
  else
    return "Pn";
}

//Translate code from API to weather description.
String getWeatherDescriptionFromCode(int code) {
  if (code >= 200 && code < 300) {
    return "Burza";
  }
  if (code >= 300 && code < 400)
    return "Mzawka";

  if (code >= 500 && code < 600) {
    if (code == 500)
      return "Lekki desz.";
    else if (code > 501 && code < 505)
      return "Ulew. desz.";
    else if (code == 511)
      return "Marz. desz.";
    else
      return "Deszcz";
  }

  if (code >= 600 && code < 700) {
    if (code == 600 || code == 620)
      return "Lekki snieg";
    if (code == 602 || code == 622)
      return "Sniezyca";
    if (code >= 611 && code <= 616)
      return "Desz/snieg";
    else
      return "Snieg";
  }

  if (code >= 700 && code < 800) {
    if (code == 701 || code == 741)
      return "Mgla";
    else if (code == 721)
      return "Szron";
    else
      return "Zaniecz. pow";
  }

  if (code >= 800 && code < 900) {
    if (code == 800)
      return "Bezchmurnie";
    if (code == 801 || code == 802)
      return "Lek. zachm";
    if (code == 803 || code == 804)
      return "Pochmurno";
  }
  return "kodnieznany";
}

void toCharArray(String str, char *buff, int len) {
  str.toCharArray(buff, len);
}

//Parse weather JSON and save to weather struct
int parseWeatherJson(const char *json, struct weatherDataPacketStruct *wdps) {
#ifdef COMMUNICATION_WIFI_DEBUG
  Serial.println("ParsingWeather");
#endif
  DeserializationError err = deserializeJson(weatherDoc, json);

  if (err) {
#ifdef COMMUNICATION_WIFI_DEBUG
    Serial.print("ParsingWeather failed ");
#endif
    Serial.println(err.c_str());

    return -1;
  }

  getHourFromTimestamp(weatherDoc["current"]["sunset"].template as<long>(),
                       weatherDoc["timezone_offset"].as<int>(),
                       &wdps->currentWeatherData.sunsetHour,
                       &wdps->currentWeatherData.sunsetMinutes);

  getHourFromTimestamp(weatherDoc["current"]["sunrise"].as<long>(),
                       weatherDoc["timezone_offset"].as<int>(),
                       &wdps->currentWeatherData.sunriseHour,
                       &wdps->currentWeatherData.sunriseMinutes);

  wdps->currentWeatherData.windSpeed = mpsToKmph(
                                         weatherDoc["current"]["wind_speed"].as<double>());

  toCharArray(getWindDirection(weatherDoc["current"]["wind_deg"].as<int>()), wdps->currentWeatherData.windDirection, 8);

  wdps->currentWeatherData.feelsLikeTemp =
    weatherDoc["current"]["feels_like"].as<int>();
  wdps->currentWeatherData.humidity =
    weatherDoc["current"]["humidity"].as<int>();

  for (int i = 0; i < 4; i++) {
    wdps->hourlyWeatherData[i].id = i;
    JsonObject obj = weatherDoc["hourly"][i + 1];
    wdps->hourlyWeatherData[i].temperature = obj["temp"].as<int>();
    JsonObject obj2 = obj["weather"][0];
    String s = getWeatherDescriptionFromCode(obj2["id"].as<int>());
    toCharArray(s, wdps->hourlyWeatherData[i].description, 12);
  }

  for (int i = 0; i < 4; i++) {
    wdps->dailyWeatherData[i].id = i;
    JsonObject obj = weatherDoc["daily"][i + 1];
    wdps->dailyWeatherData[i].temperature = obj["temp"]["day"].as<int>();
    wdps->dailyWeatherData[i].temperatureNight =
      obj["temp"]["night"].as<int>();
    JsonObject obj2 = obj["weather"][0];
    toCharArray(getWeatherDescriptionFromCode(obj2["id"].as<int>()), wdps->dailyWeatherData[i].description, 12);
  }

  return 1;
}

int fetchWeatherJson(struct weatherDataPacketStruct *wdps) {
  client.flush();
  if (!isConnected())
    return -1;
#ifdef COMMUNICATION_WIFI_DEBUG
  Serial.println("FETCHING weather");
#endif

  if (!client.connect(weatherHost, 80)) {
#ifdef COMMUNICATION_WIFI_DEBUG
    Serial.println("connection failed");
#endif
    return -1;
  }
  client.setTimeout(15000);
  client.print(String("GET ") + weatherUrl + " HTTP/1.1\r\n" +
               "Host: " + weatherHost + "\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
#ifdef COMMUNICATION_WIFI_DEBUG
    Serial.println(line);
#endif
    if (line == "\r") {
#ifdef COMMUNICATION_WIFI_DEBUG
      Serial.println("headers received");
#endif
      break;
    }
  }
  long int time = millis();
  long int wait = 1000 * 10;
  int counter = 0;
  while ((time + wait) > millis()) {
    while (client.available()) {
      char c = client.read();  //Read Line by Line
      responseBuff[counter] = c;
      counter++;
#ifdef COMMUNICATION_WIFI_DEBUG
      Serial.print(c);
#endif
      if (c == '\0')
        continue;
    }
  }
#ifdef COMMUNICATION_WIFI_DEBUG
  Serial.println(counter);
#endif
  return parseWeatherJson(responseBuff, wdps);
}


//Parse datetime string from API 2020-12-26T16:33:28
void getDateTimeFromString(const char *dateTimeStr,
                           struct timeDataStruct *tds) {
  char buff[3];
  strncpy(buff, &dateTimeStr[2], 2);
  tds->year = atoi(buff);
  strncpy(buff, &dateTimeStr[5], 2);
  tds->month = atoi(buff);
  strncpy(buff, &dateTimeStr[8], 2);
  tds->day = atoi(buff);

  strncpy(buff, &dateTimeStr[11], 2);
  tds->hour = atoi(buff);
  strncpy(buff, &dateTimeStr[14], 2);
  tds->minute = atoi(buff);
  strncpy(buff, &dateTimeStr[17], 2);
  tds->second = atoi(buff);
}

int parseTimeJson(const char *json, struct timeDataStruct *tds) {
#ifdef COMMUNICATION_WIFI_DEBUG
  Serial.println("ParsingTime");
#endif
  DeserializationError err = deserializeJson(timeDoc, json);

  if (err) {
    return -1;
  }

  String dateTime = timeDoc["datetime"];
  getDateTimeFromString(dateTime.c_str(), tds);
  tds->dayOfWeek = timeDoc["day_of_week"].as<int>();

  return 1;
}

int fetchTimeJson(struct timeDataStruct *tds) {
  client.flush();
  if (!isConnected())
    return -1;
#ifdef COMMUNICATION_WIFI_DEBUG
  Serial.println("FETCHING TIME");
#endif
  if (!client.connect(timeHost, 80)) {
#ifdef COMMUNICATION_WIFI_DEBUG
    Serial.println("connection failed");
#endif
    return -1;
  }
  client.setTimeout(10000);
  client.print(String("GET ") + timeUrl + " HTTP/1.1\r\n" +
               "Host: " + timeHost + "\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
#ifdef COMMUNICATION_WIFI_DEBUG
    Serial.println(line);
#endif
    if (line == "\r") {
#ifdef COMMUNICATION_WIFI_DEBUG
      Serial.println("headers received");
#endif
      break;
    }
  }
  long int time = millis();
  long int wait = 1000 * 10;
  int counter = 0;
  while ((time + wait) > millis()) {
    while (client.available()) {
      char c = client.read();  //Read Line by Line
      responseBuff[counter] = c;
      counter++;
#ifdef COMMUNICATION_WIFI_DEBUG
      Serial.print(c);
#endif
      if (c == '\0')
        continue;
    }
  }
#ifdef COMMUNICATION_WIFI_DEBUG
  Serial.println(counter);
#endif
  return parseTimeJson(responseBuff, tds);
}
