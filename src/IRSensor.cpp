#include <Arduino.h>
#include "..\lib\IRSensor.h"

IRSensor::IRSensor(int PIN){
    pin = PIN;
}

int IRSensor::AverageRead(){
  int out = 0;
  out     = analogRead(pin);
  out     += analogRead(pin);
  out     += analogRead(pin);
  out     += analogRead(pin);
  out     += analogRead(pin);
  out     += analogRead(pin);
  out     += analogRead(pin);
  out     += analogRead(pin);

  out     = out / 8;
  return    out;
}

int IRSensor::Read(){
    return analogRead(pin);
}