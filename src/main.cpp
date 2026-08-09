#include <Arduino.h>
#include "Chassis.h"
#include "WebServerManager.h"

// Створюємо екземпляри класів
Chassis chassis;
WebServerManager webServer(&chassis); // Передаємо шасі серверу за посиланням

unsigned long lastUpdateTime = 0;
const unsigned long UPDATE_INTERVAL = 20;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[SYSTEM] Старт системи в архітектурі ООП...");

  // Ініціалізуємо шасі (піни)
  chassis.init();

  // Запускаємо мережу та сервери
  webServer.begin();
}

void loop()
{
  // Неблокуючий таймер рампи працює суворо у своїй часовій площині
  if (millis() - lastUpdateTime >= UPDATE_INTERVAL)
  {
    lastUpdateTime = millis();
    chassis.updateSmooth();
  }
}