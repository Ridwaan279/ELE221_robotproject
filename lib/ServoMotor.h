#include <Servo.h>

class ServoMotor {
    private:
        int pin;
        int refPulseWidth; 
        Servo servo;
    
    public:
        ServoMotor(int PIN, int refPulse = 1500);  
        void SetAngle(int degrees);
        void Attach();    
        void Detach();
        int  SetAngleRad(float servoRadAngle); 
};