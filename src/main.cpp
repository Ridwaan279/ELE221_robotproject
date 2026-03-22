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
#define PIN_BASE_SERVO    10
#define PIN_JOINT_SERVO   11
#define PIN_GRIPPER_SERVO 12

/* --------------------------------------------------------------------------------- */
/*                        IK Lookup Tables                                           */
/*                                                                                   */
/*  Arm:  L1=95mm, L2=250mm                                                         */
/*  X fixed at 265mm                                                                 */
/*  Y strokes from 0mm (bottom) to +100mm (top) in 51 steps                        */
/* --------------------------------------------------------------------------------- */

const int IK_STEPS = 51;

 
const float servo_base[IK_STEPS] = {
  0.6897, 0.7170, 0.7439, 0.7703, 0.7963, 0.8218, 0.8469, 0.8716, 0.8959, 0.9196,
  0.9429, 0.9658, 0.9882, 1.0100, 1.0314, 1.0523, 1.0727, 1.0926, 1.1119, 1.1307,
  1.1489, 1.1666, 1.1837, 1.2003, 1.2163, 1.2317, 1.2465, 1.2607, 1.2742, 1.2872,
  1.2996, 1.3113, 1.3224, 1.3329, 1.3427, 1.3519, 1.3605, 1.3684, 1.3757, 1.3823,
  1.3883, 1.3936, 1.3982, 1.4022, 1.4055, 1.4082, 1.4102, 1.4115, 1.4122, 1.4121,
  1.4114,
};
 
const float servo_joint[IK_STEPS] = {
  1.3866, 1.4034, 1.4194, 1.4347, 1.4493, 1.4632, 1.4764, 1.4889, 1.5008, 1.5119,
  1.5224, 1.5321, 1.5412, 1.5497, 1.5574, 1.5645, 1.5709, 1.5766, 1.5817, 1.5860,
  1.5897, 1.5928, 1.5951, 1.5968, 1.5978, 1.5982, 1.5978, 1.5968, 1.5951, 1.5928,
  1.5897, 1.5860, 1.5817, 1.5766, 1.5709, 1.5645, 1.5574, 1.5497, 1.5412, 1.5321,
  1.5224, 1.5119, 1.5008, 1.4889, 1.4764, 1.4632, 1.4493, 1.4347, 1.4194, 1.4034,
  1.3866,
};

// Retracted position — physical resting pose of the arm
#define RETRACTED_BASE  0.0f
#define RETRACTED_JOINT 0.0f

// Slow interpolation config — used when moving to/from retracted position
#define INTERP_STEPS     40     // number of steps for the slow move
#define INTERP_DELAY_MS  50     // ms between each step (~2 seconds total)

/* --------------------------------------------------------------------------------- */
/*                                      Components                                   */
/* --------------------------------------------------------------------------------- */

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
  // Servo motors
  ServoMotor BaseServo(PIN_BASE_SERVO, 800);
  ServoMotor JointServo(PIN_JOINT_SERVO, 600);
  ServoMotor GripperServo(PIN_GRIPPER_SERVO, 1000);

/* --------------------------------------------------------------------------------- */
/*                                      State                                        */
/* --------------------------------------------------------------------------------- */

unsigned long lastDisplay = 0;
const unsigned long delayDisplay = 250;

int           stage      = 0;
unsigned long stageStart = 0;
int           ikStep     = 0;
int           interpStep = 0;   // used for slow retract/deploy moves

/* --------------------------------------------------------------------------------- */
/*                                      Setup                                        */
/* --------------------------------------------------------------------------------- */

void setup() {
  Serial.begin(9600);
  L_Motor.resetDistance();
  R_Motor.resetDistance();

  // Close gripper and hold
  GripperServo.Attach();
  GripperServo.SetAngle(0);
  delay(500);
  GripperServo.Detach();

}

/* --------------------------------------------------------------------------------- */
/*                                      Loop                                         */
/* --------------------------------------------------------------------------------- */

