/*
Created on: 20.04.2017
Author: Wakabajaszi
Soft ver. 2.0
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include "LedTablePixels.h"
#include "LedTableIrPanel.h"
#include "Logger.h"
#include "config.h"
#include "ota.h"

LedTablePixels pixels = LedTablePixels(pixelsNumb, pixelsPin, NEO_GRB + NEO_KHZ800);
LedTableIrPanel ledTableIrPanel = LedTableIrPanel(latchPin, clockPin, dataPin, sensorsPin, 4);
WiFiUDP Udp;
Logger logger;

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

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "lolollo!");
  digitalWrite(led, 0);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}
void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(pixelsPin, OUTPUT);

  pixels.begin();
  pixels.enableGammaCorrection();
  pixels.show(); // Initialize all pixels to 'off'


  Serial.begin(115200);
  Serial.setDebugOutput(true);
  logger.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    toogleLed();
    delay(250);
    toogleLed();
    logger.printf(".");
    delay(250);
  }

  logger.printf(" connected\n");


  setupOTA();

  MDNS.addService("table", "udp", 6454);
  MDNS.addService("http", "tcp", 80);

  server.on("/", handleRoot);

server.on("/inline", [](){
  server.send(200, "text/plain", "this works as well");
});

server.onNotFound(handleNotFound);

server.begin();
Serial.println("HTTP server started");

  if(Udp.begin(localUdpPort) == 1) {
    logger.info("Now listening at IP %s, UDP port %d\n",
                  WiFi.localIP().toString().c_str(), localUdpPort);
  }
  else {
    logger.error("Failed to open UDP port! Rebooting...\n");
    ESP.restart();
  }
}

void handlePacket(uint8_t packet[], int len){
  logger.debug("Got packet (0x%x) ", incomingPacket[0]);
  switch (incomingPacket[0]) {
    case CALIBRATE: {
      logger.info("Got CALIBRATE packet, calibrating...");
      ledTableIrPanel.createRefferenceTable();
    }
    break;

    case PING: {
       logger.info("Got PING, sending back PONG.\n");

      if(Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()) != 1) {
        logger.error("Failed to begin packet PONG to: %s on port %d\n",
        Udp.remoteIP().toString().c_str(), Udp.remotePort());
      }

      Udp.write(PONG);

      if(Udp.endPacket() != 1) {
        logger.error("Failed to end packet PONG to: %s on port %d\n",
        Udp.remoteIP().toString().c_str(), Udp.remotePort());
      }
    }
    break;

    case SET_MATRIX: {
      logger.debug("Got SET_MATRIX of length %i and %i pixels\n", len, incomingPacket[1]);
      byte* tmp = &(incomingPacket[2]);

      for (int i = 0; i < incomingPacket[1]; i++)
      pixels.setPixelColor(i, tmp[i * 3], tmp[i * 3 + 1], tmp[i * 3 + 2]);

      pixels.show();
    }
    break;

    case GET_TOUCHSCREEN: {
      logger.debug("Got GET_TOUCHSCREEN collecting data...");

      ledTableIrPanel.collectData();
      ledTableIrPanel.convertReadingsToBoll();

      if(Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()) != 1) {
          logger.error("Failed to begin packet GET_TOUCHSCREEN to %s on port %d\n",
          Udp.remoteIP().toString().c_str(), Udp.remotePort());
      }

      Udp.write((byte)TOUCHSCREEN);
      Udp.write(ledTableIrPanel.getRaw(), 100);

      if(Udp.endPacket() != 1) {
        logger.error("Failed to end packet GET_TOUCHSCREEN to: %s on port %d\n",
                      Udp.remoteIP().toString().c_str(), Udp.remotePort());
      }
    }
    break;

    case GET_TOUCHSCREEN_RAW: {
      logger.debug("Got GET_TOUCHSCREEN_RAW collecting data...\n");
      ledTableIrPanel.collectData();

      if(Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()) != 1) {
        logger.error("Failed to begin packet GET_TOUCHSCREEN_RAW to: %s on port %d\n",
        Udp.remoteIP().toString().c_str(), Udp.remotePort());
        return;
      }

      Udp.write((byte)TOUCHSCREEN_RAW);
      Udp.write(ledTableIrPanel.getRaw(), 100);

      if(Udp.endPacket() != 1) {
        logger.error("Failed to end packet GET_TOUCHSCREEN_RAW to: %s on port %d\n",
        Udp.remoteIP().toString().c_str(), Udp.remotePort());
        return;
      }
    }
    break;

    case GET_REFFERENCE_TABLE: {
      logger.debug("Got GET_REFFERENCE_TABLE");

      uint8_t buff[100];
      ledTableIrPanel.getRefferenceTable(buff);

      if(Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()) != 1) {
        logger.error("Failed to begin packet GET_REFFERENCE_TABLE to: %s on port %d\n",
        Udp.remoteIP().toString().c_str(), Udp.remotePort());
        return;
      }

      Udp.write((byte)REFFERENCE_TABLE);
      Udp.write(buff, 100);

      if(Udp.endPacket() != 1) {
        logger.error("Failed to end packet GET_REFFERENCE_TABLE to: %s on port %d\n",
        Udp.remoteIP().toString().c_str(), Udp.remotePort());
        return;
      }
    }
    break;

    case RESET: {
      logger.info("Got RESET packet type, restarting...\n");
      ESP.restart();
    }
    break;

    ///TO DO: proper parsing and validation
    case ARTNET: {
      logger.debug("Got ARTNET packet\n");

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
      logger.error("Unimplemented packet type.\n");
    }
    break;
  }
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  long static lastPacketTime = 0;
  if ((connected) && (millis() - lastPacketTime > CONNECTION_TIMEOUT_TIME)) {
    logger.info("Lost connection with server.\n");
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
      logger.info("Got connection with server.\n");
      digitalWrite(ledPin, HIGH); //disable LED
      connected = true;
    }

    lastPacketTime = millis();
    int len = Udp.read(incomingPacket, 400);
    Udp.flush();

    if(len == 0) {
      logger.error("Failed to read data from packet! Reseting...\n");
      ESP.restart();
    }

    handlePacket(incomingPacket, len);
  }
}
