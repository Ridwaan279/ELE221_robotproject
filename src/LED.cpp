#include "../lib/LED.h"
#include <Arduino.h>

LED::LED(int PIN)
{
    pin = PIN;
    pinMode(pin, OUTPUT);
}

void LED::FlashLED(){
  digitalWrite(pin, HIGH);
  delay(500);
  digitalWrite(pin, LOW);
  delay(500);
}