void loop() {
  unsigned long now = millis();

  if (now - lastDisplay >= delayDisplay) {
    lastDisplay = now;
    Serial.print("Stage: "); Serial.print(stage);
    Serial.print(" | IR R: "); Serial.print(R_IRSensor.AverageRead());
    Serial.print(" | IR L: "); Serial.println(L_IRSensor.AverageRead());
  }

  led1.FlashLED();

  switch (stage) {

    // ---- Stage 0: Attach servos at retracted position, init interpolation ----
    case 0:
      interpStep = 0;
      JointServo.Attach();
      BaseServo.Attach();
      JointServo.SetAngleRad(RETRACTED_JOINT);
      BaseServo.SetAngleRad(RETRACTED_BASE);
      delay(1000);
      stageStart = now;
      JointServo.Detach();
      BaseServo.Detach();
      stage = 2;
      break;

    // ---- Stage 2: Turn 360 degrees ----
    case 2:
      L_Motor.Move(120);
      R_Motor.Move(120);
      if (L_Motor.getAngle() >= 840) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 3;
      }
      break;

    // ---- Stage 3: Wait 1s then reset encoders ----
    case 3:
      if (now - stageStart >= 1000) {
        L_Motor.resetDistance();
        R_Motor.resetDistance();
        stage = 4;
      }
      break;

    // ---- Stage 4: Move backwards ----
    case 4:
      L_Motor.Move(100);
      R_Motor.Move(-110);
      if (L_Motor.getDistance() >= 0.15) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 5;
      }
      break;

    // ---- Stage 5: Wait 1s then reset encoders ----
    case 5:
      if (now - stageStart >= 1000) {
        L_Motor.resetDistance();
        R_Motor.resetDistance();
        stage = 6;
      }
      break;

    // ---- Stage 6: Move forward until IR sensors detect wall ----
    case 6:
      L_Motor.Move(-90);
      R_Motor.Move(100);
      if (L_IRSensor.AverageRead() >= 500 || R_IRSensor.AverageRead() >= 400) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 7;
      }
      break;

    // ---- Stage 7: Wait 1s then reset encoders ----
    case 7:
      if (now - stageStart >= 1000) {
        L_Motor.resetDistance();
        R_Motor.resetDistance();
        stage = 8;
      }
      break;

    // ---- Stage 8: Turn 90 degrees ----
    case 8:
      L_Motor.Move(120);
      R_Motor.Move(120);
      if (L_Motor.getAngle() >= 200) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 9;
      }
      break;

    // ---- Stage 9: Wait 1s then attach servos ----
    case 9:
      if (now - stageStart >= 1000) {
        L_Motor.resetDistance();
        R_Motor.resetDistance();
        interpStep = 0;
        BaseServo.Attach();
        JointServo.Attach();
        stageStart = now;
        stage = 10;
      }
      break;

    // ---- Stage 10: Slowly interpolate from retracted pos to IK start (step 0) ----
    case 10:
      if (now - stageStart >= INTERP_DELAY_MS) {
        stageStart = now;
        float t = (float)interpStep / (float)(INTERP_STEPS - 1);
        float b = RETRACTED_BASE  + t * (servo_base[0]  - RETRACTED_BASE);
        float j = RETRACTED_JOINT + t * (servo_joint[0] - RETRACTED_JOINT);
        BaseServo.SetAngleRad(b);
        JointServo.SetAngleRad(j);
        interpStep++;
        if (interpStep >= INTERP_STEPS) {
          ikStep     = 0;
          stageStart = now;
          stage      = 11;
        }
      }
      break;

    // ---- Stage 11: Wait 1s at start position before drawing ----
    case 11:
      if (now - stageStart >= 1000) stage = 12;
      break;

    // ---- Stage 12: Step through vertical stroke ----
    case 12:
      if (ikStep < IK_STEPS) {
        BaseServo.SetAngleRad(servo_base[ikStep]);
        JointServo.SetAngleRad(servo_joint[ikStep]);

        Serial.print("Step "); Serial.print(ikStep);
        Serial.print(" | Base: ");  Serial.print(servo_base[ikStep]);
        Serial.print(" rad | Joint: "); Serial.print(servo_joint[ikStep]);
        Serial.println(" rad");

        ikStep++;
        stageStart = now;
        stage = 13;
      } else {
        stageStart = now;
        stage = 14;
      }
      break;

    // ---- Stage 13: Wait 300ms between each IK step ----
    case 13:
      if (now - stageStart >= 300) stage = 12;
      break;

    // ---- Stage 14: Hold at end of stroke for 2s then start return ----
    case 14:
      if (now - stageStart >= 2000) {
        interpStep = 0;
        stageStart = now;
        stage = 15;
      }
      break;

    // ---- Stage 15: Slowly interpolate from IK end (step 0) back to retracted pos ----
    case 15:
      if (now - stageStart >= INTERP_DELAY_MS) {
        stageStart = now;
        float t = (float)interpStep / (float)(INTERP_STEPS - 1);
        float b = servo_base[0]  + t * (RETRACTED_BASE  - servo_base[0]);
        float j = servo_joint[0] + t * (RETRACTED_JOINT - servo_joint[0]);
        BaseServo.SetAngleRad(b);
        JointServo.SetAngleRad(j);
        interpStep++;
        if (interpStep >= INTERP_STEPS) {
          BaseServo.Detach();
          JointServo.Detach();
          stage = 16;
        }
      }
      break;

    // ---- Stage 16: Done — arm is back at retracted position ----
    case 16:
      // Finished. Change stage = 0 here if you want to repeat the full sequence.
      break;
  }
}