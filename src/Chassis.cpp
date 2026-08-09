#include "Chassis.h"

Chassis::Chassis() {}

void Chassis::init()
{
    pinMode(pinLB_Bwd, OUTPUT);
    pinMode(pinLB_Fwd, OUTPUT);
    pinMode(pinRB_Bwd, OUTPUT);
    pinMode(pinRB_Fwd, OUTPUT);
    pinMode(pinLF_Bwd, OUTPUT);
    pinMode(pinLF_Fwd, OUTPUT);
    pinMode(pinRF_Bwd, OUTPUT);
    pinMode(pinRF_Fwd, OUTPUT);
}

void Chassis::setTargets(int x, int y)
{
    isTurning = (abs(x) > abs(y));

    int leftTarget = y + x;
    int rightTarget = y - x;

    targetLeftSpeed = map(leftTarget, -100, 100, -255, 255);
    targetRightSpeed = map(rightTarget, -100, 100, -255, 255);

    targetLeftSpeed = constrain(targetLeftSpeed, -255, 255);
    targetRightSpeed = constrain(targetRightSpeed, -255, 255);
}

void Chassis::updateSmooth()
{
    int currentStep = isTurning ? STEP_TURN : STEP_LINEAR;

    // Рампа для лівого борту
    if (currentLeftSpeed < targetLeftSpeed)
    {
        currentLeftSpeed += currentStep;
        if (currentLeftSpeed > targetLeftSpeed)
            currentLeftSpeed = targetLeftSpeed;
    }
    else if (currentLeftSpeed > targetLeftSpeed)
    {
        currentLeftSpeed -= currentStep;
        if (currentLeftSpeed < targetLeftSpeed)
            currentLeftSpeed = targetLeftSpeed;
    }

    // Рампа для правого борту
    if (currentRightSpeed < targetRightSpeed)
    {
        currentRightSpeed += currentStep;
        if (currentRightSpeed > targetRightSpeed)
            currentRightSpeed = targetRightSpeed;
    }
    else if (currentRightSpeed > targetRightSpeed)
    {
        currentRightSpeed -= currentStep;
        if (currentRightSpeed < targetRightSpeed)
            currentRightSpeed = targetRightSpeed;
    }

    setMotorRaw(currentLeftSpeed, currentRightSpeed);
}

void Chassis::stopEmergency()
{
    targetLeftSpeed = 0;
    targetRightSpeed = 0;
    currentLeftSpeed = 0;
    currentRightSpeed = 0;
    setMotorRaw(0, 0);
    isTurning = false;
}

void Chassis::setMotorRaw(int leftSpeed, int rightSpeed)
{
    // Лівий борт
    if (leftSpeed > 0)
    {
        analogWrite(pinLB_Bwd, 0);
        analogWrite(pinLB_Fwd, leftSpeed);
        analogWrite(pinLF_Bwd, 0);
        analogWrite(pinLF_Fwd, leftSpeed);
    }
    else if (leftSpeed < 0)
    {
        analogWrite(pinLB_Bwd, -leftSpeed);
        analogWrite(pinLB_Fwd, 0);
        analogWrite(pinLF_Bwd, -leftSpeed);
        analogWrite(pinLF_Fwd, 0);
    }
    else
    {
        analogWrite(pinLB_Bwd, 0);
        analogWrite(pinLB_Fwd, 0);
        analogWrite(pinLF_Bwd, 0);
        analogWrite(pinLF_Fwd, 0);
    }

    // Правий борт
    if (rightSpeed > 0)
    {
        analogWrite(pinRB_Bwd, 0);
        analogWrite(pinRB_Fwd, rightSpeed);
        analogWrite(pinRF_Bwd, 0);
        analogWrite(pinRF_Fwd, rightSpeed);
    }
    else if (rightSpeed < 0)
    {
        analogWrite(pinRB_Bwd, -rightSpeed);
        analogWrite(pinRB_Fwd, 0);
        analogWrite(pinRF_Bwd, -rightSpeed);
        analogWrite(pinRF_Fwd, 0);
    }
    else
    {
        analogWrite(pinRB_Bwd, 0);
        analogWrite(pinRB_Fwd, 0);
        analogWrite(pinRF_Bwd, 0);
        analogWrite(pinRF_Fwd, 0);
    }
}