/* --------------------------------------------------------------------------------- */
/*                                      Libraries                                    */
/* --------------------------------------------------------------------------------- */


#include <Arduino.h>
#include <math.h>
#include "..\lib\LED.h"
#include "..\lib\IRSensor.h"
#include "..\lib\EncoderMotor.h"
#include "..\lib\LineFollower.h"
#include "..\lib\ServoMotor.h"

/* --------------------------------------------------------------------------------- */
/*                                      Globals                                      */
/* --------------------------------------------------------------------------------- */


/* Constants */

//    ANALOG PINS
//  IR Sensors
#define PIN_IR_LEFT A6
#define PIN_IR_RIGHT A7
// Line Follower 
#define PIN_LF_L A0
#define PIN_LF_C A1
#define PIN_LF_R A2
//  Colour sensor
#define PIN_CS_0 15
#define PIN_CS_1 16
#define PIN_CS_2 17
#define PIN_CS_3 18

//    DIGITAL PINS
//  LEDs 
#define PIN_LED 49
// Left motor — encoder on interrupt pins 2 & 3
#define PIN_ENC_L_A    2
#define PIN_ENC_L_B    3
#define PIN_ENC_L_I1   4
#define PIN_ENC_L_I2   8
#define PIN_ENC_L_PWM  9
 
// Right motor — encoder on interrupt pins 19 & 18
#define PIN_ENC_R_A    19
#define PIN_ENC_R_B    18
#define PIN_ENC_R_I1   6
#define PIN_ENC_R_I2   5
#define PIN_ENC_R_PWM  7

// Servo motors
#define PIN_BASE_SERVO 10
#define PIN_JOINT_SERVO 11
#define PIN_GRIPPER_SERVO 12



// Components

  // LED
  LED led1(PIN_LED, 1000);
  // IR sensors
  IRSensor L_IRSensor(PIN_IR_LEFT);
  IRSensor R_IRSensor(PIN_IR_RIGHT);
  // Encoder motors
  EncoderMotor L_Motor(PIN_ENC_L_A, PIN_ENC_L_B, PIN_ENC_L_I1, PIN_ENC_L_I2, PIN_ENC_L_PWM, true);
  EncoderMotor R_Motor(PIN_ENC_R_A, PIN_ENC_R_B, PIN_ENC_R_I1, PIN_ENC_R_I2, PIN_ENC_R_PWM, false);
  // Line follower
  LineFollower LineFollower1(PIN_LF_L, PIN_LF_C, PIN_LF_R);
  // Servo motor
  ServoMotor BaseServo(PIN_BASE_SERVO, 800);
  ServoMotor JointServo(PIN_JOINT_SERVO, 600);
  ServoMotor GripperServo(PIN_GRIPPER_SERVO, 1000);

unsigned long lastDisplay = 0;
const unsigned long delayDisplay = 250;
 
int stage = 0;
unsigned long stageStart = 0;

/* --------------------------------------------------------------------------------- */
/*                                      Setup and Loop                               */
/* --------------------------------------------------------------------------------- */



/* Setup function (on first boot) */
void setup() {

  Serial.begin(9600);
  L_Motor.resetDistance();
  R_Motor.resetDistance();
}


/* Loop function (continuous operation) */
int angle = 0;
void loop() {
  BaseServo.Detach();
  JointServo.Detach();
  GripperServo.Detach();

  //Serial.println(L_Motor.getAngle());

  /*
  angle += 1;
  if ( angle < 90 ){
    BaseServo.SetAngle(angle);
  }
  else if ( angle > 90 ) { 
    BaseServo.SetAngle(180 - angle);
  }
  else if ( angle > 180 ){
    angle = 0;
  }
  delay(200);*/
  
  //JointServo.SetAngle(0);
  //GripperServo.SetAngle(0);



  
  // Get a read of the line follower
  LineFollowerResult lfr = LineFollower1.Read();
  
  /*Serial.println(String(lfr.left) + ": " + LineFollower1.CheckResultColour(lfr.left) + "\t" + 
   String(lfr.centre) + ": " + LineFollower1.CheckResultColour(lfr.centre) + "\t" +
   String(lfr.right) + ": " + LineFollower1.CheckResultColour(lfr.right) + "\t");*/

  if ( L_IRSensor.Read() > 300 || R_IRSensor.Read() > 300 ){
    L_Motor.Move(-100);
    L_Motor.Move(-100);
  }
  else{
    // Follow the desired colour (line following)
    char colourToFollow = 'b';
    if ( LineFollower1.CheckResultColour(lfr.left) != colourToFollow & LineFollower1.CheckResultColour(lfr.centre) != colourToFollow )
    {
      L_Motor.Move(100);
      R_Motor.Move(50);
    }
    else if ( LineFollower1.CheckResultColour(lfr.centre) != colourToFollow && LineFollower1.CheckResultColour(lfr.right) != colourToFollow ){
      L_Motor.Move(50);
      R_Motor.Move(100);
    }
    else{
      L_Motor.Move(125);
      R_Motor.Move(125);
    }
  }

  delay(300);


  
  //LineFollowerResult result = LineFollower1.Read();
  //int result_ls = L_IRSensor.AverageRead();
  //int result_rs = R_IRSensor.AverageRead();
  //Serial.println(String(result_ls) + "\t" + String(result_rs) );
  //Serial.println(String(result.left) + String(result.centre) + String(result.right) + "\t IR Sensor: L:" + String(result_ls) + " | R:" + String(result_rs));
  /*unsigned long now = millis();
  // Print distance every 250ms
  if (now - lastDisplay >= delayDisplay) {
    lastDisplay = now;
    Serial.print("IR sensor Right: ");
    Serial.print(R_IRSensor.AverageRead());
    Serial.print(" IR sensor Left: ");
    Serial.println(L_IRSensor.AverageRead());
  }
  led1.FlashLED();

  switch (stage) {
    case 0:
      // Turn 360
      L_Motor.Move(120);
      R_Motor.Move(120);
      if (L_Motor.getAngle() >= 800) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 1;
      }
      break;
 
    case 1:
      // Wait 1 second then reset and go again
      if (now - stageStart >= 1000) {
        L_Motor.resetDistance();
        R_Motor.resetDistance();
        stage = 2;
      }
      break;

    case 2:
      // move backwards for a bit
      L_Motor.Move(100);
      R_Motor.Move(-115);
      if (L_Motor.getDistance() >= 0.14) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 3;
      }
      break;

    case 3:
      // Wait 1 second then reset and go again
      if (now - stageStart >= 1000) {
        L_Motor.resetDistance();
        R_Motor.resetDistance();
        stage = 4;
      }
      break;

    case 4:
      // move forward until IR sensors detect something, then stop
      L_Motor.Move(-90);
      R_Motor.Move(100);
      if (L_IRSensor.AverageRead() >= 500 || R_IRSensor.AverageRead() >= 400) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 5;
      }
      break;

    case 5:
      // Wait 1 second then reset and go again
      if (now - stageStart >= 1000) {
        L_Motor.resetDistance();
        R_Motor.resetDistance();
        stage = 6;
      }
      break;

    case 6:
      // Turn 90
      L_Motor.Move(120);
      R_Motor.Move(120);
      if (L_Motor.getAngle() >= 250) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 7;
      }
      break;

    case 7:
      // Wait 1 second then reset and go again
      if (now - stageStart >= 1000) {
        L_Motor.resetDistance();
        R_Motor.resetDistance();
        // stage = 6;
      }
      break;
  }*/
  


}
