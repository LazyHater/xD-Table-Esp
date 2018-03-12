#ifndef LED_TABLE_PIXELS_H
#define LED_TABLE_PIXELS_H

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

class LedTablePixels : public Adafruit_NeoPixel {
  public:
    LedTablePixels(uint16_t n, uint8_t p = 6, neoPixelType t = NEO_GRB + NEO_KHZ800);

    void enableGammaCorrection();

    void disableGammaCorrection();

    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);

      void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w);

    void setPixelColor(uint16_t n, uint32_t c);

  private:

    bool gammaCorrection = false;

    static const byte reshuffle_tab[];

    static const byte gamma[];
};

#endif // LED_TABLE_PIXELS_H
