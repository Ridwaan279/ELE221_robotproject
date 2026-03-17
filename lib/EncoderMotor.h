class EncoderMotor{
    private:
        int pin_encoderA;
        int pin_encoderB;
        int pin_I1;
        int pin_I2;
        int pin_PWM;

        const int ENC_K = 3840;

        int wheelAngle;
        int encCount;

    public:
        EncoderMotor(int encoderA, int encoderB, int I1, int I2, int PWM);
        void Move(int speedL, int speedR);
        void ChannelB();
        void ChannelA();

};