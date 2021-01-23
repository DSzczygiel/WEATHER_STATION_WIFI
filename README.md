# WEATHER_STATION_WIFI

Weather station from my other repo extended by LoLin NodeMCU v3 module. The module is used to fetch time and weather from web APIs (weather data is fetched every 10 minutes and time every hour). For now, the weather station is showing date, time, data from sensors, current conditions, 4 hours and 4 days forecasts. Screens are switched using touch sensor. Time API is used to sync time, as cheap RTC module keeps losing some minutes. AVR and NodeMCU are connected the way, that AVR acts as a "client" and NodeMCU acts as a "server". AVR send requests (and retry the request in case of error) and the NodeMCU sends responses. Communication is based on UART and my own, very simple protocol for sending requests and responses. Everything is in testing phase. If there won't be any problems, I need to make PCB and build up some kind of case to hide the cable mess.
## Weather station
### Home scren
![station](/1.jpg "Home screen")
### Current day info
![station](/2.jpg "Current day info")
### 4 hours forecast
![station](/3.jpg "4 hours forecast")
### 4 days forecast
![station](/4.jpg "4 day forecast")

## Circuit diagram

![station](/circuit.png "Circuit diagram")
