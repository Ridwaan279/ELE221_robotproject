class LED {
    private:
        int pin;
        unsigned long lastFlash;   // tracks last time LED toggled
        unsigned long interval;    // how often to toggle (ms)
        bool state;                // current LED on/off state
 
    public:
        LED(int PIN, unsigned long flashInterval = 500);
        void FlashLED();   // call repeatedly in loop()
        void On();
        void Off();
};