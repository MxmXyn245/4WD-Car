#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

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

const int STEP_LINEAR = 8;
const int STEP_TURN = 2;
bool isTurningGlobal = false;

// Timer setings
unsigned long lastUpdateTime = 0;
const unsigned long UPDATE_INTERVAL = 20;

AsyncWebServer *server = nullptr;
AsyncWebSocket *ws = nullptr;

// Defining functions
void setMotor(int, int);
void updateMotorsSmooth(int, int);
void handleWebSocketMessage(void *, uint8_t *, size_t);
void onEvent(AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType, void *, uint8_t *, size_t);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>ESP32 Standalone Joystick</title>
    <style>
        body { background-color: #121212; margin: 0; padding: 0; overflow: hidden; display: flex; justify-content: center; align-items: center; height: 100vh; font-family: sans-serif; user-select: none; }
        #joystick-base { width: 200px; height: 200px; background-color: rgba(255, 255, 255, 0.1); border: 4px solid #333; border-radius: 50%; position: relative; display: flex; justify-content: center; align-items: center; }
        #joystick-stick { width: 80px; height: 80px; background-color: #007bff; border-radius: 50%; position: absolute; box-shadow: 0 0 20px rgba(0, 123, 255, 0.5); touch-action: none; }
    </style>
</head>
<body>

    <div id="joystick-base">
        <div id="joystick-stick"></div>
    </div>

    <script>
    const base = document.getElementById('joystick-base');
    const stick = document.getElementById('joystick-stick');
    
    let startX, startY;
    const maxRadius = 80;

    const gateway = "ws://192.168.4.1/ws";
    alert("Намагаюся відкрити сокет до: " + gateway);

    let websocket = new WebSocket(gateway);

    websocket.onopen = function() {
        console.log("Веб-сокет успішно відкрився!");
    };

    websocket.onerror = function(error) {
        console.log("Помилка сокета:", error);
    };

    stick.addEventListener('touchstart', (e) => {
        const touch = e.touches[0];
        startX = touch.clientX;
        startY = touch.clientY;
    });

    stick.addEventListener('touchmove', (e) => {
        const touch = e.touches[0];
        let deltaX = touch.clientX - startX;
        let deltaY = touch.clientY - startY;
        let distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY);

        if (distance > maxRadius) {
            let angle = Math.atan2(deltaY, deltaX);
            deltaX = Math.cos(angle) * maxRadius;
            deltaY = Math.sin(angle) * maxRadius;
        }

        stick.style.transform = `translate(${deltaX}px, ${deltaY}px)`;

        let xPercent = Math.round((deltaX / maxRadius) * 100);
        let yPercent = Math.round(-(deltaY / maxRadius) * 100);

        // ВІДПРАВКА ДАНИХ: Якщо сокет відкритий, шлемо "X,Y"
        if (websocket.readyState === WebSocket.OPEN) {
            websocket.send(`${xPercent},${yPercent}`);
        }
    });

    stick.addEventListener('touchend', () => {
        stick.style.transform = 'translate(0px, 0px)';
        if (websocket.readyState === WebSocket.OPEN) {
            websocket.send("0,0"); // При відпусканні скидаємо швидкість
        }
    });
</script>
</body>
</html>
)rawliteral";

void setup()
{
  Serial.begin(115200); // Вмикаємо монітор порту для діагностики
  delay(1000);
  Serial.println("\n[INFO] Система запускається...");

  pinMode(LEFT_BACK_BACKWD_PIN, OUTPUT);
  pinMode(LEFT_BACK_FWD_PIN, OUTPUT);
  pinMode(RIGHT_BACK_BACKWD_PIN, OUTPUT);
  pinMode(RIGHT_BACK_FWD_PIN, OUTPUT);
  pinMode(LEFT_FRONT_BACKWD_PIN, OUTPUT);
  pinMode(LEFT_FRONT_FWD_PIN, OUTPUT);
  pinMode(RIGHT_FRONT_BACKWD_PIN, OUTPUT);
  pinMode(RIGHT_FRONT_FWD_PIN, OUTPUT);

  Serial.println("[INFO] Ініціалізація Wi-Fi мережі...");
  WiFi.softAP("ESP32_Joystick_Car", "");

  server = new AsyncWebServer(80);
  ws = new AsyncWebSocket("/ws");

  ws->onEvent(onEvent);
  server->addHandler(ws);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("[INFO] Точка доступу піднята! IP-адреса робота: ");
  Serial.println(IP);

  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
             { request->send(200, "text/html; charset=utf-8", index_html); });

  server->begin();
  Serial.println("[INFO] Асинхронний сервер та WebSockets успішно підняті!");
}

void loop()
{

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
  // Використовуємо глобальний прапорець, який розраховується з координат джойстика
  int currentStep = isTurningGlobal ? STEP_TURN : STEP_LINEAR;

  if (currentLeftSpeed < targetLeft)
  {
    currentLeftSpeed += currentStep;
    if (currentLeftSpeed > targetLeft)
      currentLeftSpeed = targetLeft;
  }
  else if (currentLeftSpeed > targetLeft)
  {
    currentLeftSpeed -= currentStep;
    if (currentLeftSpeed < targetLeft)
      currentLeftSpeed = targetLeft;
  }

  if (currentRightSpeed < targetRight)
  {
    currentRightSpeed += currentStep;
    if (currentRightSpeed > targetRight)
      currentRightSpeed = targetRight;
  }
  else if (currentRightSpeed > targetRight)
  {
    currentRightSpeed -= currentStep;
    if (currentRightSpeed < targetRight)
      currentRightSpeed = targetRight;
  }

  setMotor(currentLeftSpeed, currentRightSpeed);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->opcode == WS_TEXT)
  {

    // Створюємо рядок безпосередньо з масиву байтів вказаної довжини
    String msg = String((char *)data).substring(0, len);

    int commaIndex = msg.indexOf(',');
    if (commaIndex != -1)
    {
      String xStr = msg.substring(0, commaIndex);
      String yStr = msg.substring(commaIndex + 1);

      int x = xStr.toInt();
      int y = yStr.toInt();

      isTurningGlobal = (abs(x) > abs(y));

      int leftTarget = y + x;
      int rightTarget = y - x;

      finalLeftSpeed = map(leftTarget, -100, 100, -255, 255);
      finalRightSpeed = map(rightTarget, -100, 100, -255, 255);

      finalLeftSpeed = constrain(finalLeftSpeed, -255, 255);
      finalRightSpeed = constrain(finalRightSpeed, -255, 255);
    }
  }
}

// Головний асинхронний обробник подій веб-сокета
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("[WS] Клієнт підключився зі стріма: %u\n", client->id());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("[WS] Клієнт відключився\n");
    finalLeftSpeed = 0; // Безпека: якщо пропав зв'язок - машина стоп
    finalRightSpeed = 0;
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  default:
    break;
  }
}