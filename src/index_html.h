#ifndef INDEX_HTML_H
#define INDEX_HTML_H

#include <Arduino.h>

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
    let websocket = new WebSocket(gateway);

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

        if (websocket.readyState === WebSocket.OPEN) {
            websocket.send(`${xPercent},${yPercent}`);
        }
    });

    stick.addEventListener('touchend', () => {
        stick.style.transform = 'translate(0px, 0px)';
        if (websocket.readyState === WebSocket.OPEN) {
            websocket.send("0,0");
        }
    });
</script>
</body>
</html>
)rawliteral";

#endif