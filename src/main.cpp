/* --------------------------------------------------------------------------------- */
/*                                      Libraries                                    */
/* --------------------------------------------------------------------------------- */
#include <Arduino.h>
#include "..\lib\LED.h"
#include "..\lib\IRSensor.h"
#include "..\lib\EncoderMotor.h"
 
/* --------------------------------------------------------------------------------- */
/*                                      Pin Definitions                              */
/* --------------------------------------------------------------------------------- */
 
#define PIN_IR_LEFT  0
#define PIN_IR_RIGHT 1
#define PIN_LED      49
 
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
 
/* --------------------------------------------------------------------------------- */
/*                                      Components                                   */
/* --------------------------------------------------------------------------------- */
 
LED      led1(PIN_LED);
IRSensor L_IRSensor(PIN_IR_LEFT);
IRSensor R_IRSensor(PIN_IR_RIGHT);
 
EncoderMotor L_Motor(PIN_ENC_L_A, PIN_ENC_L_B, PIN_ENC_L_I1, PIN_ENC_L_I2, PIN_ENC_L_PWM, true);
EncoderMotor R_Motor(PIN_ENC_R_A, PIN_ENC_R_B, PIN_ENC_R_I1, PIN_ENC_R_I2, PIN_ENC_R_PWM, false);
 
/* --------------------------------------------------------------------------------- */
/*                                      Globals                                      */
/* --------------------------------------------------------------------------------- */
 
unsigned long lastDisplay = 0;
const unsigned long delayDisplay = 250;
 
int stage = 0;
unsigned long stageStart = 0;
 
/* --------------------------------------------------------------------------------- */
/*                                      Setup                                        */
/* --------------------------------------------------------------------------------- */
 
void setup() {
  Serial.begin(9600);
  L_Motor.resetDistance();
  R_Motor.resetDistance();
}
 
/* --------------------------------------------------------------------------------- */
/*                                      Loop                                         */
/* --------------------------------------------------------------------------------- */
 
void loop() {
  unsigned long now = millis();
 
  // Print distance every 250ms
  if (now - lastDisplay >= delayDisplay) {
    lastDisplay = now;
    Serial.print("L Distance (m): ");
    Serial.print(L_Motor.getDistance());
    Serial.print("   R Distance (m): ");
    Serial.println(R_Motor.getDistance());
  }
 
  switch (stage) {
    case 0:
      // Drive forward until 0.1m travelled
      L_Motor.Move(150);
      R_Motor.Move(-150);
      if (L_Motor.getDistance() >= 0.1) {
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
        stage = 0;
      }
      break;
  }
}