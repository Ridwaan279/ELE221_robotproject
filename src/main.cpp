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
/*                                      Pin Definitions                              */
/* --------------------------------------------------------------------------------- */
 
#define PIN_IR_LEFT       A6
#define PIN_IR_RIGHT      A7
#define PIN_LF_L          A0
#define PIN_LF_C          A1
#define PIN_LF_R          A2
 
#define PIN_CS_0          15
#define PIN_CS_1          16
#define PIN_CS_2          17
#define PIN_CS_3          18
 
#define PIN_LED           49
 
#define PIN_ENC_L_A       2
#define PIN_ENC_L_B       3
#define PIN_ENC_L_I1      4
#define PIN_ENC_L_I2      8
#define PIN_ENC_L_PWM     9
 
#define PIN_ENC_R_A       19
#define PIN_ENC_R_B       18
#define PIN_ENC_R_I1      6
#define PIN_ENC_R_I2      5
#define PIN_ENC_R_PWM     7
 
#define PIN_BASE_SERVO    10
#define PIN_JOINT_SERVO   11
#define PIN_GRIPPER_SERVO 12
 
/*                        IK Lookup Tables                                           */
/*                                                                                   */
/*  Arm:  L1=95mm, L2=250mm                                                         */
/*  X fixed at 265mm — optimal reach for L1+L2=345mm                               */
/*  Y strokes from +70mm (top) to -30mm (bottom) in 51 steps                       */
/*  Joint stays ~90° throughout — arm points straight down                          */
/* --------------------------------------------------------------------------------- */

const int IK_STEPS = 51;

const float servo_base[IK_STEPS] = {
  1.2317, 1.2391, 1.2465, 1.2536, 1.2607, 1.2675, 1.2742, 1.2808, 1.2872, 1.2935,
  1.2996, 1.3055, 1.3113, 1.3170, 1.3224, 1.3277, 1.3329, 1.3379, 1.3427, 1.3474,
  1.3519, 1.3563, 1.3605, 1.3645, 1.3684, 1.3721, 1.3757, 1.3791, 1.3823, 1.3854,
  1.3883, 1.3910, 1.3936, 1.3960, 1.3982, 1.4003, 1.4022, 1.4040, 1.4055, 1.4070,
  1.4082, 1.4093, 1.4102, 1.4109, 1.4115, 1.4119, 1.4122, 1.4122, 1.4121, 1.4119,
  1.4114,
};

const float servo_joint[IK_STEPS] = {
  1.5982, 1.5981, 1.5978, 1.5974, 1.5968, 1.5961, 1.5951, 1.5940, 1.5928, 1.5913,
  1.5897, 1.5880, 1.5860, 1.5839, 1.5817, 1.5792, 1.5766, 1.5738, 1.5709, 1.5678,
  1.5645, 1.5610, 1.5574, 1.5536, 1.5497, 1.5455, 1.5412, 1.5368, 1.5321, 1.5273,
  1.5224, 1.5172, 1.5119, 1.5064, 1.5008, 1.4949, 1.4889, 1.4828, 1.4764, 1.4699,
  1.4632, 1.4564, 1.4493, 1.4421, 1.4347, 1.4271, 1.4194, 1.4115, 1.4034, 1.3951,
  1.3866,
};
 
// Retracted position — matches image 2 (arm folded back diagonally)
#define RETRACTED_BASE  -1.0   // adjust if needed
#define RETRACTED_JOINT  0.8   // adjust if needed
 
/* --------------------------------------------------------------------------------- */
/*                                      Components                                   */
/* --------------------------------------------------------------------------------- */
 
LED          led1(PIN_LED, 1000);
IRSensor     L_IRSensor(PIN_IR_LEFT);
IRSensor     R_IRSensor(PIN_IR_RIGHT);
EncoderMotor L_Motor(PIN_ENC_L_A, PIN_ENC_L_B, PIN_ENC_L_I1, PIN_ENC_L_I2, PIN_ENC_L_PWM, true);
EncoderMotor R_Motor(PIN_ENC_R_A, PIN_ENC_R_B, PIN_ENC_R_I1, PIN_ENC_R_I2, PIN_ENC_R_PWM, false);
LineFollower LineFollower1(PIN_LF_L, PIN_LF_C, PIN_LF_R);
 
ServoMotor   BaseServo(PIN_BASE_SERVO,       800);
ServoMotor   JointServo(PIN_JOINT_SERVO,     600);
ServoMotor   GripperServo(PIN_GRIPPER_SERVO, 1000);
 
/* --------------------------------------------------------------------------------- */
/*                                      Globals                                      */
/* --------------------------------------------------------------------------------- */
 
unsigned long lastDisplay = 0;
const unsigned long delayDisplay = 250;
 
int           stage      = 0;
unsigned long stageStart = 0;
int           ikStep     = 0;
 
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
}
 
/* --------------------------------------------------------------------------------- */
/*                                      Loop                                         */
/* --------------------------------------------------------------------------------- */
 
void loop() {
  unsigned long now = millis();
 
  // Attach servos and hold current IK position to prevent snap
  BaseServo.Attach();
  JointServo.Attach();
  BaseServo.SetAngleRad(servo_base[ikStep]);
  JointServo.SetAngleRad(servo_joint[ikStep]);
 
  if (now - lastDisplay >= delayDisplay) {
    lastDisplay = now;
    Serial.print("Stage: "); Serial.print(stage);
    Serial.print("  IK Step: "); Serial.println(ikStep);
  }
 
  led1.FlashLED();
 
  switch (stage) {
 
    case 0:
      // Go to retracted position and wait 3 seconds
      BaseServo.SetAngleRad(RETRACTED_BASE);
      JointServo.SetAngleRad(RETRACTED_JOINT);
      stageStart = now;
      stage = 1;
      break;
 
    case 1:
      // Hold retracted for 3 seconds
      if (now - stageStart >= 3000) {
        ikStep     = 0;
        stageStart = now;
        stage      = 2;
      }
      break;
 
    case 2:
      // Move to start of IK stroke (top)
      BaseServo.SetAngleRad(servo_base[0]);
      JointServo.SetAngleRad(servo_joint[0]);
      stageStart = now;
      stage = 3;
      break;
 
    case 3:
      // Wait 1 second at top before drawing
      if (now - stageStart >= 1000) stage = 4;
      break;
 
    case 4:
      // Step through vertical stroke
      if (ikStep < IK_STEPS) {
        BaseServo.SetAngleRad(servo_base[ikStep]);
        JointServo.SetAngleRad(servo_joint[ikStep]);
 
        Serial.print("Step "); Serial.print(ikStep);
        Serial.print(" | Base: ");  Serial.print(servo_base[ikStep]);
        Serial.print(" rad | Joint: "); Serial.print(servo_joint[ikStep]);
        Serial.println(" rad");
 
        ikStep++;
        stageStart = now;
        stage = 5;
      } else {
        stageStart = now;
        stage = 6;
      }
      break;
 
    case 5:
      // Wait between steps
      if (now - stageStart >= 300) stage = 4;
      break;
 
    case 6:
      // Hold at bottom 2 seconds then retract and repeat
      if (now - stageStart >= 2000) {
        ikStep = 0;
        stage  = 0;
      }
      break;
  }
}