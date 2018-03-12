#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

extern const char* ssid;

extern const char* password;


extern const char* hostname;

extern const int CONNECTION_TIMEOUT_TIME;

extern const unsigned int localUdpPort;  // local port to listen on 6456 for artnet 6660 for ledTable
//Analog pin connected to sensors
extern const byte sensorsPin;

//Pin connected to built in led
extern const byte ledPin;

//Pin connected to DATA_IN of pixels
extern const byte pixelsPin;

//Nmber of pixels
extern const byte pixelsNumb;

//Pin connected to ST_CP of 74HC595
extern const byte latchPin;

//Pin connected to SH_CP of 74HC595
extern const byte clockPin;

//Pin connected to DS of 74HC595
extern const byte dataPin;

#endif // CONFIG_H
