struct LineFollowerResult{
    int left;
    int centre;
    int right;
};

class LineFollower{
    private:
        int pin_sensor_l;
        int pin_sensor_c;
        int pin_sensor_r;
    
    public:
        LineFollower(int leftSensorPin, int centreSensorPin, int rightSensorPin);
        LineFollowerResult Read();
};