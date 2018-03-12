#ifndef TABLE_SERVER_H
#define TABLE_SERVER_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>

#include "LedTablePixels.h"
#include "LedTableIrPanel.h"
#include "Logger.h"
#include "../config/config.h"


void toogleLed();

class TableServer {
  enum PACKET_TYPE {
    PING = 0x0,
    PONG = 0x1,
    SET_MATRIX = 0x2,
    GET_TOUCHSCREEN = 0x3,
    GET_TOUCHSCREEN_RAW = 0x4,
    GET_REFFERENCE_TABLE = 0x5,
    TOUCHSCREEN = 0x6,
    TOUCHSCREEN_RAW = 0x7,
    REFFERENCE_TABLE = 0x8,
    CALIBRATE = 0x9,
    RESET = 0xa,
    PANIC = 0x45,
    ARTNET = 0x41,
  };

  uint8_t incomingPacket[400];  // buffer for incoming packet
  bool connected = false;
  WiFiUDP Udp;
  LedTablePixels pixels = LedTablePixels(pixelsNumb, pixelsPin, NEO_GRB + NEO_KHZ800);
  LedTableIrPanel ledTableIrPanel = LedTableIrPanel(latchPin, clockPin, dataPin, sensorsPin, 4);

  void handlePacket(uint8_t packet[], int len);

public:
  void init();

  void handle();

  inline bool isConnected() { return connected; }
};

#endif // TABLE_SERVER_H
