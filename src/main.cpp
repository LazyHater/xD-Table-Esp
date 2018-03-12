/*
Created on: 20.04.2017
Author: Wakabajaszi
Soft ver. 2.0
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include "LedTablePixels.h"
#include "LedTableIrPanel.h"
#include "Logger.h"
#include "config.h"
#include "ota.h"
#include "TableServer.h"

TableServer tableServer;
ESP8266WebServer server(80);

void toogleLed() {
  static bool state = false;
  state = !state;
  digitalWrite(ledPin, (state) ? HIGH : LOW);
}

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


  tableServer.init();
}


void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  tableServer.handle();
}
