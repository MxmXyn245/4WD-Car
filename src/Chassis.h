#ifndef CHASSIS_H
#define CHASSIS_H

#include <Arduino.h>

class Chassis
{
private:
    // Конфігурація пінів
    const int pinLB_Bwd = 4;
    const int pinLB_Fwd = 16;
    const int pinRB_Bwd = 17;
    const int pinRB_Fwd = 18;
    const int pinLF_Bwd = 26;
    const int pinLF_Fwd = 25;
    const int pinRF_Bwd = 33;
    const int pinRF_Fwd = 32;

    // Стан швидкостей
    int currentLeftSpeed = 0;
    int currentRightSpeed = 0;
    int targetLeftSpeed = 0;
    int targetRightSpeed = 0;

    // Параметри рампи
    const int STEP_LINEAR = 8;
    const int STEP_TURN = 2;
    bool isTurning = false;

    void setMotorRaw(int left, int right);

public:
    Chassis();
    void init();
    void setTargets(int x, int y);
    void updateSmooth();
    void stopEmergency();
};

#endif