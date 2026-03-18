#include <Arduino.h>
#include <WiFi.h>

const char* ssid = "Deine ssID";
const char* password = "Dein Passwort";


void setup() {
     Serial.begin(115200);
    delay(1000);

    Serial.println("Verbinde mit WLAN...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    } 

    Serial.println("");
    Serial.println("WLAN verbunden!");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    
}