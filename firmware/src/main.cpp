#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

WebServer server(80);

Servo servos[6];

// Pins anpassen, falls nötig
const int servoPins[6] = {13, 12, 14, 27, 26, 25};

// Grundstellung: alle 90 Grad
int servoPositions[6] = {90, 90, 90, 90, 90, 90};
int servoTargets[6]   = {90, 90, 90, 90, 90, 90};

// Grenzen
const int servoMin[6] = {0, 0, 0, 20, 0, 50};
const int servoMax[6] = {180, 180, 180, 160, 180, 125};

// Access Point Daten
const char* apName = "ESP32-Roboterarm";
const char* apPassword = "robotarm123";

// Geschwindigkeit der weichen Bewegung
unsigned long lastServoUpdate = 0;
const int servoStepDelayMs = 15;
const int servoStepSize = 1;

String buildHtml() {
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
            max-width: 900px;
            margin: 20px auto;
            padding: 20px;
            background: #f4f4f4;
        }
        h1 {
            text-align: center;
        }
        .card {
            background: white;
            border-radius: 10px;
            padding: 16px;
            margin-bottom: 16px;
            box-shadow: 0 2px 6px rgba(0,0,0,0.1);
        }
        .servo-row {
            margin-bottom: 18px;
            padding-bottom: 12px;
            border-bottom: 1px solid #ddd;
        }
        .servo-row:last-child {
            border-bottom: none;
        }
        label {
            font-weight: bold;
            display: block;
            margin-bottom: 8px;
        }
        input[type=range] {
            width: 100%;
        }
        .value {
            margin-top: 6px;
            color: #333;
        }
        .hint {
            color: #666;
            font-size: 14px;
        }
    </style>
</head>
<body>
    <h1>ESP32 Roboterarm</h1>
    <div class="card">
        <p><strong>Netzwerk:</strong> AP_NAME</p>
        <p><strong>IP-Adresse:</strong> IP_PLACEHOLDER</p>
        <p class="hint">Die Servos folgen den Schiebern weich in Echtzeit.</p>
    </div>

    <div class="card">
        SERVO_ROWS
    </div>

    <script>
        let sendTimers = {};

        function scheduleServoUpdate(servoId, value) {
            document.getElementById("value" + servoId).innerText = value;

            if (sendTimers[servoId]) {
                clearTimeout(sendTimers[servoId]);
            }

            sendTimers[servoId] = setTimeout(() => {
                fetch(`/set?servo=${servoId}&angle=${value}`)
                    .then(response => response.text())
                    .then(text => console.log(text))
                    .catch(error => console.error(error));
            }, 10);
        }
    </script>
</body>
</html>
)rawliteral";

    html.replace("IP_PLACEHOLDER", WiFi.softAPIP().toString());
    html.replace("AP_NAME", String(apName));

    String rows = "";
    for (int i = 0; i < 6; i++) {
        String row = R"rawliteral(
<div class="servo-row">
    <label for="servoSERVO_ID">Servo SERVO_ID</label>
    <input
        type="range"
        id="servoSERVO_ID"
        min="SERVO_MIN"
        max="SERVO_MAX"
        value="SERVO_VALUE"
        oninput="scheduleServoUpdate(SERVO_ID, this.value)"
    >
    <div class="value">Winkel: <span id="valueSERVO_ID">SERVO_VALUE</span>°</div>
</div>
)rawliteral";

        row.replace("SERVO_ID", String(i));
        row.replace("SERVO_MIN", String(servoMin[i]));
        row.replace("SERVO_MAX", String(servoMax[i]));
        row.replace("SERVO_VALUE", String(servoTargets[i]));
        rows += row;
    }

    html.replace("SERVO_ROWS", rows);
    return html;
}

void setServoTarget(int servoId, int angle) {
    if (servoId < 0 || servoId >= 6) {
        return;
    }

    angle = constrain(angle, servoMin[servoId], servoMax[servoId]);
    servoTargets[servoId] = angle;

    Serial.print("Servo ");
    Serial.print(servoId);
    Serial.print(" Ziel -> ");
    Serial.print(angle);
    Serial.println(" Grad");
}

void updateServosSmoothly() {
    unsigned long now = millis();
    if (now - lastServoUpdate < servoStepDelayMs) {
        return;
    }
    lastServoUpdate = now;

    for (int i = 0; i < 6; i++) {
        if (servoPositions[i] < servoTargets[i]) {
            servoPositions[i] += servoStepSize;
            if (servoPositions[i] > servoTargets[i]) {
                servoPositions[i] = servoTargets[i];
            }
            servos[i].write(servoPositions[i]);
        } else if (servoPositions[i] > servoTargets[i]) {
            servoPositions[i] -= servoStepSize;
            if (servoPositions[i] < servoTargets[i]) {
                servoPositions[i] = servoTargets[i];
            }
            servos[i].write(servoPositions[i]);
        }
    }
}

void handleRoot() {
    server.send(200, "text/html", buildHtml());
}

void handleSetServo() {
    if (!server.hasArg("servo") || !server.hasArg("angle")) {
        server.send(400, "text/plain", "Fehlende Parameter");
        return;
    }

    int servoId = server.arg("servo").toInt();
    int angle = server.arg("angle").toInt();

    if (servoId < 0 || servoId >= 6) {
        server.send(400, "text/plain", "Ungueltige Servo-ID");
        return;
    }

    setServoTarget(servoId, angle);

    String response = "Servo " + String(servoId) + " Ziel gesetzt auf " + String(servoTargets[servoId]) + " Grad";
    server.send(200, "text/plain", response);
}

void handleNotFound() {
    server.send(404, "text/plain", "Seite nicht gefunden");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Starte ESP32 Access Point...");

    bool apStarted = WiFi.softAP(apName, apPassword);

    if (apStarted) {
        Serial.println("Access Point gestartet!");
        Serial.print("WLAN-Name: ");
        Serial.println(apName);
        Serial.print("IP-Adresse: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("Fehler beim Start des Access Points!");
    }

    for (int i = 0; i < 6; i++) {
        servos[i].setPeriodHertz(50);
        servos[i].attach(servoPins[i], 500, 2400);

        servoPositions[i] = 90;
        servoTargets[i] = 90;

        servoPositions[i] = constrain(servoPositions[i], servoMin[i], servoMax[i]);
        servoTargets[i] = constrain(servoTargets[i], servoMin[i], servoMax[i]);

        servos[i].write(servoPositions[i]);

        Serial.print("Servo ");
        Serial.print(i);
        Serial.print(" gestartet auf ");
        Serial.print(servoPositions[i]);
        Serial.println(" Grad");

        delay(200);
    }

    server.on("/", handleRoot);
    server.on("/set", handleSetServo);
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.println("Webserver gestartet!");
}

void loop() {
    server.handleClient();
    updateServosSmoothly();
}