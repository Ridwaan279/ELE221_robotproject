class IRSensor{
    private: 
        int pin;

    public:
        IRSensor(int PIN);
        int AverageRead();
        int Read();

};