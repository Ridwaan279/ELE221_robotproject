#include "../lib/LED.h"
#include <Arduino.h>
 
LED::LED(int PIN, unsigned long flashInterval)
{
    pin       = PIN;
    interval  = flashInterval;
    lastFlash = 0;
    state     = false;
    pinMode(pin, OUTPUT);
}
 
// Call this repeatedly in loop() — it toggles every `interval` ms without blocking
void LED::FlashLED() {
    unsigned long now = millis();
    if (now - lastFlash >= interval) {
        lastFlash = now;
        state = !state;
        digitalWrite(pin, state ? HIGH : LOW);
    }
}
 
void LED::On() {
    state = true;
    digitalWrite(pin, HIGH);
}
 
void LED::Off() {
    state = false;
    digitalWrite(pin, LOW);
}
