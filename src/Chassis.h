#ifndef CHASSIS_H
#define CHASSIS_H

#include <Arduino.h>

enum class SteeringMode
{
    DIFFERENTIAL, // Танковий поворот (на місці)
    CURVE         // Плавна дуга (без реверсу внутрішнього борту)
};

struct MotorPins
{
    uint8_t bwd;
    uint8_t fwd;
};

struct ChassisConfig
{
    MotorPins leftBack{4, 16};
    MotorPins rightBack{17, 18};
    MotorPins leftFront{26, 25};
    MotorPins rightFront{33, 32};

    int stepLinear = 8;
    int stepTurn = 2;
    int minPwmThreshold = 40; // Мертва зона (порог страту моторів)
};

class Chassis
{
private:
    ChassisConfig config;

    // --- Робота моторів

    // Лівий передній
    int currentLF = 0;
    int targetLF = 0;

    // Правий передній
    int currentRF = 0;
    int targetRF = 0;

    // Лівий задній
    int currentLB = 0;
    int targetLB = 0;

    // Правий задній
    int currentRB = 0;
    int targetRB = 0;

    bool isTurning = false;

    void setMotorPairRaw(const MotorPins &pins, int speed);
    int applyDeadband(int speed) const;

public:
    explicit Chassis(const ChassisConfig &cfg = ChassisConfig());
    void init();
    void setTargets(int x, int y, SteeringMode mode = SteeringMode::DIFFERENTIAL);
    void updateSmooth();
    void stopEmergency();
};

#endif