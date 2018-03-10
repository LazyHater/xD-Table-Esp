/*
Created on: 20.04.2017
Author: Wakabajaszi
Soft ver. 2.0
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "LedTablePixels.h"
#include "LedTableIrPanel.h"

//Analog pin connected to sensors
const byte sensorsPin = A0;

//Pin connected to built in led
const byte ledPin = D4;

//Pin connected to DATA_IN of pixels
const byte pixelsPin = D8;

//Nmber of pixels
const byte pixelsNumb = 100;

//Pin connected to ST_CP of 74HC595
const byte latchPin = D2;

//Pin connected to SH_CP of 74HC595
const byte clockPin = D1;

//Pin connected to DS of 74HC595
const byte dataPin = D0;

#define V_LEVEL 10 // Verbose level
#define DEBUG 10
#define INFO 20
#define WARNING 30
#define ERROR 40
#define CRITICAL 50

#define WROC_B

#ifdef WROC_B
const char* ssid = "NETIASPOT-B32900";
const char* password = "7hkudpspjc89";
#elif defined KAL
const char* ssid = "MajkaCafe";
const char* password = "izarobert";
#endif

LedTablePixels pixels = LedTablePixels(pixelsNumb, pixelsPin, NEO_GRB + NEO_KHZ800);
LedTableIrPanel ledTableIrPanel = LedTableIrPanel(latchPin, clockPin, dataPin, sensorsPin, 4);
WiFiUDP Udp;

const unsigned int localUdpPort = 6454;  // local port to listen on 6456 for artnet 6660 for ledTable
uint8_t incomingPacket[400];  // buffer for incoming packet

#define CONNECTION_TIMEOUT_TIME 5000
bool connected = false;

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

void toogleLed() {
  static bool state = false;
  state = !state;
  digitalWrite(ledPin, (state) ? HIGH : LOW);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(pixelsPin, OUTPUT);

  pixels.begin();
  pixels.enableGammaCorrection();
  pixels.show(); // Initialize all pixels to 'off'


  Serial.begin(115200);
  Serial.setDebugOutput(true);
  #if V_LEVEL <= INFO
  Serial.printf("Connecting to %s ", ssid);
  #endif
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    toogleLed();
    delay(250);
    toogleLed();
    #if V_LEVEL <= INFO
    Serial.print(".");
    #endif
    delay(250);
  }

  #if V_LEVEL <= INFO
  Serial.println("connected");
  #endif

  if(Udp.begin(localUdpPort) == 1) {

    #if V_LEVEL <= INFO
    Serial.printf("Now listening at IP %s, UDP port %d\n",
                  WiFi.localIP().toString().c_str(), localUdpPort);
    #endif
  }
  else {
    #if V_LEVEL <= CRITICAL
    Serial.println("Failed to open UDP port! Rebooting...");
    #endif
    ESP.restart();
  }
}

void handlePacket(uint8_t packet[], int len){
  #if V_LEVEL <= DEBUG
  Serial.printf("Got packet (0x%x) ", incomingPacket[0]);
  #endif
  switch (incomingPacket[0]) {
    case CALIBRATE: {
      #if V_LEVEL <= INFO
      Serial.println("Got CALIBRATE packet, calibrating...");
      #endif
      ledTableIrPanel.createRefferenceTable();
    }
    break;

    case PING: {
      #if V_LEVEL <= INFO
      Serial.println("Got PING, sending back PONG.");
      #endif

      if(Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()) != 1) {
        #if V_LEVEL <= WARNING
        Serial.printf("Failed to begin packet PONG to: %s on port %d\n",
        Udp.remoteIP().toString().c_str(), Udp.remotePort());
        #endif
      }

      Udp.write(PONG);

      if(Udp.endPacket() != 1) {
        #if V_LEVEL <= WARNING
        Serial.printf("Failed to end packet PONG to: %s on port %d\n",
        Udp.remoteIP().toString().c_str(), Udp.remotePort());
        #endif
      }
    }
    break;

    case SET_MATRIX: {
      #if V_LEVEL <= DEBUG
      Serial.printf("Got SET_MATRIX of length %i and %i pixels\n", len, incomingPacket[1]);
      #endif

      byte* tmp = &(incomingPacket[2]);

      for (int i = 0; i < incomingPacket[1]; i++)
      pixels.setPixelColor(i, tmp[i * 3], tmp[i * 3 + 1], tmp[i * 3 + 2]);

      pixels.show();
    }
    break;

    case GET_TOUCHSCREEN: {
      #if V_LEVEL <= DEBUG
      Serial.println("Got GET_TOUCHSCREEN collecting data...");
      #endif

      ledTableIrPanel.collectData();
      ledTableIrPanel.convertReadingsToBoll();

      if(Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()) != 1) {
        #if V_LEVEL <= WARNING
        Serial.printf("Failed to begin packet GET_TOUCHSCREEN to %s on port %d\n",
          Udp.remoteIP().toString().c_str(), Udp.remotePort());
        #endif
      }

      Udp.write((byte)TOUCHSCREEN);
      Udp.write(ledTableIrPanel.getRaw(), 100);

      if(Udp.endPacket() != 1) {
        #if V_LEVEL <= WARNING
        Serial.printf("Failed to end packet GET_TOUCHSCREEN to: %s on port %d\n",
                      Udp.remoteIP().toString().c_str(), Udp.remotePort());
        #endif
      }
    }
    break;

    case GET_TOUCHSCREEN_RAW: {
      #if V_LEVEL <= DEBUG
      Serial.println("Got GET_TOUCHSCREEN_RAW collecting data...");
      #endif
      ledTableIrPanel.collectData();

      if(Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()) != 1) {
        #if V_LEVEL <= WARNING
        Serial.printf("Failed to begin packet GET_TOUCHSCREEN_RAW to: %s on port %d\n",
        Udp.remoteIP().toString().c_str(), Udp.remotePort());
        #endif
        return;
      }

      Udp.write((byte)TOUCHSCREEN_RAW);
      Udp.write(ledTableIrPanel.getRaw(), 100);

      if(Udp.endPacket() != 1) {
        #if V_LEVEL <= WARNING
        Serial.printf("Failed to end packet GET_TOUCHSCREEN_RAW to: %s on port %d\n",
        Udp.remoteIP().toString().c_str(), Udp.remotePort());
        #endif
        return;
      }
    }
    break;

    case GET_REFFERENCE_TABLE: {
      #if V_LEVEL <= DEBUG
      Serial.println("Got GET_REFFERENCE_TABLE");
      #endif

      uint8_t buff[100];
      ledTableIrPanel.getRefferenceTable(buff);

      if(Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()) != 1) {
        #if V_LEVEL <= WARNING
        Serial.printf("Failed to begin packet GET_REFFERENCE_TABLE to: %s on port %d\n",
        Udp.remoteIP().toString().c_str(), Udp.remotePort());
        #endif
        return;
      }

      Udp.write((byte)REFFERENCE_TABLE);
      Udp.write(buff, 100);

      if(Udp.endPacket() != 1) {
        #if V_LEVEL <= WARNING
        Serial.printf("Failed to end packet GET_REFFERENCE_TABLE to: %s on port %d\n",
        Udp.remoteIP().toString().c_str(), Udp.remotePort());
        #endif
        return;
      }
    }
    break;

    case RESET: {
      #if V_LEVEL <= CRITICAL
      Serial.println("Got RESET packet type, restarting...");
      #endif
      ESP.restart();
    }
    break;

    ///TO DO: proper parsing and validation
    case ARTNET: {
      #if V_LEVEL <= DEBUG
      Serial.println("Got ARTNET");
      #endif

      byte upperHalf = incomingPacket[16];
      byte lowerHalf = incomingPacket[17];
      int len = (upperHalf << 8) + lowerHalf;
      byte* tmp = &(incomingPacket[18]);
      for (int i = 0; i < incomingPacket[1]; i++) {
        pixels.setPixelColor(i, tmp[i * 3], tmp[i * 3 + 1], tmp[i * 3 + 2]);
      }
      pixels.show();
    }
    break;

    default:{
      #if V_LEVEL <= WARNING
      Serial.println("Unimplemented packet type.");
      #endif
    }
    break;
  }
}

void loop() {
  long static lastPacketTime = 0;
  if ((connected) && (millis() - lastPacketTime > CONNECTION_TIMEOUT_TIME)) {
    #if V_LEVEL <= INFO
    Serial.println("Lost connection with server.");
    #endif
    connected = false;
    for (int i = 0; i < 100; i++)
    pixels.setPixelColor(i, 0);
    pixels.show();
  }

  static long blinkTime = 0;
  if ((!connected) && (millis() - blinkTime > 500)) {
    toogleLed();
    blinkTime = millis();
  }

  int packetSize = Udp.parsePacket();

  if (packetSize) {
    if (!connected) {
      #if V_LEVEL <= INFO
      Serial.println("Got connection with server.");
      #endif
      digitalWrite(ledPin, HIGH); //disable LED
      connected = true;
    }

    lastPacketTime = millis();
    int len = Udp.read(incomingPacket, 400);
    Udp.flush();

    if(len == 0) {
      #if V_LEVEL <= CRITICAL
      Serial.println("Failed to read data from packet! Reseting...");
      #endif
      ESP.restart();
    }

    handlePacket(incomingPacket, len);
  }
}
