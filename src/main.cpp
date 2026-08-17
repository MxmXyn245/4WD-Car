#include <Arduino.h>
#include "Chassis.h"
#include "WebServerManager.h"

// Структура для передачі векторів руху через FreeRTOS Queue
struct CommandMsg
{
  int x;
  int y;
};

// Глобальні об'єкти та примітиви FreeRTOS
Chassis chassis;
WebServerManager webServer(&chassis);
QueueHandle_t xCmdQueue = NULL;

// Таска для детермінованого оновлення рампи шасі (50 Гц)
void chassisTask(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(20); // 20 мс = 50 Гц

  CommandMsg incomingCmd;
  TickType_t xLastCmdTime = xTaskGetTickCount();
  const TickType_t xTimeout = pdMS_TO_TICKS(500); // 500 мс Fail-Safe таймаут

  for (;;)
  {
    // Вичитуємо всі накопичені команди з черги
    while (xQueueReceive(xCmdQueue, &incomingCmd, 0) == pdTRUE)
    {
      chassis.setTargets(incomingCmd.x, incomingCmd.y, SteeringMode::DIFFERENTIAL);
      xLastCmdTime = xTaskGetTickCount();
    }

    // Software Watchdog: якщо від WebSocket немає сигналів > 500 мс — зупинка
    if ((xTaskGetTickCount() - xLastCmdTime) > xTimeout)
    {
      chassis.stopEmergency();
    }
    else
    {
      chassis.updateSmooth();
    }

    // Забезпечує строго точний інтервал виконання незалежно від часу роботи функцій
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

void setup()
{
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH); // Світлодіод запрямиться, якщо setup() запустився

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- START ---");

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[SYSTEM] Ініціалізація системи з FreeRTOS...");

  // Ініціалізація заліза
  chassis.init();

  // Створення потокобезпечної черги на 10 елементів
  xCmdQueue = xQueueCreate(10, sizeof(CommandMsg));

  if (xCmdQueue != NULL)
  {
    // Створення таски шасі на Core 1 (Core 0 зайнятий системою/Wi-Fi)
    xTaskCreatePinnedToCore(
        chassisTask,   // Функція таски
        "ChassisTask", // Назва
        3072,          // Розмір стеку (в байтах)
        NULL,          // Параметри
        2,             // Пріоритет
        NULL,          // Хендл
        1              // Ядро (Core 1)
    );
    Serial.println("[RTOS] Chassis Task успішно запущена на Core 1");
  }
  else
  {
    Serial.println("[ERROR] Не вдалося створити FreeRTOS Queue!");
  }

  // Запуск Wi-Fi та WebServer
  webServer.begin();
}

void loop()
{
  // Вся робота шасі винесена у FreeRTOS Task.
  // loop() залишається вільним або використовується для системного моніторингу.
  vTaskDelay(pdMS_TO_TICKS(1000));
}