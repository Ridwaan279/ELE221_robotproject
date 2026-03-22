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
#define PIN_GRIPPER_SERVO 13

/* --------------------------------------------------------------------------------- */
/*                        IK Lookup Tables                                           */
/*                                                                                   */
/*  Arm:  L1=95mm, L2=250mm                                                         */
/*  X fixed at 265mm                                                                 */
/*  Y strokes from 0mm (bottom) to +100mm (top) in 51 steps                        */
/* --------------------------------------------------------------------------------- */

const int IK_STEPS = 20;
float test1 = 0.77;
float test2 = 0.63;

const float servo_base[IK_STEPS] = {
  1.28, 1.1758, 1.0811, 1.0116, 0.9547, 0.9084, 0.8668, 0.8268, 0.7921, 0.7553,
  0.7147, 0.6926, 0.6711, 0.6595, 0.6489, 0.6432, 0.6453, 0.6568, 0.7021, 0.76,
};
 
const float servo_joint[IK_STEPS] = {
  2.0, 1.8947, 1.7895, 1.6842, 1.5789, 1.4737, 1.3763, 1.2974, 1.2289, 1.1763,
  1.1237, 1.0711, 1.0184, 0.9658, 0.9132, 0.8605, 0.8079, 0.7553, 0.7026, 0.65,
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
  L_Motor.Move(0);
  R_Motor.Move(0);
  // stage=25;

}

/* --------------------------------------------------------------------------------- */
/*                                      Loop                                         */
/* --------------------------------------------------------------------------------- */

