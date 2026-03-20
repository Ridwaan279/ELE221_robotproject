#ifndef ENCODERMOTOR_H
#define ENCODERMOTOR_H
 
#include <Arduino.h>
 
class EncoderMotor {
  private:
    int pin_encoderA;
    int pin_encoderB;
    int pin_I1;
    int pin_I2;
    int pin_PWM;

    bool reverseDirection = false; // The motors are wired in not a perfect mirror so requires a code compensation
 
    const int   ENC_K   = 3840;   // Encoder edges per revolution
    const float WHEEL_D = 0.06;   // Wheel diameter in metres
 
    float iPrev;                   // PI controller: saved integral term
 
    // One static pointer per motor so ISRs know which instance to call
    static EncoderMotor* instanceL;
    static EncoderMotor* instanceR;
  
    // ISR Values
    volatile long  encCount;
    volatile float distance_m;

    // Static ISR trampolines
    static void ISR_ChannelA_L() { instanceL->channelA(); }
    static void ISR_ChannelB_L() { instanceL->channelB(); }
    static void ISR_ChannelA_R() { instanceR->channelA(); }
    static void ISR_ChannelB_R() { instanceR->channelB(); }
    // Internal ISR logic
    void channelA();
    void channelB();
 
  public:
    // isLeft=true → left motor, isLeft=false → right motor
    EncoderMotor(int encoderA, int encoderB, int I1, int I2, int PWM, bool isLeft);
 
    void  Move(int speed);         // +speed=forward, -speed=reverse, 0=brake
    void  resetDistance();         // Zero the encoder count and distance
    float getDistance();           // Distance travelled in metres
    float getAngle();              // Wheel rotation angle in degrees
    int   piController(float Kp, float Ki, float ref, float feedback, float deltaT);
};
 
#endif