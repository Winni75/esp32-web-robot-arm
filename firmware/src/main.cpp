#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "FRITZ!Box 7530 MI-2G&5G";
const char* password = "72335155615336606495";

WebServer server(80);

void handleRoot() {
    Serial.println("Browser Anfrage empfangen!");
    server.send(200, "text/plain", "ESP32 Server funktioniert!");
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
    WiFi.begin(ssid, password);

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