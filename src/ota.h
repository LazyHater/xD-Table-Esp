#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#include <ArduinoOTA.h>
#include "Logger.h"
#include "../lib/config/config.h"

void setupOTA() {
  ArduinoOTA.setHostname(hostname);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    logger.info("Start updating %s\n", type.c_str());
  });
  ArduinoOTA.onEnd([]() {
    logger.info("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    logger.info("Progress: %u%%\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    logger.error("[%u]: ", error);
    if (error == OTA_AUTH_ERROR) logger.printf("Auth Failed\n");
    else if (error == OTA_BEGIN_ERROR) logger.printf("Begin Failed\n");
    else if (error == OTA_CONNECT_ERROR) logger.printf("Connect Failed\n");
    else if (error == OTA_RECEIVE_ERROR) logger.printf("Receive Failed\n");
    else if (error == OTA_END_ERROR) logger.printf("End Failed\n");
  });
  ArduinoOTA.begin();
}

#endif // OTA_H
