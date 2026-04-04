#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

WebServer server(80);

Servo servos[6];

// Pins
const int servoPins[6] = {13, 12, 14, 27, 26, 25};

// Positionen
int servoPositions[6] = {90, 90, 90, 90, 90, 90};
int servoTargets[6]   = {90, 90, 90, 90, 90, 90};

// Presets (3 Stück)
int presets[3][6] = {
    {90, 90, 90, 90, 90, 90},
    {90, 90, 90, 90, 90, 90},
    {90, 90, 90, 90, 90, 90}
};

// Grenzen
const int servoMin[6] = {0, 0, 0, 20, 0, 50};
const int servoMax[6] = {180, 180, 180, 160, 180, 125};

// Access Point
const char* apName = "ESP32-Roboterarm";
const char* apPassword = "robotarm123";

// Bewegung
unsigned long lastServoUpdate = 0;
const int servoStepDelayMs = 15;
const int servoStepSize = 1;

// Sequenz
bool sequenceRunning = false;
int sequenceStep = 0;
unsigned long lastStepTime = 0;
const int stepDelay = 3000;

// -------------------------
// HTML
// -------------------------
String buildHtml() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Roboterarm</title>
<style>
body { font-family: Arial; max-width: 900px; margin: auto; }
.card { background: #fff; padding: 15px; margin: 10px; border-radius: 10px; }
button { padding: 10px; margin: 5px; }
</style>
</head>
<body>

<h1>ESP32 Roboterarm</h1>

<div class="card">
<button onclick="fetch('/sequence')">Sequenz starten</button>
</div>

<div class="card">
<button onclick="fetch('/save?preset=0')">Preset 1 speichern</button>
<button onclick="fetch('/load?preset=0')">Preset 1 laden</button>
<br>
<button onclick="fetch('/save?preset=1')">Preset 2 speichern</button>
<button onclick="fetch('/load?preset=1')">Preset 2 laden</button>
<br>
<button onclick="fetch('/save?preset=2')">Preset 3 speichern</button>
<button onclick="fetch('/load?preset=2')">Preset 3 laden</button>
</div>

SERVO_ROWS

<script>
function setServo(id, value) {
    fetch(`/set?servo=${id}&angle=${value}`);
}
</script>

</body>
</html>
)rawliteral";

    String rows = "";
    for (int i = 0; i < 6; i++) {
        rows += "<div class='card'>Servo " + String(i) +
                "<br><input type='range' min='" + String(servoMin[i]) +
                "' max='" + String(servoMax[i]) +
                "' value='" + String(servoTargets[i]) +
                "' oninput='setServo(" + String(i) + ", this.value)'></div>";
    }

    html.replace("SERVO_ROWS", rows);
    return html;
}

// -------------------------
// Servo Steuerung
// -------------------------
void setServoTarget(int id, int angle) {
    angle = constrain(angle, servoMin[id], servoMax[id]);
    servoTargets[id] = angle;
}

void updateServosSmoothly() {
    if (millis() - lastServoUpdate < servoStepDelayMs) return;
    lastServoUpdate = millis();

    for (int i = 0; i < 6; i++) {
        if (servoPositions[i] < servoTargets[i]) {
            servoPositions[i]++;
            servos[i].write(servoPositions[i]);
        } else if (servoPositions[i] > servoTargets[i]) {
            servoPositions[i]--;
            servos[i].write(servoPositions[i]);
        }
    }
}

// -------------------------
// Presets
// -------------------------
void savePreset(int idx) {
    for (int i = 0; i < 6; i++) {
        presets[idx][i] = servoPositions[i];
    }
}

void loadPreset(int idx) {
    for (int i = 0; i < 6; i++) {
        servoTargets[i] = presets[idx][i];
    }
}

// -------------------------
// Sequenz
// -------------------------
void startSequence() {
    sequenceRunning = true;
    sequenceStep = 0;
    lastStepTime = millis();
    Serial.println("Sequenz gestartet");
}

void updateSequence() {
    if (!sequenceRunning) return;

    if (millis() - lastStepTime < stepDelay) return;

    lastStepTime = millis();

    loadPreset(sequenceStep);
    sequenceStep++;

    if (sequenceStep >= 3) {
        sequenceRunning = false;
        Serial.println("Sequenz beendet");
    }
}

// -------------------------
// Webserver
// -------------------------
void handleRoot() {
    server.send(200, "text/html", buildHtml());
}

void handleSet() {
    int id = server.arg("servo").toInt();
    int angle = server.arg("angle").toInt();
    setServoTarget(id, angle);
    server.send(200, "text/plain", "ok");
}

void handleSave() {
    int p = server.arg("preset").toInt();
    savePreset(p);
    server.send(200, "text/plain", "saved");
}

void handleLoad() {
    int p = server.arg("preset").toInt();
    loadPreset(p);
    server.send(200, "text/plain", "loaded");
}

void handleSequence() {
    startSequence();
    server.send(200, "text/plain", "sequence started");
}

// -------------------------
// Setup
// -------------------------
void setup() {
    Serial.begin(115200);

    WiFi.softAP(apName, apPassword);

    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());

    for (int i = 0; i < 6; i++) {
        servos[i].attach(servoPins[i], 500, 2400);
        servos[i].write(90);
    }

    server.on("/", handleRoot);
    server.on("/set", handleSet);
    server.on("/save", handleSave);
    server.on("/load", handleLoad);
    server.on("/sequence", handleSequence);

    server.begin();
}

// -------------------------
// Loop
// -------------------------
void loop() {
    server.handleClient();
    updateServosSmoothly();
    updateSequence();
}