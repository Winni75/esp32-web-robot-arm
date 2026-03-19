#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"

WebServer server(80);

const int LED_PIN = 2;
bool ledStatus = false;

void handleRoot() {
    String statusText = ledStatus ? "AN" : "AUS";

    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Roboterarm</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin-top: 40px;
            background-color: #f5f5f5;
        }
        h1 {
            color: #333;
        }
        p {
            color: #666;
            font-size: 18px;
        }
        button {
            padding: 15px 25px;
            margin: 10px;
            font-size: 18px;
            cursor: pointer;
            border: none;
            border-radius: 8px;
            color: white;
        }
        .an {
            background-color: #4CAF50;
        }
        .aus {
            background-color: #d9534f;
        }
        button:hover {
            opacity: 0.9;
        }
    </style>
</head>
<body>
    <h1>ESP32 Roboterarm</h1>
    <p>LED Status: STATUS_TEXT</p>
    <button class="an" onclick="window.location.href='/led-an'">LED AN</button>
    <button class="aus" onclick="window.location.href='/led-aus'">LED AUS</button>
</body>
</html>
)rawliteral";

    html.replace("STATUS_TEXT", statusText);

    Serial.println("Browser Anfrage auf / empfangen");
    server.send(200, "text/html", html);
}

void handleLedAn() {
    digitalWrite(LED_PIN, HIGH);
    ledStatus = true;
    Serial.println("LED wurde eingeschaltet");
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleLedAus() {
    digitalWrite(LED_PIN, LOW);
    ledStatus = false;
    Serial.println("LED wurde ausgeschaltet");
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleNotFound() {
    Serial.print("Unbekannte Anfrage: ");
    Serial.println(server.uri());
    server.send(404, "text/plain", "Seite nicht gefunden");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.println("Verbinde mit WLAN...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WLAN verbunden!");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/led-an", handleLedAn);
    server.on("/led-aus", handleLedAus);
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.println("Webserver gestartet!");
}

void loop() {
    server.handleClient();
}