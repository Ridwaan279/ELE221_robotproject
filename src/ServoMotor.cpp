#include "..\lib\ServoMotor.h";
#include <Arduino.h>

ServoMotor::ServoMotor(int PIN){
    pin = PIN;
    servo.attach(pin);
    pinMode(pin, OUTPUT);
}

void ServoMotor::SetAngle(int angle){
    servo.write(angle);
}