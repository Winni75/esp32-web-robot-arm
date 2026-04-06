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

// Presets
int presets[3][6] = {
    {90, 90, 90, 90, 90, 90},
    {90, 90, 90, 90, 90, 90},
    {90, 90, 90, 90, 90, 90}
};

// Grenzen
const int servoMin[6] = {0, 0, 0, 20, 0, 50};
const int servoMax[6] = {180, 180, 180, 160, 180, 125};

// WLAN
const char* apName = "ESP32-Roboterarm";
const char* apPassword = "robotarm123";

// Bewegung
unsigned long lastServoUpdate = 0;
const int servoStepDelayMs = 15;
const int servoStepSize = 1;

// Gedrückt halten
int servoMoveDirection[6] = {0};
unsigned long lastHoldUpdate = 0;
const int holdStepDelayMs = 80;
const int holdStepSize = 2;

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
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">

<style>
body {
    font-family: Arial;
    text-align: center;
    background: #f4f4f4;
}

h1 {
    margin-bottom: 20px;
}

.card {
    background: white;
    padding: 15px;
    margin: 10px auto;
    border-radius: 10px;
    width: 90%;
    max-width: 500px;
    box-shadow: 0 2px 6px rgba(0,0,0,0.1);
}

.servo-row {
    margin-bottom: 15px;
}

.value {
    font-size: 20px;
    font-weight: bold;
    margin-bottom: 10px;
}

.button-group {
    display: flex;
    justify-content: center;
    gap: 20px;
}

button {
    width: 100px;
    height: 70px;
    font-size: 28px;
    border: none;
    border-radius: 12px;
    color: white;
}

.plus {
    background-color: #28a745;
}

.minus {
    background-color: #dc3545;
}

.stop {
    background-color: black;
    width: 150px;
}

.seq {
    background-color: #007bff;
    width: 200px;
}
</style>

</head>
<body>

<h1>ESP32 Roboterarm</h1>

<div class="card">
    <button class="seq" onclick="startSequence()">Sequenz starten</button><br><br>
    <button class="stop" onclick="stopSequence()">STOP</button>
</div>

SERVO_ROWS

<script>

function startMove(id, dir) {
    fetch(`/startMove?servo=${id}&dir=${dir}`);
}

function stopMove(id) {
    fetch(`/stopMove?servo=${id}`);
}

function stopAll() {
    for (let i = 0; i < 6; i++) {
        fetch(`/stopMove?servo=${i}`);
    }
}

// GLOBAL STOP
document.addEventListener("mouseup", stopAll);
document.addEventListener("touchend", stopAll);
document.addEventListener("touchcancel", stopAll);

function startSequence() {
    fetch('/sequence');
}

function stopSequence() {
    fetch('/stop');
}

// Positionsanzeige
function refreshPositions() {
    fetch('/positions')
    .then(res => res.json())
    .then(data => {
        for (let i = 0; i < data.length; i++) {
            document.getElementById("val"+i).innerText = data[i] + "°";
        }
    })
    .catch(err => console.error("Fehler bei /positions:", err));
}

setInterval(refreshPositions, 300);

</script>

