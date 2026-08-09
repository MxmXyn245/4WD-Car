#include "WebServerManager.h"
#include "index_html.h"

static WebServerManager *instance = nullptr;

WebServerManager::WebServerManager(Chassis *targetChassis)
{
    chassis = targetChassis;
    server = new AsyncWebServer(80);
    ws = new AsyncWebSocket("/ws");
}

void WebServerManager::begin()
{
    WiFi.softAP("ESP32_Joystick_Car", "");
    Serial.print("[INFO] Точка доступу піднята. IP: ");
    Serial.println(WiFi.softAPIP());

    instance = this;

    ws->onEvent(onEvent);
    server->addHandler(ws);

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(200, "text/html; charset=utf-8", index_html); });

    server->begin();
    Serial.println("[INFO] Мережеві сервіси ООП успішно підняті!");
}

void WebServerManager::handleMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->opcode == WS_TEXT)
    {
        String msg = String((char *)data).substring(0, len);
        int commaIndex = msg.indexOf(',');
        if (commaIndex != -1)
        {
            int x = msg.substring(0, commaIndex).toInt();
            int y = msg.substring(commaIndex + 1).toInt();

            chassis->setTargets(x, y);
        }
    }
}

void WebServerManager::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                               AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (!instance)
        return;

    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("[WS] Клієнт підключився ID: %u\n", client->id());
        break;
    case WS_EVT_DISCONNECT:
        Serial.println("[WS] Клієнт відключився");
        instance->chassis->stopEmergency();
        break;
    case WS_EVT_DATA:
        instance->handleMessage(arg, data, len);
        break;
    default:
        break;
    }
}