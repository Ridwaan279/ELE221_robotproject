#include <..\lib\LineFollower.h>
#include <Arduino.h>

LineFollower::LineFollower(int leftSensorPin, int centreSensorPin, int rightSensorPin){
    pin_sensor_l = leftSensorPin;
    pin_sensor_c = centreSensorPin;
    pin_sensor_r = rightSensorPin;

    //pinMode(pin_sensor_l, INPUT);
    pinMode(pin_sensor_c, INPUT);
    pinMode(pin_sensor_r, INPUT);
}

LineFollowerResult LineFollower::Read(){   
    int sensorLOut = analogRead(pin_sensor_l); //digitalRead(pin_sensor_l);
    int sensorCOut = digitalRead(pin_sensor_c);
    int sensorROut = digitalRead(pin_sensor_r);

    LineFollowerResult result = { sensorLOut, sensorCOut, sensorROut };

    return result;
}