</body>
</html>
)rawliteral";

    String rows = "";

    for (int i = 0; i < 6; i++) {

        String minusEvents =
            "onmousedown=\"startMove(" + String(i) + ",-1)\" "
            "onmouseup=\"stopMove(" + String(i) + ")\" "
            "onmouseleave=\"stopMove(" + String(i) + ")\" "
            "ontouchstart=\"startMove(" + String(i) + ",-1)\" "
            "ontouchend=\"stopMove(" + String(i) + ")\"";

        String plusEvents =
            "onmousedown=\"startMove(" + String(i) + ",1)\" "
            "onmouseup=\"stopMove(" + String(i) + ")\" "
            "onmouseleave=\"stopMove(" + String(i) + ")\" "
            "ontouchstart=\"startMove(" + String(i) + ",1)\" "
            "ontouchend=\"stopMove(" + String(i) + ")\"";

        rows +=
        "<div class='card'>"
        "<div class='servo-row'>"
        "<h3>Servo " + String(i) + "</h3>"
        "<div class='value' id='val" + String(i) + "'>" + String(servoPositions[i]) + "°</div>"
        "<div class='button-group'>"
        "<button class='minus' " + minusEvents + ">-</button>"
        "<button class='plus' " + plusEvents + ">+</button>"
        "</div>"
        "</div>"
        "</div>";
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

void updateHeldButtons() {
    if (millis() - lastHoldUpdate < holdStepDelayMs) return;
    lastHoldUpdate = millis();

    for (int i = 0; i < 6; i++) {
        if (servoMoveDirection[i] != 0) {
            setServoTarget(i, servoTargets[i] + servoMoveDirection[i] * holdStepSize);
        }
    }
}

void updateServosSmoothly() {
    if (millis() - lastServoUpdate < servoStepDelayMs) return;
    lastServoUpdate = millis();

    for (int i = 0; i < 6; i++) {
        if (servoPositions[i] < servoTargets[i]) {
            servoPositions[i]++;
        } else if (servoPositions[i] > servoTargets[i]) {
            servoPositions[i]--;
        }
        servos[i].write(servoPositions[i]);
    }
}

// -------------------------
// Presets
// -------------------------
void loadPreset(int idx) {
    for (int i = 0; i < 6; i++) {
        servoTargets[i] = presets[idx][i];
    }
}

// -------------------------
// Sequenz
// -------------------------
void startSequenceInternal() {
    sequenceRunning = true;
    sequenceStep = 0;
    lastStepTime = millis();
}

void stopSequenceInternal() {
    sequenceRunning = false;
    for (int i = 0; i < 6; i++) {
        servoTargets[i] = servoPositions[i];
        servoMoveDirection[i] = 0;
    }
}

void updateSequence() {
    if (!sequenceRunning) return;

    if (millis() - lastStepTime < stepDelay) return;

    lastStepTime = millis();

    loadPreset(sequenceStep);
    sequenceStep++;

    if (sequenceStep >= 3) {
        sequenceRunning = false;
    }
}

// -------------------------
// Webserver
// -------------------------
void handleRoot() {
    server.send(200, "text/html", buildHtml());
}

void handleStartMove() {
    int id = server.arg("servo").toInt();
    int dir = server.arg("dir").toInt();

    sequenceRunning = false;
    servoMoveDirection[id] = dir;

    server.send(200, "text/plain", "move start");
}

void handleStopMove() {
    int id = server.arg("servo").toInt();

    servoMoveDirection[id] = 0;
    servoTargets[id] = servoPositions[id];

    server.send(200, "text/plain", "move stop");
}

void handleSequence() {
    startSequenceInternal();
    server.send(200, "text/plain", "sequence");
}

void handleStop() {
    stopSequenceInternal();
    server.send(200, "text/plain", "stop");
}

void handlePositions() {
    String json = "[";

    for (int i = 0; i < 6; i++) {
        json += String(servoPositions[i]);
        if (i < 5) json += ",";
    }

    json += "]";
    server.send(200, "application/json", json);
}

// -------------------------
// Setup
// -------------------------
void setup() {
    Serial.begin(115200);

    WiFi.softAP(apName, apPassword);

    for (int i = 0; i < 6; i++) {
        servos[i].attach(servoPins[i], 500, 2400);
        servos[i].write(90);
    }

    server.on("/", handleRoot);
    server.on("/startMove", handleStartMove);
    server.on("/stopMove", handleStopMove);
    server.on("/sequence", handleSequence);
    server.on("/stop", handleStop);
    server.on("/positions", handlePositions);

    server.begin();
}

// -------------------------
// Loop
// -------------------------
void loop() {
    server.handleClient();
    updateHeldButtons();
    updateServosSmoothly();
    updateSequence();
}