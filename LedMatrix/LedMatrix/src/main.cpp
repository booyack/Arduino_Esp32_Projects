#include <Arduino.h>
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;
uint32_t frame[3] = {0, 0, 0};
int currentLed = 0;
const int potPin = A0;
const int minDelayMs = 20;
const int maxDelayMs = 800;

void setup() {
  matrix.begin();
  matrix.clear();
}

void loop() {
  frame[0] = 0;
  frame[1] = 0;
  frame[2] = 0;

  int wordIndex = currentLed / 32;
  int bitIndex = 31 - (currentLed % 32);
  frame[wordIndex] = 1u << bitIndex;

  matrix.loadFrame(frame);

  int potValue = analogRead(potPin);
  int delayMs = map(potValue, 0, 4095, minDelayMs, maxDelayMs);
  delayMs = constrain(delayMs, minDelayMs, maxDelayMs);

  currentLed = (currentLed + 1) % (8 * 12);
  delay(delayMs);
}

