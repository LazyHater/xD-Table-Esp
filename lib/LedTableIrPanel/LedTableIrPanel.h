#ifndef LED_TABLE_IR_PANNEL_H
#define LED_TABLE_IR_PANNEL_H

#include <Arduino.h>

class LedTableIrPanel {
  public:
    LedTableIrPanel(byte latchPin, byte clockPin, byte dataPin, byte sensorsPin, byte threshold);

    void collectData();

    uint8_t getThrreshold();

    uint8_t setThrreshold(uint8_t threshold);

    uint8_t* getRaw();

    void getRefferenceTable(uint8_t* buff);

    void createRefferenceTable();

    void convertReadingsToBoll();

  private:
    uint8_t latchPin;
    uint8_t clockPin;
    uint8_t dataPin;
    uint8_t sensorsPin;
    uint8_t threshold;

    uint8_t  readings[100] = {0};
    uint16_t  refferenceTable[100] = {0};

    static const uint8_t iRReshuffle[];

    void set(uint8_t p);
};

#endif // LED_TABLE_IR_PANNEL_H