void loop() {
  unsigned long now = millis();

  if (now - lastDisplay >= delayDisplay) {
    lastDisplay = now;
    // Serial.print("Stage: "); Serial.print(stage);
    // Serial.print(" | IR R: "); Serial.print(R_IRSensor.AverageRead());
    // Serial.print(" | IR L: "); Serial.println(L_IRSensor.AverageRead());
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
      if (L_Motor.getAngle() >= 810) {
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
      if (L_Motor.getDistance() >= 0.165) {
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
      L_Motor.Move(-88);
      R_Motor.Move(100);
      if (L_IRSensor.AverageRead() >= 500 || R_IRSensor.AverageRead() >= 500) {
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
    
    case 9:{
      // black line follower
      LineFollowerResult lfr = LineFollower1.Read();

      if (lfr.left > 500 && lfr.centre > 350 && lfr.right > 500){
        L_Motor.Move(-70);
        R_Motor.Move(80);
      }
      else if (lfr.left > 500 && lfr.centre > 350 ){
        L_Motor.Move(-70);
        R_Motor.Move(80);
      } 
      else if (lfr.centre > 350 && lfr.right > 500){
        L_Motor.Move(-70);
        R_Motor.Move(80);
      }
      else if (lfr.left > 500){
        L_Motor.Move(40);
        R_Motor.Move(40);
      }
      else if (lfr.right > 500){
        L_Motor.Move(-40);
        R_Motor.Move(-40);
      }
      else{
        L_Motor.Move(40);
        R_Motor.Move(0);
      }

      if (L_IRSensor.AverageRead() >= 300 && R_IRSensor.AverageRead() >= 300) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 10;
      }
      break;
    }
    
    case 10:
      if (now - stageStart >= 1000) {
        L_Motor.resetDistance();
        R_Motor.resetDistance();
        interpStep = 0;
        BaseServo.Attach();
        JointServo.Attach();
        stageStart = now;
        stage = 11;
      }
      break;

    //---- Stage 10: Slowly interpolate from retracted pos to IK start (step 0) ----
    case 11:
      if (now - stageStart >= INTERP_DELAY_MS) {
        stageStart = now;
        float t = (float)interpStep / (float)(INTERP_STEPS - 1);
        float b = RETRACTED_BASE  + t * (servo_base[0]  - RETRACTED_BASE);
        float j = RETRACTED_JOINT + t * (servo_joint[0] - RETRACTED_JOINT);
        JointServo.SetAngleRad(j);
        BaseServo.SetAngleRad(b);
        interpStep++;
        if (interpStep >= INTERP_STEPS) {
          ikStep     = 0;
          stageStart = now;
          stage      = 12;
        }
      }
      break;

    // ---- Stage 11: Wait 1s at start position before drawing ----
    case 12:
      if (now - stageStart >= 1000) stage = 13;
      break;

    // ---- Stage 12: Step through vertical stroke ----
    case 13:
      if (ikStep < IK_STEPS) {
        BaseServo.SetAngleRad(servo_base[ikStep]);
        JointServo.SetAngleRad(servo_joint[ikStep]);

        Serial.print("Step "); Serial.print(ikStep);
        Serial.print(" | Base: ");  Serial.print(servo_base[ikStep]);
        Serial.print(" rad | Joint: "); Serial.print(servo_joint[ikStep]);
        Serial.println(" rad");

        ikStep++;
        stageStart = now;
        stage = 14;
      } else {
        stageStart = now;
        stage = 15;
      }
      break;

    // ---- Stage 13: Wait  between each IK step ----
    case 14:
      if (now - stageStart >= 500) stage = 13;
      break;

    // ---- Stage 14: Hold at end of stroke for 2s then start return ----
    case 15:
      if (now - stageStart >= 2000) {
        interpStep = 0;
        stageStart = now;
        stage = 16;
      }
      break;

    // ---- Stage 15: Slowly interpolate from IK end (step 0) back to retracted pos ----
    case 16:
        if (now - stageStart >= INTERP_DELAY_MS) {
            stageStart = now;
            float t = (float)interpStep / (float)(INTERP_STEPS - 1);
            float b = servo_base[IK_STEPS - 1]  + t * (RETRACTED_BASE  - servo_base[IK_STEPS - 1]);
            float j = servo_joint[IK_STEPS - 1] + t * (RETRACTED_JOINT - servo_joint[IK_STEPS - 1]);
            JointServo.SetAngleRad(j);
            BaseServo.SetAngleRad(b);
            interpStep++;
            if (interpStep >= INTERP_STEPS) {
                BaseServo.Detach();
                JointServo.Detach();
                stage = 18;
            }
        }
        break;

    case 18:
      L_Motor.Move(120);
      R_Motor.Move(120);
      if (L_Motor.getAngle() >= 200) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 19;
      }
      break;

    case 19:
      if (now - stageStart >= 1000) {
        L_Motor.resetDistance();
        R_Motor.resetDistance();
        stage = 20;
      }
      break;

    case 20:
      L_Motor.Move(-100);
      R_Motor.Move(115);
      if (L_IRSensor.AverageRead() >= 500 || R_IRSensor.AverageRead() >= 600) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 21;
      }
      break;

    case 21:{
      if (now - stageStart >= 1000) {
        L_Motor.resetDistance();
        R_Motor.resetDistance();
        stage = 22;
      }
      break;
    }

    case 22:
      L_Motor.Move(120);
      R_Motor.Move(120);
      if (L_Motor.getAngle() >= 200) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 23;
      }
      break;
    
    case 23:{
      if (now - stageStart >= 1000) {
        L_Motor.resetDistance();
        R_Motor.resetDistance();
        stage = 24;
      }
      break;
    }

    case 24:
      L_Motor.Move(-100);
      R_Motor.Move(110);
      if (L_Motor.getDistance() <= -0.5) {
        L_Motor.Move(0);
        R_Motor.Move(0);
        stageStart = now;
        stage = 25;
      }
      break;
    
    case 25:
      delay(1000);
      GripperServo.Attach();
      GripperServo.SetAngle(45);
      delay(500);
      break;
    

    // ---- Stage 9: Wait 1s then attach servos ----
    // case 9:
    //   if (now - stageStart >= 1000) {
    //     L_Motor.resetDistance();
    //     R_Motor.resetDistance();
    //     interpStep = 0;
    //     BaseServo.Attach();
    //     JointServo.Attach();
    //     stageStart = now;
    //     stage = 10;
    //   }
    //   break;

    // ---- Stage 10: Slowly interpolate from retracted pos to IK start (step 0) ----
    // case 10:
    //   if (now - stageStart >= INTERP_DELAY_MS) {
    //     stageStart = now;
    //     float t = (float)interpStep / (float)(INTERP_STEPS - 1);
    //     float b = RETRACTED_BASE  + t * (servo_base[0]  - RETRACTED_BASE);
    //     float j = RETRACTED_JOINT + t * (servo_joint[0] - RETRACTED_JOINT);
    //     JointServo.SetAngleRad(j);
    //     BaseServo.SetAngleRad(b);
    //     interpStep++;
    //     if (interpStep >= INTERP_STEPS) {
    //       ikStep     = 0;
    //       stageStart = now;
    //       stage      = 11;
    //     }
    //   }
    //   break;

    // // ---- Stage 11: Wait 1s at start position before drawing ----
    // case 11:
    //   if (now - stageStart >= 1000) stage = 12;
    //   break;

    // // ---- Stage 12: Step through vertical stroke ----
    // case 12:
    //   if (ikStep < IK_STEPS) {
    //     BaseServo.SetAngleRad(servo_base[ikStep]);
    //     JointServo.SetAngleRad(servo_joint[ikStep]);

    //     Serial.print("Step "); Serial.print(ikStep);
    //     Serial.print(" | Base: ");  Serial.print(servo_base[ikStep]);
    //     Serial.print(" rad | Joint: "); Serial.print(servo_joint[ikStep]);
    //     Serial.println(" rad");

    //     ikStep++;
    //     stageStart = now;
    //     stage = 13;
    //   } else {
    //     stageStart = now;
    //     stage = 14;
    //   }
    //   break;

    // // ---- Stage 13: Wait  between each IK step ----
    // case 13:
    //   if (now - stageStart >= 2000) stage = 12;
    //   break;

    // // ---- Stage 14: Hold at end of stroke for 2s then start return ----
    // case 14:
    //   if (now - stageStart >= 2000) {
    //     interpStep = 0;
    //     stageStart = now;
    //     stage = 15;
    //   }
    //   break;

    // // ---- Stage 15: Slowly interpolate from IK end (step 0) back to retracted pos ----
    // case 15:
    //   if (now - stageStart >= INTERP_DELAY_MS) {
    //     stageStart = now;
    //     float t = (float)interpStep / (float)(INTERP_STEPS - 1);
    //     float b = servo_base[0]  + t * (RETRACTED_BASE  - servo_base[0]);
    //     float j = servo_joint[0] + t * (RETRACTED_JOINT - servo_joint[0]);
    //     BaseServo.SetAngleRad(b);
    //     JointServo.SetAngleRad(j);
    //     interpStep++;
    //     if (interpStep >= INTERP_STEPS) {
    //       BaseServo.Detach();
    //       JointServo.Detach();
    //       stage = 16;
    //     }
    //   }
    //   break;

    // ---- Stage 16: Done — arm is back at retracted position ----
    case 26:
      // Finished. Change stage = 0 here if you want to repeat the full sequence.
      break;
  }
}