#ifndef SERVO_H
#define SERVO_H

#include <pigpio.h>
#include <thread>
#include <atomic>

class Servo {
public:
    Servo(int pin, int minPulse, int maxPulse);
    ~Servo();

    void start();                     // Start the servo control thread
    void stop();                      // Stop the control thread
    void setPosition(int pulseWidth); // Set servo position in µs

private:
    int pin;                          // GPIO pin for the servo
    int minPulse;
    int maxPulse;

    std::atomic<int> currentPosition;
    std::thread controlThread;
    std::atomic<bool> running;        // For thread loop control

    void updateServo();               // Internal thread loop
};

#endif // SERVO_H
