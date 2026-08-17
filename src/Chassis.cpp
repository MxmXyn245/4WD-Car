#include "Chassis.h"

Chassis::Chassis(const ChassisConfig &cfg) : config(cfg) {}

void Chassis::init()
{
    const uint8_t pins[] = {
        config.leftBack.bwd, config.leftBack.fwd,
        config.rightBack.bwd, config.rightBack.fwd,
        config.leftFront.bwd, config.leftFront.fwd,
        config.rightFront.bwd, config.rightFront.fwd};

    for (uint8_t pin : pins)
    {
        pinMode(pin, OUTPUT);
        analogWrite(pin, 0);
    }
}

int Chassis::applyDeadband(int speed) const
{
    if (speed == 0)
        return 0;
    int sign = (speed > 0) ? 1 : -1;
    int absSpeed = abs(speed);

    // Масштабуємо [1..255] у [minPwmThreshold..255]
    int scaled = map(absSpeed, 1, 255, config.minPwmThreshold, 255);
    return sign * constrain(scaled, config.minPwmThreshold, 255);
}

void Chassis::setTargets(int x, int y, SteeringMode mode)
{
    // Фіксуємо стан для вибору кроку рампи (turn / linear)
    isTurning = (abs(x) > abs(y));

    // Обмеження вхідних сигналів джойстика (-100..100)
    x = constrain(x, -100, 100);
    y = constrain(y, -100, 100);

    int rawLF = 0, rawRF = 0, rawLB = 0, rawRB = 0;

    if (mode == SteeringMode::DIFFERENTIAL)
    {
        // Класичний танковий режим (МЦШ в центрі шасі при Y=0)
        int leftBase = y + x;
        int rightBase = y - x;

        rawLF = rawLB = leftBase;
        rawRF = rawRB = rightBase;
    }
    else if (mode == SteeringMode::CURVE)
    {
        // Режим МЦШ з геометрією повороту навколо задньої внутрішньої осі
        if (x > 0)
        {
            // Поворот праворуч (МЦШ праворуч від робота)
            int outerBase = y + x;
            int innerBase = y - x;

            rawLF = outerBase;                           // Зовнішнє переднє (найбільший радіус)
            rawLB = static_cast<int>(outerBase * 0.85f); // Зовнішнє заднє
            rawRF = static_cast<int>(innerBase * 0.60f); // Внутрішнє переднє
            rawRB = static_cast<int>(innerBase * 0.30f); // Внутрішнє заднє (найближче до МЦШ)
        }
        else if (x < 0)
        {
            // Поворот ліворуч (МЦШ ліворуч від робота)
            int outerBase = y - x; // оскільки x від'ємний, -x є додатнім
            int innerBase = y + x;

            rawRF = outerBase;                           // Зовнішнє переднє
            rawRB = static_cast<int>(outerBase * 0.85f); // Зовнішнє заднє
            rawLF = static_cast<int>(innerBase * 0.60f); // Внутрішнє переднє
            rawLB = static_cast<int>(innerBase * 0.30f); // Внутрішнє заднє
        }
        else
        {
            // Прямий рух
            rawLF = rawLB = rawRF = rawRB = y;
        }
    }

    // Масштабування відсотків (-100..100) у діапазон PWM (-255..255)
    targetLF = map(constrain(rawLF, -100, 100), -100, 100, -255, 255);
    targetRF = map(constrain(rawRF, -100, 100), -100, 100, -255, 255);
    targetLB = map(constrain(rawLB, -100, 100), -100, 100, -255, 255);
    targetRB = map(constrain(rawRB, -100, 100), -100, 100, -255, 255);
}

void Chassis::updateSmooth()
{
    int step = isTurning ? config.stepTurn : config.stepLinear;

    // Лямбда для обчислення інерційного кроку
    auto stepRamp = [step](int current, int target)
    {
        if (current < target)
            return min(current + step, target);
        if (current > target)
            return max(current - step, target);
        return target;
    };

    // Плавно оновлюємо поточні швидкості всіх 4 коліс
    currentLF = stepRamp(currentLF, targetLF);
    currentRF = stepRamp(currentRF, targetRF);
    currentLB = stepRamp(currentLB, targetLB);
    currentRB = stepRamp(currentRB, targetRB);

    // Відправляємо оновлені PWM-сигнали на кожен мотор окремо
    setMotorPairRaw(config.leftFront, currentLF);
    setMotorPairRaw(config.rightFront, currentRF);
    setMotorPairRaw(config.leftBack, currentLB);
    setMotorPairRaw(config.rightBack, currentRB);
}

void Chassis::stopEmergency()
{
    targetLF = targetRF = targetLB = targetRB = 0;
    currentLF = currentRF = currentLB = currentRB = 0;
    isTurning = false;

    setMotorPairRaw(config.leftFront, 0);
    setMotorPairRaw(config.rightFront, 0);
    setMotorPairRaw(config.leftBack, 0);
    setMotorPairRaw(config.rightBack, 0);
}

void Chassis::setMotorPairRaw(const MotorPins &pins, int speed)
{
    int pwm = applyDeadband(speed);

    if (pwm > 0)
    {
        analogWrite(pins.bwd, 0);
        analogWrite(pins.fwd, pwm);
    }
    else if (pwm < 0)
    {
        analogWrite(pins.bwd, -pwm);
        analogWrite(pins.fwd, 0);
    }
    else
    {
        analogWrite(pins.bwd, 0);
        analogWrite(pins.fwd, 0);
    }
}