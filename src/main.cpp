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

/* --------------------------------------------------------------------------------- */
/*                                      Globals                                      */
/* --------------------------------------------------------------------------------- */


// Colour sensor
int flag=0;
int counter=0;
int countR=0,countG=0,countB=0;
// DOF Sensor
//Adafruit_MPU6050 mpu;

/* Constants */

//    ANALOG PINS
//  IR Sensors
#define PIN_IR_LEFT 0
#define PIN_IR_RIGHT 1

//    DIGITAL PINS
//  LEDs 
#define PIN_LED 49
//  Colour sensor
#define PIN_CS_0 15
#define PIN_CS_1 16
#define PIN_CS_2 17
#define PIN_CS_3 18
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

  LED led1(PIN_LED);
  // IR sensor
  IRSensor L_IRSensor(PIN_IR_LEFT);
  IRSensor R_IRSensor(PIN_IR_RIGHT);
  // Encoder motor
  EncoderMotor L_EncoderMotor(PIN_ENC_L_A, PIN_ENC_L_B, PIN_ENC_L_AI1, PIN_ENC_L_AI2, PIN_ENC_L_PWMA);
  EncoderMotor R_EncoderMotor(PIN_ENC_R_A, PIN_ENC_R_B, PIN_ENC_R_BI1, PIN_ENC_R_BI2, PIN_ENC_R_PWMB);


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
  //Serial.begin(9600);

}

/* Loop function (continuous operation) */
void loop() {
  // These two go in opposite directions but they're both supposed to be going forward?
  L_EncoderMotor.Move(100);
  R_EncoderMotor.Move(100);
}




/* --------------------------------------------------------------------------------- */
/*                                      Functions                                    */
/* --------------------------------------------------------------------------------- */




void TCS()
{
  digitalWrite(PIN_CS_1,HIGH);
  digitalWrite(PIN_CS_0,LOW);
  flag=0;
  attachInterrupt(0, ISR_INTO, CHANGE);
  timer2_init();
}

void ISR_INTO()
{
  counter++;
}

void timer2_init(void)
{
  TCCR2A=0x00;
  TCCR2B=0x07; //the clock frequency source 1024 points
  TCNT2= 100;    //10 ms overflow again
  TIMSK2 = 0x01; //allow interrupt
}


ISR(TIMER2_OVF_vect)//the timer 2, 10ms interrupt overflow again. Internal overflow interrupt executive function
{
TCNT2=100;
flag++;
if(flag==1)
 {
   counter=0;
 }
else if(flag==2)
  {
   digitalWrite(PIN_CS_2,LOW);
   digitalWrite(PIN_CS_3,LOW); 
   countR=counter/1.051;
   Serial.print("red=");
   Serial.println(countR,DEC);
   digitalWrite(PIN_CS_2,HIGH);
   digitalWrite(PIN_CS_3,HIGH);   
  }
else if(flag==3)
   {
    countG=counter/1.0157;
   Serial.print("green=");
   Serial.println(countG,DEC);
    digitalWrite(PIN_CS_2,LOW);
    digitalWrite(PIN_CS_3,HIGH); 
    }
else if(flag==4)
   {
    countB=counter/1.114;
   Serial.print("blue=");
   Serial.println(countB,DEC);
    digitalWrite(PIN_CS_2,LOW);
    digitalWrite(PIN_CS_3,LOW);
    }
else
    {
    flag=0; 
     TIMSK2 = 0x00;
    }
    counter=0;
    delay(2);
}

void DetectColour(){ //Rid test code for colour sensor, placed in loop
  delay(10);
  TCS();
  if((countR>10)||(countG>10)||(countB>10))
    {
      if((countR>countG)&&(countR>countB))
        {
            Serial.print("red");
            Serial.print("\n");
            delay(1000);
        }
      else if((countG>=countR)&&(countG>countB))
        {
            Serial.print("green");
            Serial.print("\n");
            delay(1000);
        } 
      else if((countB>countG)&&(countB>countR))
      {
            Serial.print("blue");
            Serial.print("\n");
            delay(1000);
      }
    }
  else 
  {
      delay(1000);       
  }

}

/*void SetupDOFSensor(){
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);  
}

void ReadDOFSensor(){
 /* Get new sensor events with the readings */
  /*sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);*/

  /* Print out the values */
  /*Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");

  Serial.println("");
  delay(500);  
}*/