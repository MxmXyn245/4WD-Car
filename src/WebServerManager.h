#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "Chassis.h"

class WebServerManager
{
private:
    AsyncWebServer *server;
    AsyncWebSocket *ws;
    Chassis *chassis; // Посилання на шасі для передачі команд

    void handleMessage(void *arg, uint8_t *data, size_t len);

    // Статичний метод для обробки подій бібліотеки
    static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                        AwsEventType type, void *arg, uint8_t *data, size_t len);

public:
    WebServerManager(Chassis *targetChassis);
    void begin();
};

#endif