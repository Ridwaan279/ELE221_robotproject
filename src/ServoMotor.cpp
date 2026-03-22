#include "..\lib\ServoMotor.h"
#include <Arduino.h>

ServoMotor::ServoMotor(int PIN, int refPulse) {
    pin           = PIN;
    refPulseWidth = refPulse;  
    servo.attach(pin);
    pinMode(pin, OUTPUT);
}

void ServoMotor::SetAngle(int angle){
    servo.write(angle);
}

void ServoMotor::Attach() {
    servo.attach(pin);
}

void ServoMotor::Detach() {
    servo.detach();
}

int ServoMotor::SetAngleRad(float servoRadAngle) {
    const float Pi      = 3.142;
    float minPulseWidth = float(refPulseWidth) - 1000.0;
    int cmdSignal       = (servoRadAngle + (Pi / 2)) * (2000.0 / Pi) + minPulseWidth;
    servo.writeMicroseconds(cmdSignal);
    return cmdSignal;
}