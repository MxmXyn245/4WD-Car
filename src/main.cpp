#include <Arduino.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Pinouts settings
const int LEFT_BACK_BACKWD_PIN = 4;
const int LEFT_BACK_FWD_PIN = 16;
const int RIGHT_BACK_BACKWD_PIN = 17;
const int RIGHT_BACK_FWD_PIN = 18;
const int LEFT_FRONT_BACKWD_PIN = 26;
const int LEFT_FRONT_FWD_PIN = 25;
const int RIGHT_FRONT_BACKWD_PIN = 33;
const int RIGHT_FRONT_FWD_PIN = 32;

// Velocity motor data
int currentLeftSpeed = 0;
int currentRightSpeed = 0;
int finalLeftSpeed = 0;
int finalRightSpeed = 0;
const int SPEED_STEP = 5;

// Timer setings
unsigned long lastUpdateTime = 0;
const unsigned long UPDATE_INTERVAL = 20;

BluetoothSerial SerialBT;

// Defining functions
void setMotor(int, int);
void updateMotorsSmooth(int, int);

void setup()
{
  Serial.begin(115200); // Вмикаємо монітор порту для діагностики
  delay(1000);

  pinMode(LEFT_BACK_BACKWD_PIN, OUTPUT);
  pinMode(LEFT_BACK_FWD_PIN, OUTPUT);

  pinMode(RIGHT_BACK_BACKWD_PIN, OUTPUT);
  pinMode(RIGHT_BACK_FWD_PIN, OUTPUT);

  pinMode(LEFT_FRONT_BACKWD_PIN, OUTPUT);
  pinMode(LEFT_FRONT_FWD_PIN, OUTPUT);

  pinMode(RIGHT_FRONT_BACKWD_PIN, OUTPUT);
  pinMode(RIGHT_FRONT_FWD_PIN, OUTPUT);

  Serial.println("Запуск Bluetooth...");

  // Перевіряємо результат запуску Bluetooth
  if (!SerialBT.begin("ESP32_Robot_Car"))
  {
    Serial.println("❌ КРИТИЧНА ПОМИЛКА: Не вдалося виділити пам'ять під Bluetooth!");
  }
  else
  {
    Serial.println("🚀 БЛЮТУЗ ЗАПУЩЕНО! Шукай пристрій: ESP32_Robot_Car");
  }
}

void loop()
{
  if (SerialBT.available())
  {
    char input = SerialBT.read();

    switch (input)
    {
    case 'F':
      finalLeftSpeed = 255;
      finalRightSpeed = 255;
      break;
    case 'B':
      finalLeftSpeed = -255;
      finalRightSpeed = -255;
      break;
    case 'S':
      finalLeftSpeed = 0;
      finalRightSpeed = 0;
      break;
    case 'L':
      finalLeftSpeed = 255;
      finalRightSpeed = -255;
      break;
    case 'R':
      finalLeftSpeed = -255;
      finalRightSpeed = 255;
      break;
    }
  }

  if (millis() - lastUpdateTime >= UPDATE_INTERVAL)
  {
    lastUpdateTime = millis();
    updateMotorsSmooth(finalLeftSpeed, finalRightSpeed);
  }
}

void setMotor(int leftSpeed, int rightSpeed)
{
  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);

  if (leftSpeed > 0)
  {
    analogWrite(LEFT_BACK_BACKWD_PIN, 0);
    analogWrite(LEFT_BACK_FWD_PIN, leftSpeed);

    analogWrite(LEFT_FRONT_BACKWD_PIN, 0);
    analogWrite(LEFT_FRONT_FWD_PIN, leftSpeed);
  }
  else if (leftSpeed < 0)
  {
    analogWrite(LEFT_BACK_BACKWD_PIN, -leftSpeed);
    analogWrite(LEFT_BACK_FWD_PIN, 0);

    analogWrite(LEFT_FRONT_BACKWD_PIN, -leftSpeed);
    analogWrite(LEFT_FRONT_FWD_PIN, 0);
  }
  else
  {
    analogWrite(LEFT_BACK_BACKWD_PIN, 0);
    analogWrite(LEFT_BACK_FWD_PIN, 0);

    analogWrite(LEFT_FRONT_BACKWD_PIN, 0);
    analogWrite(LEFT_FRONT_FWD_PIN, 0);
  }

  if (rightSpeed > 0)
  {
    analogWrite(RIGHT_BACK_BACKWD_PIN, 0);
    analogWrite(RIGHT_BACK_FWD_PIN, rightSpeed);

    analogWrite(RIGHT_FRONT_BACKWD_PIN, 0);
    analogWrite(RIGHT_FRONT_FWD_PIN, rightSpeed);
  }
  else if (rightSpeed < 0)
  {
    analogWrite(RIGHT_BACK_BACKWD_PIN, -rightSpeed);
    analogWrite(RIGHT_BACK_FWD_PIN, 0);

    analogWrite(RIGHT_FRONT_BACKWD_PIN, -rightSpeed);
    analogWrite(RIGHT_FRONT_FWD_PIN, 0);
  }
  else
  {
    analogWrite(RIGHT_BACK_BACKWD_PIN, 0);
    analogWrite(RIGHT_BACK_FWD_PIN, 0);

    analogWrite(RIGHT_FRONT_BACKWD_PIN, 0);
    analogWrite(RIGHT_FRONT_FWD_PIN, 0);
  }
}

void updateMotorsSmooth(int targetLeft, int targetRight)
{
  // Left side
  if (currentLeftSpeed < targetLeft)
  {
    currentLeftSpeed += SPEED_STEP;
    if (currentLeftSpeed > targetLeft)
      currentLeftSpeed = targetLeft;
  }
  else if (currentLeftSpeed > targetLeft)
  {
    currentLeftSpeed -= SPEED_STEP;
    if (currentLeftSpeed < targetLeft)
      currentLeftSpeed = targetLeft;
  }

  // Ride side
  if (currentRightSpeed < targetRight)
  {
    currentRightSpeed += SPEED_STEP;
    if (currentRightSpeed > targetRight)
      currentRightSpeed = targetRight;
  }
  else if (currentRightSpeed > targetRight)
  {
    currentRightSpeed -= SPEED_STEP;
    if (currentRightSpeed < targetRight)
      currentRightSpeed = targetRight;
  }

  setMotor(currentLeftSpeed, currentRightSpeed);
}