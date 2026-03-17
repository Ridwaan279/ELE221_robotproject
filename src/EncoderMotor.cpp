#include "..\lib\EncoderMotor.h"
#include <Arduino.h>

EncoderMotor::EncoderMotor(int encoderA, int encoderB, int I1, int I2, int PWM){
    pin_encoderA = encoderA;
    pin_encoderB = encoderB;
    pin_I1 = I1;
    pin_I2 = I2;
    pin_PWM = PWM;

    pinMode(pin_encoderA, INPUT);
    pinMode(pin_encoderB, INPUT);
    pinMode(pin_I1, OUTPUT);
    pinMode(pin_I2, OUTPUT);
    pinMode(pin_PWM, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(pin_encoderA),ChannelA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pin_encoderB),ChannelB, CHANGE);
}

void EncoderMotor::ChannelA() {
// Use if else statements to increment or decrement the encCount variable
// Use information in the table in the Encoder section of the The Rotary 
// Position Encoder section of the laboratory Documentation.
// Focus on the table rows where Output A is changing
// Use digitalread() to acquire the states of PINA and PINB

  int stateA = digitalRead(pin_encoderA); // Read the state of channel A
  int stateB = digitalRead(pin_encoderB); // Read the state of channel B

  // Check whether channel A has gone HIGH (rising edge)
  if (stateA == 1) {
    // When Output A changes from 0→1, use the state of Output B to determine direction
    if (stateB == 0) {
      encCount++;  // Encoder rotating in one direction, e.g. clockwise
    } else {
      // Encoder rotating in the opposite direction, e.g. counter clockwise
      encCount--;
    }
  }

  // Otherwise, channel A has gone LOW (falling edge)
  else {
    // When Output A changes from 1→0, again use Output B to determine direction
    if (stateB == 1) {
      encCount++;  // Encoder rotating in one direction, e.g. clockwise
    } else {
      // Encoder rotating in the opposite direction, e.g. counter clockwise
      encCount--;
    }
  }

  //compute the angle of rotation of the wheel using the pulse count, encCount
  wheelAngle = ((float)encCount / ENC_K)*360.0; 
}
void EncoderMotor::ChannelA() {
  // Use function channelA() as a template
  // This time focus on the rows of the table where Output B is changing
  int stateA = digitalRead(pin_encoderA); // Read the state of channel A
  int stateB = digitalRead(pin_encoderB); // Read the state of channel B

   // Check whether channel B has gone HIGH (rising edge)
  if (stateB == 1) {
    // When Output B changes from 0→1, use the state of Output A to determine direction
    if (stateA == 1) {
      encCount++;  // Encoder rotating in one direction, e.g. clockwise
      //Serial.println("ADDB");
    } else {
      // Encoder rotating in the opposite direction, e.g. counter clockwise
      encCount--;
      //Serial.println("SUBB");
    }
  }

  // Otherwise, channel B has gone LOW (falling edge)
  else {
    // When Output B changes from 1→0, again use Output A to determine direction
    if (stateA == 0) {
      encCount++;  // Encoder rotating in one direction, e.g. clockwise
    } else {
      // Encoder rotating in the opposite direction, e.g. counter clockwise
      encCount--;
    }
  }


  //compute the angle of rotation of the wheel using the pulse count, encCount
  wheelAngle = ((float)encCount / ENC_K)*360.0; 
}