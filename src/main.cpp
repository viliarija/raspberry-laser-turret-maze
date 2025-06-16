#include "game.h"
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <chrono>
#include <thread>
#include <cmath>
#include <thread>
#include <atomic>
#include <functional>
#include <unistd.h>
#include "servo.h"
#include "../include/MCP3008/include/MCP3008.h"

using namespace MCP3008Lib;

std::atomic<bool> running(true);

enum Command {
    NONE,
    PAUSE,
    DEBUG,
    RESET,
    CALIB,
    QUIT
};
std::atomic<Command> currentCommand(Command::NONE);

const int TARGET_FPS = 60;
const double TARGET_FRAME_TIME = 1000.0 / TARGET_FPS;

float PLANE_WIDTH_CM;
float PLANE_HEIGHT_CM;
float DISTANCE_CM;

float PX_PER_CM_X;
float PX_PER_CM_Y;
float CENTER_X_PX;
float CENTER_Y_PX;

int offsetX;
int offsetY;

std::map<std::string, std::string> readSettingsCSV(const std::string& filename) {
    std::map<std::string, std::string> settings;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open settings file.\n";
        return settings;
    }

    std::string key, value, line;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        size_t comma = line.find(',');
        if (comma != std::string::npos) {
            key = line.substr(0, comma);
            value = line.substr(comma + 1);
            settings[key] = value;
        }
    }

    return settings;
}

inline float degrees(float radians) {
    return radians * 180.0f / M_PI;
}

void calculateRayAngles(float x_px, float y_px, float& angleYaw_deg, float& anglePitch_deg) {
    // Pixel offset from center
    float dx_px = x_px - CENTER_X_PX;
    float dy_px = y_px - CENTER_Y_PX;

    // Convert to cm (real-world)
    float dx_cm = dx_px / PX_PER_CM_X;
    float dy_cm = -dy_px / PX_PER_CM_Y;  // Invert Y axis
    float dz_cm = DISTANCE_CM;

    // Construct 3D direction vector from origin to target
    // Forward direction is (0, 0, 1), so:
    // Yaw (θ) is angle between Z axis and X-Z projection
    // Pitch (ϕ) is angle between Z axis and Y-Z projection
    angleYaw_deg   = degrees(atan2(dx_cm, dz_cm));
    anglePitch_deg = degrees(atan2(dy_cm, std::sqrt(dx_cm * dx_cm + dz_cm * dz_cm)));
}

void readJoystick(MCP3008& adc, int set[2]) {
    set[0] = adc.read(0) + offsetY;  // Joystick Y
    set[1] = adc.read(1) + offsetX;  // Joystick X
}

float mapValue(float val, float in_min, float in_max, float out_min, float out_max) {
    return out_min + (val - in_min) * (out_max - out_min) / (in_max - in_min);
}

void mainLoop(Game& game, MCP3008& adc, Servo& servoX, Servo& servoY, float speed, float maxAngle, float minPulse, float maxPulse) {
    const auto targetFrameTime = std::chrono::duration<double, std::milli>(TARGET_FRAME_TIME);
    int joyVals[2];
    float playerCoords[2];

    bool debug = false;
    bool paused = false;

    while (running.load()) {
        auto frameStartTime = std::chrono::high_resolution_clock::now();

        Command cmd = currentCommand.exchange(Command::NONE);  // get & reset
        switch (cmd) {
            case PAUSE:
                paused ^= 1;
                break;
            case DEBUG:
                debug = true;
                break;
            case RESET:
                game.reset();
                break;
            case CALIB:
                game.calib();
                break;
            case QUIT:
                running = false;
                break;
            default:
                break;
        }


        if (!paused) {        
            readJoystick(adc, joyVals);

            float dx = mapValue((float)joyVals[1], 0, 1023.0f, -speed, speed);
            float dy = mapValue((float)joyVals[0], 0, 1023.0f, speed, -speed);

            game.movePlayer(dx, dy);
            game.getCoords(playerCoords);

            float angleX, angleY;
            calculateRayAngles(playerCoords[0], playerCoords[1], angleX, angleY);

            int pulseX = mapValue(angleX, (float)(0 - (maxAngle / 2)), (float)(maxAngle / 2), minPulse, maxPulse);
            int pulseY = mapValue(angleY, (float)(0 - (maxAngle / 2)), (float)(maxAngle / 2), minPulse, maxPulse);

            servoX.setPosition(pulseX);
            servoY.setPosition(pulseY);

            if (debug) {
                printf("Joystick X: %d, Joystick Y: %d  |  Angle X: %.1f°, Angle Y: %.1f°  |  Servo X: %d, Servo Y: %d\n", 
                        joyVals[1], joyVals[0], angleX, angleY, pulseX, pulseY);
                printf("Player X: %.1f, Player Y: %.1f\n", playerCoords[0], playerCoords[1]);
            }
        }


        auto frameEndTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> frameDuration = frameEndTime - frameStartTime;

        if (frameDuration < targetFrameTime) {
            std::this_thread::sleep_for(targetFrameTime - frameDuration);
        }
    }
}

int main() {
    auto settings = readSettingsCSV("settings.csv");

    int mazeWidth = std::stoi(settings["MAZE_WIDTH"]);
    int mazeHeight = std::stoi(settings["MAZE_HEIGHT"]);
    int cellSize = std::stoi(settings["CELL_SIZE"]);
    float speed = std::stof(settings["PLAYER_SPEED"]);

    PLANE_WIDTH_CM = std::stoi(settings["PLANE_WIDTH_CM"]);
    PLANE_HEIGHT_CM = std::stoi(settings["PLANE_HEIGHT_CM"]);
    DISTANCE_CM = std::stoi(settings["DISTANCE_CM"]);

    float minPulse = std::stoi(settings["MIN_PULSE"]);
    float maxPulse = std::stoi(settings["MAX_PULSE"]);
    float maxAngle = std::stoi(settings["MAX_ANGLE"]);

    offsetX = std::stoi(settings["OFFSET_X"]);
    offsetY = std::stoi(settings["OFFSET_Y"]);

    PX_PER_CM_X = (float)(mazeWidth * cellSize) / PLANE_WIDTH_CM;
    PX_PER_CM_Y = (float)(mazeHeight * cellSize) / PLANE_HEIGHT_CM;
    CENTER_X_PX = (float)(mazeWidth * cellSize) / 2.0f;
    CENTER_Y_PX = (float)(mazeHeight * cellSize) / 2.0f;

    Game game(mazeWidth, mazeHeight, cellSize);
    game.initialize();

    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialization failed\n");
        return 1;
    }

    Servo servoX(18, minPulse, maxPulse);
    Servo servoY(17, minPulse, maxPulse);
    servoX.start();
    servoY.start();

    MCP3008 adc;
    adc.connect();

    // Start the main game loop in a separate thread
    std::thread gameThread(mainLoop, std::ref(game), std::ref(adc), std::ref(servoX), std::ref(servoY),
                           speed, maxAngle, minPulse, maxPulse);

    // Command loop (can be extended)
    std::string input;
    while (running) {
        std::cin >> input;
        if (input == "pause") {
            currentCommand = PAUSE;
        } else if (input == "debug") {
            currentCommand = DEBUG;
        } else if (input == "calib") {
            currentCommand = CALIB;
        } else if (input == "reset") {
            currentCommand = RESET;
        } else if (input == "quit") {
            running = false;
        }
    }

    // Cleanup
    if (gameThread.joinable())
        gameThread.join();

    servoX.stop();
    servoY.stop();
    gpioTerminate();

    std::cout << "Program terminated safely." << std::endl;
    return 0;
}