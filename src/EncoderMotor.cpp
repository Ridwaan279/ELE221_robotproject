#include "..\lib\EncoderMotor.h"
#include <Arduino.h>

/* --------------------------------------------------------------------------------- */
/*                          Static instance initialisation                           */
/* --------------------------------------------------------------------------------- */

EncoderMotor* EncoderMotor::instanceL = nullptr;
EncoderMotor* EncoderMotor::instanceR = nullptr;

/* --------------------------------------------------------------------------------- */
/*                                   Constructor                                     */
/* --------------------------------------------------------------------------------- */

EncoderMotor::EncoderMotor(int encoderA, int encoderB, int I1, int I2, int PWM, bool isLeft)
{
  pin_encoderA = encoderA;
  pin_encoderB = encoderB;
  pin_I1       = I1;
  pin_I2       = I2;
  pin_PWM      = PWM;


  encCount   = 0;
  distance_m = 0.0;
  iPrev      = 0.0;

  pinMode(pin_encoderA, INPUT);
  pinMode(pin_encoderB, INPUT);
  pinMode(pin_I1,  OUTPUT);
  pinMode(pin_I2,  OUTPUT);
  pinMode(pin_PWM, OUTPUT);

  if (isLeft) {
    instanceL = this;
    attachInterrupt(digitalPinToInterrupt(pin_encoderA), ISR_ChannelA_L, CHANGE);
    attachInterrupt(digitalPinToInterrupt(pin_encoderB), ISR_ChannelB_L, CHANGE);
  } else {
    instanceR = this;
    attachInterrupt(digitalPinToInterrupt(pin_encoderA), ISR_ChannelA_R, CHANGE);
    attachInterrupt(digitalPinToInterrupt(pin_encoderB), ISR_ChannelB_R, CHANGE);
  }
}

/* --------------------------------------------------------------------------------- */
/*                                 ISR Handlers                                      */
/* --------------------------------------------------------------------------------- */

// Note: local variables named enc_A / enc_B to avoid clashing with AVR register PINB
void EncoderMotor::channelA() {
  int enc_A = digitalRead(pin_encoderA);
  int enc_B = digitalRead(pin_encoderB);

  if (enc_A == 1) {
    if (enc_B == 0) encCount++;
    else            encCount--;
  } else {
    if (enc_B == 1) encCount++;
    else            encCount--;
  }
  distance_m = ((float)encCount / ENC_K) * 3.1416 * WHEEL_D;
}

void EncoderMotor::channelB() {
  int enc_A = digitalRead(pin_encoderA);
  int enc_B = digitalRead(pin_encoderB);

  if (enc_B == 1) {
    if (enc_A == 1) encCount++;
    else            encCount--;
  } else {
    if (enc_A == 0) encCount++;
    else            encCount--;
  }
  distance_m = ((float)encCount / ENC_K) * 3.1416 * WHEEL_D;
}

/* --------------------------------------------------------------------------------- */
/*                                 Public Methods                                    */
/* --------------------------------------------------------------------------------- */

void EncoderMotor::Move(int speed) {
  if (speed < 0) {
    digitalWrite(pin_I1, LOW);
    digitalWrite(pin_I2, HIGH);

    speed = -speed;
  } else if (speed > 0) {
    digitalWrite(pin_I1, HIGH);
    digitalWrite(pin_I2, LOW);
  } else {
    digitalWrite(pin_I1, HIGH);
    digitalWrite(pin_I2, HIGH);  // Brake
  }
  analogWrite(pin_PWM, speed);
}

void EncoderMotor::resetDistance() {
  encCount   = 0;
  distance_m = 0.0;
}

float EncoderMotor::getDistance() {
  return distance_m;
}

float EncoderMotor::getAngle() {
  return ((float)encCount / ENC_K) * 360.0;
}

int EncoderMotor::piController(float Kp, float Ki, float ref, float feedback, float deltaT) {
  float error = ref - feedback;
  float p = error * Kp;
  float i = iPrev + (error * Ki * deltaT);

  if      (i >  255) i =  255;
  else if (i < -255) i = -255;

  float m = p + i;

  if      (m >  255) m =  255;
  else if (m < -255) m = -255;

  iPrev = i;
  return (int)m;
}