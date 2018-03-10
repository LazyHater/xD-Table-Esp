#include "LedTableIrPanel.h"

LedTableIrPanel::LedTableIrPanel(byte latchPin, byte clockPin, byte dataPin, byte sensorsPin, byte threshold) :
  latchPin(latchPin), clockPin(clockPin), dataPin(dataPin), sensorsPin(sensorsPin), threshold(threshold) {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(sensorsPin, INPUT);

  createRefferenceTable();
};

void LedTableIrPanel::collectData() {
  for (uint8_t j = 0; j < 4; j++) {
    for (uint8_t i = 0; i < 25; i++) {
      set(i + (j << 5));
      if (i == 0 || i == 8 || i == 16) delayMicroseconds(200); ///need to check
      readings[iRReshuffle[i + 25 * j]] = analogRead(sensorsPin) >> 2;
    }
  }
  set(1 << 7); //turn off IR
}

uint8_t LedTableIrPanel::getThrreshold() {
  return threshold;
}

uint8_t LedTableIrPanel::setThrreshold(uint8_t threshold) {
  uint8_t tmp = this->threshold;
  this->threshold = threshold;
  return tmp;
}

uint8_t* LedTableIrPanel::getRaw() {
  return readings;
}

void LedTableIrPanel::getRefferenceTable(uint8_t* buff) {
  for (uint8_t i = 0; i < 100; i++) {
      buff[i] = refferenceTable[i];
  }
}

void LedTableIrPanel::createRefferenceTable() { // averages readings and stores it in array
  const uint8_t level = 2; //level = n means that there will be 2^n data readings

  memset(refferenceTable, 0, sizeof(refferenceTable));

  for (uint8_t j = 0; j < (1 << level); j++) { // sum iterations
    collectData();
    for (uint8_t i = 0; i < 100; i++)
      refferenceTable[i] += readings[i];
  }

  for (uint8_t i = 0; i < 100; i++) // divide all by number of additions
    refferenceTable[i] >>= level;
}

void LedTableIrPanel::convertReadingsToBoll() {
  for (uint8_t i = 0; i < 100; i++) {
     readings[i] = (abs(readings[i] - refferenceTable[i]) >= threshold) ? 255 : 0;
  }
}

const uint8_t LedTableIrPanel::iRReshuffle[100] = {
  0,  1,  2,  3,  4,
  10, 11, 12, 13, 14,
  20, 21, 22, 23, 24,
  30, 31, 32, 33, 34,
  40, 41, 42, 43, 44,

  5,  6,  7,  8,  9,
  15, 16, 17, 18, 19,
  25, 26, 27, 28, 29,
  35, 36, 37, 38, 39,
  45, 46, 47, 48, 49,

  50, 51, 52, 53, 54,
  60, 61, 62, 63, 64,
  70, 71, 72, 73, 74,
  80, 81, 82, 83, 84,
  90, 91, 92, 93, 94,

  55, 56, 57, 58, 59,
  65, 66, 67, 68, 69,
  75, 76, 77, 78, 79,
  85, 86, 87, 88, 89,
  95, 96, 97, 98, 99
};

void LedTableIrPanel::set(uint8_t p) {
  digitalWrite(latchPin, LOW);
  // shift out the bits:
  shiftOut(dataPin, clockPin, MSBFIRST, p);
  //take the latch pin high so the LEDs will light up:
  digitalWrite(latchPin, HIGH);
}
