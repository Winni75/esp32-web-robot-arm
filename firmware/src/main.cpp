#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"

WebServer server(80);

void handleRoot() {
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
        }
        button {
            padding: 15px 25px;
            margin: 10px;
            font-size: 18px;
            cursor: pointer;
            border: none;
            border-radius: 8px;
            background-color: #4CAF50;
            color: white;
        }
        button:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>
    <h1>ESP32 Roboterarm</h1>
    <p>Webserver läuft erfolgreich.</p>
    <button onclick="alert('Button 1 wurde geklickt')">Button 1</button>
    <button onclick="alert('Button 2 wurde geklickt')">Button 2</button>
</body>
</html>
)rawliteral";

    Serial.println("Browser Anfrage auf / empfangen");
    server.send(200, "text/html", html);
}

void handleNotFound() {
    Serial.print("Unbekannte Anfrage: ");
    Serial.println(server.uri());
    server.send(404, "text/plain", "Seite nicht gefunden");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

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
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.println("Webserver gestartet!");
}

void loop() {
    server.handleClient();
}