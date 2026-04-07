#include <Arduino.h>
#include <ESP32Servo.h>
#include <WebServer.h>
#include <WiFi.h>
#include <vector>

namespace {

constexpr uint8_t kServoCount = 6;
constexpr uint16_t kHttpPort = 80;
constexpr uint32_t kServoStepDelayMs = 15;
constexpr int kServoStepSize = 1;
constexpr uint32_t kHoldStepDelayMs = 80;
constexpr int kHoldStepSize = 2;
constexpr uint32_t kSequenceHoldMs = 1000;
constexpr uint16_t kServoPulseMinUs = 500;
constexpr uint16_t kServoPulseMaxUs = 2400;
constexpr uint16_t kSerialBaudRate = 115200;

#ifndef ROBOT_ARM_AP_NAME
#define ROBOT_ARM_AP_NAME "ESP32-Roboterarm"
#endif

#ifndef ROBOT_ARM_AP_PASSWORD
#define ROBOT_ARM_AP_PASSWORD "robotarm123"
#endif

constexpr char kApName[] = ROBOT_ARM_AP_NAME;
constexpr char kApPassword[] = ROBOT_ARM_AP_PASSWORD;

constexpr int kServoPins[kServoCount] = {13, 12, 14, 27, 26, 25};
constexpr int kServoMin[kServoCount] = {0, 0, 0, 20, 0, 50};
constexpr int kServoMax[kServoCount] = {180, 180, 180, 160, 180, 125};
constexpr int kServoHome[kServoCount] = {18, 90, 180, 96, 0, 90};

struct SequenceStep {
    int positions[kServoCount];
};

const char kHtmlHeader[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 Roboterarm</title>
<style>
body {
    font-family: Arial, sans-serif;
    text-align: center;
    background: #f4f4f4;
    margin: 0;
    padding: 20px 12px 40px;
}
h1 {
    margin-bottom: 20px;
}
.card {
    background: white;
    padding: 15px;
    margin: 10px auto;
    border-radius: 10px;
    width: min(90%, 500px);
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
.record {
    background-color: #6f42c1;
    width: 200px;
}
.clear {
    background-color: #fd7e14;
    width: 200px;
}
.home {
    background-color: #ff8c00;
    width: 200px;
}
.status {
    font-size: 18px;
    margin-top: 12px;
}
</style>
</head>
<body>
<h1>ESP32 Roboterarm</h1>
<div class="card">
    <button class="home" onclick="moveHome()">Grundstellung</button><br><br>
    <button class="record" onclick="recordPosition()">Position speichern</button><br><br>
    <button class="seq" onclick="playSequence()">Sequenz abspielen</button><br><br>
    <button class="clear" onclick="clearSequence()">Sequenz löschen</button><br><br>
    <button class="stop" onclick="stopSequence()">STOP</button>
    <div class="status" id="sequenceStatus">Gespeicherte Positionen: 0</div>
</div>
)rawliteral";

const char kHtmlFooter[] PROGMEM = R"rawliteral(
<script>
const SERVO_COUNT = 6;

function startMove(id, dir) {
    fetch(`/startMove?servo=${id}&dir=${dir}`);
}

function stopMove(id) {
    fetch(`/stopMove?servo=${id}`);
}

function stopAll() {
    for (let i = 0; i < SERVO_COUNT; i++) {
        fetch(`/stopMove?servo=${i}`);
    }
}

function handleGlobalStop(event) {
    if (event.target.closest(".minus, .plus")) {
        stopAll();
    }
}

document.addEventListener("mouseup", handleGlobalStop);
document.addEventListener("touchend", handleGlobalStop);
document.addEventListener("touchcancel", handleGlobalStop);

function recordPosition() {
    fetch("/record")
        .then((res) => res.text())
        .then(() => refreshSequenceStatus())
        .catch((err) => console.error("Fehler bei /record:", err));
}

function playSequence() {
    fetch("/playSequence")
        .then((res) => {
            if (!res.ok) {
                return res.text().then((text) => Promise.reject(new Error(text)));
            }
            return res.text();
        })
        .then(() => refreshSequenceStatus())
        .catch((err) => console.error("Fehler bei /playSequence:", err));
}

function clearSequence() {
    fetch("/clearSequence")
        .then((res) => res.text())
        .then(() => refreshSequenceStatus())
        .catch((err) => console.error("Fehler bei /clearSequence:", err));
}

function moveHome() {
    fetch("/home");
}

function stopSequence() {
    fetch("/stop")
        .then((res) => res.text())
        .then(() => refreshSequenceStatus())
        .catch((err) => console.error("Fehler bei /stop:", err));
}

function refreshSequenceStatus() {
    fetch("/sequenceStatus")
        .then((res) => res.json())
        .then((data) => {
            document.getElementById("sequenceStatus").innerText =
                `Gespeicherte Positionen: ${data.count} | Wiedergabe: ${data.playing ? "aktiv" : "bereit"}`;
        })
        .catch((err) => console.error("Fehler bei /sequenceStatus:", err));
}

function refreshPositions() {
    fetch("/positions")
        .then((res) => res.json())
        .then((data) => {
            for (let i = 0; i < data.length; i++) {
                document.getElementById(`val${i}`).innerText =
                    `${data[i]} ${String.fromCharCode(176)}`;
            }
        })
        .catch((err) => console.error("Fehler bei /positions:", err));
}

setInterval(refreshPositions, 300);
refreshPositions();
setInterval(refreshSequenceStatus, 500);
refreshSequenceStatus();
</script>
</body>
</html>
)rawliteral";

WebServer server(kHttpPort);
Servo servos[kServoCount];
int servoPositions[kServoCount] = {90, 90, 90, 90, 90, 90};
int servoTargets[kServoCount] = {90, 90, 90, 90, 90, 90};
int servoMoveDirection[kServoCount] = {0};
std::vector<SequenceStep> recordedSequence;
bool sequencePlaying = false;
size_t sequenceIndex = 0;
unsigned long lastServoUpdate = 0;
unsigned long lastHoldUpdate = 0;
unsigned long sequenceStepReachedAt = 0;

bool parseServoId(uint8_t &servoId) {
    if (!server.hasArg("servo")) {
        server.send(400, "text/plain", "missing servo parameter");
        return false;
    }

    const String value = server.arg("servo");
    const long parsedValue = value.toInt();
    if (value.length() == 0 || String(parsedValue) != value || parsedValue < 0 || parsedValue >= kServoCount) {
        server.send(400, "text/plain", "invalid servo parameter");
        return false;
    }

    servoId = static_cast<uint8_t>(parsedValue);
    return true;
}

bool parseDirection(int &direction) {
    if (!server.hasArg("dir")) {
        server.send(400, "text/plain", "missing dir parameter");
        return false;
    }

    const String value = server.arg("dir");
    const long parsedValue = value.toInt();
    if (value.length() == 0 || String(parsedValue) != value || (parsedValue != -1 && parsedValue != 1)) {
        server.send(400, "text/plain", "invalid dir parameter");
        return false;
    }

    direction = static_cast<int>(parsedValue);
    return true;
}

void setServoTarget(uint8_t id, int angle) {
    servoTargets[id] = constrain(angle, kServoMin[id], kServoMax[id]);
}

void recordCurrentPosition() {
    SequenceStep step = {};
    for (uint8_t i = 0; i < kServoCount; i++) {
        step.positions[i] = servoPositions[i];
    }
    recordedSequence.push_back(step);
}

void applySequenceStep(size_t index) {
    if (index >= recordedSequence.size()) {
        return;
    }

    for (uint8_t i = 0; i < kServoCount; i++) {
        setServoTarget(i, recordedSequence[index].positions[i]);
        servoMoveDirection[i] = 0;
    }
    sequenceStepReachedAt = 0;
}

bool allServosAtTarget() {
    for (uint8_t i = 0; i < kServoCount; i++) {
        if (servoPositions[i] != servoTargets[i]) {
            return false;
        }
    }
    return true;
}

void moveToHomePosition() {
    for (uint8_t i = 0; i < kServoCount; i++) {
        setServoTarget(i, kServoHome[i]);
        servoMoveDirection[i] = 0;
    }
}

void sendServoCard(uint8_t servoId) {
    char row[640];
    snprintf(
        row,
        sizeof(row),
        "<div class='card'><div class='servo-row'><h3>Servo %u</h3>"
        "<div class='value' id='val%u'>%d&deg;</div>"
        "<div class='button-group'>"
        "<button class='minus' onmousedown='startMove(%u,-1)' onmouseup='stopMove(%u)' "
        "onmouseleave='stopMove(%u)' ontouchstart='startMove(%u,-1)' ontouchend='stopMove(%u)'>-</button>"
        "<button class='plus' onmousedown='startMove(%u,1)' onmouseup='stopMove(%u)' "
        "onmouseleave='stopMove(%u)' ontouchstart='startMove(%u,1)' ontouchend='stopMove(%u)'>+</button>"
        "</div></div></div>",
        servoId,
        servoId,
        servoPositions[servoId],
        servoId,
        servoId,
        servoId,
        servoId,
        servoId,
        servoId,
        servoId,
        servoId,
        servoId,
        servoId);
    server.sendContent(row);
}

void stopSequenceInternal() {
    sequencePlaying = false;
    sequenceIndex = 0;
    sequenceStepReachedAt = 0;
    for (uint8_t i = 0; i < kServoCount; i++) {
        servoTargets[i] = servoPositions[i];
        servoMoveDirection[i] = 0;
    }
}

void startRecordedSequenceInternal() {
    stopSequenceInternal();
    if (recordedSequence.empty()) {
        return;
    }

    sequencePlaying = true;
    sequenceIndex = 0;
    applySequenceStep(sequenceIndex);
}

void updateHeldButtons() {
    if (millis() - lastHoldUpdate < kHoldStepDelayMs) {
        return;
    }

    lastHoldUpdate = millis();
    for (uint8_t i = 0; i < kServoCount; i++) {
        if (servoMoveDirection[i] != 0) {
            setServoTarget(i, servoTargets[i] + servoMoveDirection[i] * kHoldStepSize);
        }
    }
}

void updateServosSmoothly() {
    if (millis() - lastServoUpdate < kServoStepDelayMs) {
        return;
    }

    lastServoUpdate = millis();
    for (uint8_t i = 0; i < kServoCount; i++) {
        if (servoPositions[i] < servoTargets[i]) {
            servoPositions[i] = min(servoPositions[i] + kServoStepSize, servoTargets[i]);
        } else if (servoPositions[i] > servoTargets[i]) {
            servoPositions[i] = max(servoPositions[i] - kServoStepSize, servoTargets[i]);
        }
        servos[i].write(servoPositions[i]);
    }
}

void updateSequence() {
    if (!sequencePlaying) {
        return;
    }

    if (!allServosAtTarget()) {
        sequenceStepReachedAt = 0;
        return;
    }

    if (sequenceStepReachedAt == 0) {
        sequenceStepReachedAt = millis();
        return;
    }

    if (millis() - sequenceStepReachedAt < kSequenceHoldMs) {
        return;
    }

    sequenceIndex++;
    if (sequenceIndex >= recordedSequence.size()) {
        sequencePlaying = false;
        sequenceIndex = 0;
        sequenceStepReachedAt = 0;
        return;
    }

    applySequenceStep(sequenceIndex);
}

void handleRoot() {
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html; charset=UTF-8", "");
    server.sendContent_P(kHtmlHeader);
    for (uint8_t i = 0; i < kServoCount; i++) {
        sendServoCard(i);
    }
    server.sendContent_P(kHtmlFooter);
}

void handleStartMove() {
    uint8_t servoId = 0;
    int direction = 0;
    if (!parseServoId(servoId) || !parseDirection(direction)) {
        return;
    }

    sequencePlaying = false;
    servoMoveDirection[servoId] = direction;
    server.send(200, "text/plain", "move start");
}

void handleStopMove() {
    uint8_t servoId = 0;
    if (!parseServoId(servoId)) {
        return;
    }

    servoMoveDirection[servoId] = 0;
    servoTargets[servoId] = servoPositions[servoId];
    server.send(200, "text/plain", "move stop");
}

void handleRecord() {
    stopSequenceInternal();
    recordCurrentPosition();
    server.send(200, "text/plain", "recorded");
}

void handlePlaySequence() {
    if (recordedSequence.empty()) {
        server.send(400, "text/plain", "no recorded sequence");
        return;
    }

    startRecordedSequenceInternal();
    server.send(200, "text/plain", "play sequence");
}

void handleHome() {
    stopSequenceInternal();
    moveToHomePosition();
    Serial.println("Grundstellung angefahren");
    server.send(200, "text/plain", "home");
}

void handleStop() {
    stopSequenceInternal();
    server.send(200, "text/plain", "stop");
}

void handleClearSequence() {
    stopSequenceInternal();
    recordedSequence.clear();
    server.send(200, "text/plain", "clear");
}

void handleSequenceStatus() {
    char json[96];
    snprintf(
        json,
        sizeof(json),
        "{\"count\":%u,\"playing\":%s}",
        static_cast<unsigned>(recordedSequence.size()),
        sequencePlaying ? "true" : "false");
    server.send(200, "application/json", json);
}

void handlePositions() {
    char json[32];
    snprintf(
        json,
        sizeof(json),
        "[%d,%d,%d,%d,%d,%d]",
        servoPositions[0],
        servoPositions[1],
        servoPositions[2],
        servoPositions[3],
        servoPositions[4],
        servoPositions[5]);
    server.send(200, "application/json", json);
}

}  // namespace

void setup() {
    Serial.begin(kSerialBaudRate);
    WiFi.softAP(kApName, kApPassword);

    for (uint8_t i = 0; i < kServoCount; i++) {
        servoPositions[i] = constrain(kServoHome[i], kServoMin[i], kServoMax[i]);
        servoTargets[i] = servoPositions[i];
        servos[i].attach(kServoPins[i], kServoPulseMinUs, kServoPulseMaxUs);
        servos[i].write(servoPositions[i]);
    }

    server.on("/", handleRoot);
    server.on("/startMove", handleStartMove);
    server.on("/stopMove", handleStopMove);
    server.on("/record", handleRecord);
    server.on("/playSequence", handlePlaySequence);
    server.on("/clearSequence", handleClearSequence);
    server.on("/sequenceStatus", handleSequenceStatus);
    server.on("/home", handleHome);
    server.on("/stop", handleStop);
    server.on("/positions", handlePositions);
    server.begin();
}

void loop() {
    server.handleClient();
    updateHeldButtons();
    updateServosSmoothly();
    updateSequence();
}
