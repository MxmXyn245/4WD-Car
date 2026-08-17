#include "WebServerManager.h"
#include "index_html.h"

WebServerManager::WebServerManager(Chassis *targetChassis)
    : server(80), ws("/ws"), chassis(targetChassis) {}

void WebServerManager::begin()
{
    WiFi.softAP("ESP32_Joystick_Car", "");

    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len)
               {
        if (type == WS_EVT_CONNECT) {
            Serial.printf("[WS] Клієнт підключився: %u\n", client->id());
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.println("[WS] Клієнт відключився! Аварійна зупинка.");
            this->chassis->stopEmergency();
        } else if (type == WS_EVT_DATA) {
            this->handleMessage(arg, data, len);
        } });

    server.addHandler(&ws);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html; charset=utf-8", index_html); });

    server.begin();
    Serial.println("[INFO] Сервер запущен!");
}

extern QueueHandle_t xCmdQueue;

struct CommandMsg
{
    int x;
    int y;
};

void WebServerManager::handleMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->opcode == WS_TEXT)
    {
        char buf[32];
        size_t copyLen = (len < sizeof(buf) - 1) ? len : sizeof(buf) - 1;
        memcpy(buf, data, copyLen);
        buf[copyLen] = '\0';

        // ДЕБАГ: виводимо сирий рядок із джойстика в Serial
        Serial.printf("[WS RAW]: %s\n", buf);

        int x = 0, y = 0;
        if (sscanf(buf, "%d,%d", &x, &y) == 2)
        {
            CommandMsg cmd = {x, y};
            // Неблокуючий запис у чергу
            if (xCmdQueue != NULL)
            {
                xQueueSend(xCmdQueue, &cmd, 0);
            }
        }
    }
}