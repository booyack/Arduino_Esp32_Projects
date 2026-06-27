#include <Arduino.h>

#define RGB_LED_PIN 48


void setup() 
{

}

int r = 0;
int g = 0;
int b = 0;

void loop() 
{
  neopixelWrite(RGB_LED_PIN, r, g, b); // Czerwony
  delay(100);

  
  r = (r + 1) % 40;
  g = (g + 2) % 40;
  b = (b + 3) % 40;
}

