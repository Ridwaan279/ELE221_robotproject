class EncoderMotor{
    private:
        int pin_encoderA;
        int pin_encoderB;
        int pin_I1;
        int pin_I2;
        int pin_PWM;

        const int ENC_K = 3840;

        int wheelAngle;
        long encCount;

    public:
        EncoderMotor(int encoderA, int encoderB, int I1, int I2, int PWM);
        void Move(int speed);
        static EncoderMotor* instance; // static pointer to the instance
        static void ISR_ChannelA() { instance->ChannelA(); }
        static void ISR_ChannelB() { instance->ChannelB(); }
        void ChannelA();
        void ChannelB();

};