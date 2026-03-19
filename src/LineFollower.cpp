#include <..\lib\LineFollower.h>
#include <Arduino.h>

LineFollower::LineFollower(int leftSensorPin, int centreSensorPin, int rightSensorPin){
    pin_sensor_l = leftSensorPin;
    pin_sensor_c = centreSensorPin;
    pin_sensor_r = rightSensorPin;
}

LineFollowerResult LineFollower::Read(){   
    int sensorLOut = analogRead(pin_sensor_l);
    int sensorCOut = analogRead(pin_sensor_c);
    int sensorROut = analogRead(pin_sensor_r);

    LineFollowerResult result = { sensorLOut, sensorCOut, sensorROut };

    return result;
}

LineFollowerResult LineFollower::AverageRead(){
    // Could call the Read() function, but trying to separate concerns.
    // Get an initial read 
    int sensorLOut = analogRead(pin_sensor_l);
    int sensorCOut = analogRead(pin_sensor_c);
    int sensorROut = analogRead(pin_sensor_r);

    // For the next 7 reads sum it up
    for ( int i = 0; i < 7; i++ ){
        sensorLOut += analogRead(pin_sensor_l);
        sensorCOut += analogRead(pin_sensor_c);
        sensorROut += analogRead(pin_sensor_r);
    }

    // Average by dividing by 8
    sensorLOut /= 8;
    sensorCOut /= 8;
    sensorROut /= 8;

    LineFollowerResult result = { sensorLOut, sensorCOut, sensorROut };

    // Return an average of the voltage read
    return result;
}

bool LineFollower::CheckIsOnColour(LineFollowerResult result, char desiredColour){ 
    bool isOnColour = false;
    switch ( desiredColour ){
        case 'w':
            if ( result.left < 100 || result.centre < 100 || result.right < 100 ){
                isOnColour = true;
            }
            break;
        case 'r':
            if ( ( result.left > 100 & result.left < 400 ) || ( result.centre > 100 & result.centre < 400 ) || ( result.right > 100 & result.right < 400 )  ){
                isOnColour = true;
            }
            break;
        case 'b':
            if ( result.left > 500 || result.centre > 500 || result.right > 500 ){
                isOnColour = true;
            }
            break;
    } 
    return isOnColour;
}