/* --------------------------------------------------------------------------------- */
/*                                      Libraries                                    */
/* --------------------------------------------------------------------------------- */

/*
  No such files? 
#include <Adafruit_MPU6050.h> ? 
#include <Adafruit_Sensor.h>
#include <Wire.h>*/
#include <Arduino.h>
#include "..\lib\LED.h"
#include "..\lib\IRSensor.h"
#include "..\lib\EncoderMotor.h"
#include "..\lib\LineFollower.h"

/* --------------------------------------------------------------------------------- */
/*                                      Globals                                      */
/* --------------------------------------------------------------------------------- */


/* Constants */

//    ANALOG PINS
//  IR Sensors
#define PIN_IR_LEFT 6
#define PIN_IR_RIGHT 7
// Line Follower 
#define PIN_LF_L 0
#define PIN_LF_C 1
#define PIN_LF_R 2
//  Colour sensor
#define PIN_CS_0 15
#define PIN_CS_1 16
#define PIN_CS_2 17
#define PIN_CS_3 18

//    DIGITAL PINS
//  LEDs 
#define PIN_LED 49
// Encoder Motor
#define PIN_ENC_L_A 2
#define PIN_ENC_L_B 3 
#define PIN_ENC_L_AI1 4
#define PIN_ENC_L_AI2 8
#define PIN_ENC_L_PWMA 9 
 
#define PIN_ENC_R_A 19
#define PIN_ENC_R_B 18
#define PIN_ENC_R_BI1  6
#define PIN_ENC_R_BI2 5
#define PIN_ENC_R_PWMB 7

// Components

  // LED
  LED led1(PIN_LED);
  // IR sensors
  IRSensor L_IRSensor(PIN_IR_LEFT);
  IRSensor R_IRSensor(PIN_IR_RIGHT);
  // Encoder motors
  EncoderMotor L_EncoderMotor(PIN_ENC_L_A, PIN_ENC_L_B, PIN_ENC_L_AI1, PIN_ENC_L_AI2, PIN_ENC_L_PWMA, true);
  EncoderMotor R_EncoderMotor(PIN_ENC_R_A, PIN_ENC_R_B, PIN_ENC_R_BI1, PIN_ENC_R_BI2, PIN_ENC_R_PWMB, false);
  // Line follower
  LineFollower LineFollower1(PIN_LF_L, PIN_LF_C, PIN_LF_R);

/* --------------------------------------------------------------------------------- */
/*                          Function Prototypes                                      */
/* --------------------------------------------------------------------------------- */

void TCS();
void ISR_INTO();
void timer2_init(void);
void DetectColour();
void SetupDOFSensor();
void ReadDOFSensor();


/* --------------------------------------------------------------------------------- */
/*                                      Setup and Loop                               */
/* --------------------------------------------------------------------------------- */



/* Setup function (on first boot) */
void setup() {
 // HELLO WORLD
  /* Initialise the components */
  //  Colour sensor
  /*pinMode(PIN_CS_0, OUTPUT);
  pinMode(PIN_CS_1, OUTPUT);
  pinMode(PIN_CS_2, OUTPUT);
  pinMode(PIN_CS_3, OUTPUT);*/
  Serial.begin(9600);

}

/* Loop function (continuous operation) */
void loop() {
  // These two go in opposite directions but they're both supposed to be going forward?
  //L_EncoderMotor.Move(100);
  //R_EncoderMotor.Move(100);

  LineFollowerResult result = LineFollower1.Read();
  int result_ls = L_IRSensor.Read();
  int result_rs = R_IRSensor.Read();
  Serial.println(String(result.left) + String(result.centre) + String(result.right) + "\t IR Sensor: L:" + String(result_ls) + " | R:" + String(result_rs));
}




/* --------------------------------------------------------------------------------- */
/*                                      Functions                                    */
/* --------------------------------------------------------------------------------- */



