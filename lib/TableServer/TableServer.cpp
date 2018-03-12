#include "TableServer.h"

void TableServer::init() {
  pixels.begin();
  pixels.enableGammaCorrection();
  pixels.show(); // Initialize all pixels to 'off'

  if(Udp.begin(localUdpPort) == 1) {
    logger.info("Now listening at IP %s, UDP port %d\n",
                  WiFi.localIP().toString().c_str(), localUdpPort);
  }
  else {
    logger.error("Failed to open UDP port! Rebooting...\n");
    ESP.restart();
  }
}

void TableServer::handle() {
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

void TableServer::handlePacket(uint8_t packet[], int len){
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
