#include "servo.h"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

Servo::Servo(int pin, int minPulse, int maxPulse)
    : pin(pin), minPulse(minPulse), maxPulse(maxPulse), currentPosition(minPulse), running(false)
{
    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialization failed\n");
        exit(1);
    }

    gpioSetMode(pin, PI_OUTPUT);
    gpioServo(pin, currentPosition);
}

Servo::~Servo()
{
    running = false;
    if (controlThread.joinable()) {
        controlThread.join();
    }
}


void Servo::start()
{
    if (running) return; // Avoid starting multiple threads

    running = true;
    controlThread = std::thread(&Servo::updateServo, this);
}

void Servo::stop()
{
    running = false;
    if (controlThread.joinable()) {
        controlThread.join();
    }

    gpioServo(pin, 0); // Turn off servo
}

void Servo::setPosition(int pulseWidth)
{
    if (pulseWidth >= minPulse && pulseWidth <= maxPulse) {
        currentPosition = pulseWidth;
    }
}

void Servo::updateServo()
{
    while (running) {
        gpioServo(pin, currentPosition);
        gpioDelay(10000); // 10ms
    }
}
