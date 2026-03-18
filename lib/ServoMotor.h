#include <Servo.h>

class ServoMotor{
    private:
        int pin;
        Servo servo;
    
    public:
        ServoMotor(int PIN);
        void SetAngle(int degrees);
};