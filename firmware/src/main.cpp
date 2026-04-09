#include <Arduino.h>
#include <ESP32Servo.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <utility>
#include <vector>

namespace {

constexpr uint8_t kServoCount = 6;
constexpr uint16_t kHttpPort = 80;
constexpr uint32_t kSequenceHoldMs = 1000;
constexpr uint16_t kServoPulseMinUs = 500;
constexpr uint16_t kServoPulseMaxUs = 2400;
constexpr uint32_t kSerialBaudRate = 115200;
constexpr uint8_t kServoSpeedMin = 1;
constexpr uint8_t kServoSpeedMax = 10;
constexpr uint8_t kServoSpeedDefault = 5;
constexpr uint32_t kServoMoveIntervalSlowMs = 120;
constexpr uint32_t kServoMoveIntervalFastMs = 12;
constexpr size_t kMaxRecordedSteps = 128;
constexpr uint16_t kSequenceFormatVersion = 1;
constexpr uint32_t kSequenceMagic = 0x52524D31;  // "RRM1"

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

WebServer server(kHttpPort);
Preferences preferences;
Servo servos[kServoCount];
int servoPositions[kServoCount] = {90, 90, 90, 90, 90, 90};
int servoTargets[kServoCount] = {90, 90, 90, 90, 90, 90};
int servoMoveDirection[kServoCount] = {0};
uint8_t servoSpeeds[kServoCount] = {
    kServoSpeedDefault,
    kServoSpeedDefault,
    kServoSpeedDefault,
    kServoSpeedDefault,
    kServoSpeedDefault,
    kServoSpeedDefault};
std::vector<SequenceStep> recordedSequence;
bool sequencePlaying = false;
size_t sequenceIndex = 0;
unsigned long lastServoMoveAt[kServoCount] = {0};
unsigned long sequenceStepReachedAt = 0;

struct StoredSequenceHeader {
    uint32_t magic;
    uint16_t version;
    uint16_t count;
};

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

bool parseSpeedValue(uint8_t &speed) {
    if (!server.hasArg("value")) {
        server.send(400, "text/plain", "missing value parameter");
        return false;
    }

    const String value = server.arg("value");
    const long parsedValue = value.toInt();
    if (value.length() == 0 || String(parsedValue) != value || parsedValue < kServoSpeedMin ||
        parsedValue > kServoSpeedMax) {
        server.send(400, "text/plain", "invalid value parameter");
        return false;
    }

    speed = static_cast<uint8_t>(parsedValue);
    return true;
}

uint32_t getServoMoveIntervalMs(uint8_t speed) {
    const long mappedInterval = map(
        speed,
        kServoSpeedMin,
        kServoSpeedMax,
        static_cast<long>(kServoMoveIntervalSlowMs),
        static_cast<long>(kServoMoveIntervalFastMs));
    return static_cast<uint32_t>(mappedInterval);
}

void setServoTarget(uint8_t id, int angle) {
    servoTargets[id] = constrain(angle, kServoMin[id], kServoMax[id]);
}

bool openPreferences(bool readOnly) {
    return preferences.begin("robotarm", readOnly);
}

bool saveSequenceToStorage() {
    if (!openPreferences(false)) {
        return false;
    }

    StoredSequenceHeader header = {
        kSequenceMagic,
        kSequenceFormatVersion,
        static_cast<uint16_t>(recordedSequence.size())};

    const size_t writtenHeader = preferences.putBytes("seqHdr", &header, sizeof(header));
    if (writtenHeader != sizeof(header)) {
        preferences.end();
        return false;
    }

    bool ok = true;
    if (recordedSequence.empty()) {
        preferences.remove("seqData");
    } else {
        const size_t totalBytes = recordedSequence.size() * sizeof(SequenceStep);
        const size_t writtenData = preferences.putBytes("seqData", recordedSequence.data(), totalBytes);
        ok = (writtenData == totalBytes);
    }

    preferences.end();
    return ok;
}

bool loadSequenceFromStorage() {
    if (!openPreferences(true)) {
        return false;
    }

    StoredSequenceHeader header = {};
    const size_t readHeader = preferences.getBytes("seqHdr", &header, sizeof(header));
    if (readHeader != sizeof(header) || header.magic != kSequenceMagic ||
        header.version != kSequenceFormatVersion) {
        preferences.end();
        return false;
    }

    if (header.count > kMaxRecordedSteps) {
        preferences.end();
        return false;
    }

    std::vector<SequenceStep> loaded(header.count);
    if (header.count > 0) {
        const size_t totalBytes = header.count * sizeof(SequenceStep);
        const size_t readData = preferences.getBytes("seqData", loaded.data(), totalBytes);
        if (readData != totalBytes) {
            preferences.end();
            return false;
        }
    }

    preferences.end();

    for (SequenceStep &step : loaded) {
        for (uint8_t i = 0; i < kServoCount; i++) {
            step.positions[i] = constrain(step.positions[i], kServoMin[i], kServoMax[i]);
        }
    }

    recordedSequence = std::move(loaded);
    return true;
}

bool recordCurrentPosition() {
    if (recordedSequence.size() >= kMaxRecordedSteps) {
        return false;
    }

    SequenceStep step = {};
    for (uint8_t i = 0; i < kServoCount; i++) {
        step.positions[i] = servoPositions[i];
    }
    recordedSequence.push_back(step);
    return true;
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

void updateServosSmoothly() {
    const unsigned long now = millis();
    for (uint8_t i = 0; i < kServoCount; i++) {
        if (now - lastServoMoveAt[i] < getServoMoveIntervalMs(servoSpeeds[i])) {
            continue;
        }

        int nextPosition = servoPositions[i];
        if (servoMoveDirection[i] != 0) {
            nextPosition = constrain(servoPositions[i] + servoMoveDirection[i], kServoMin[i], kServoMax[i]);
            servoTargets[i] = nextPosition;
        } else if (servoPositions[i] < servoTargets[i]) {
            nextPosition = servoPositions[i] + 1;
        } else if (servoPositions[i] > servoTargets[i]) {
            nextPosition = servoPositions[i] - 1;
        }

        if (nextPosition == servoPositions[i]) {
            continue;
        }

        servoPositions[i] = nextPosition;
        servos[i].write(servoPositions[i]);
        lastServoMoveAt[i] = now;
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
    File file = LittleFS.open("/index.html");
    if (!file) {
        server.send(404, "text/plain", "File not found");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
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

void handleSetSpeed() {
    uint8_t servoId = 0;
    uint8_t speed = 0;
    if (!parseServoId(servoId) || !parseSpeedValue(speed)) {
        return;
    }

    servoSpeeds[servoId] = speed;
    server.send(200, "text/plain", "speed updated");
}

void handleRecord() {
    stopSequenceInternal();
    if (!recordCurrentPosition()) {
        server.send(400, "text/plain", "sequence limit reached");
        return;
    }
    if (!saveSequenceToStorage()) {
        server.send(500, "text/plain", "persist failed");
        return;
    }
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
    if (!saveSequenceToStorage()) {
        server.send(500, "text/plain", "persist failed");
        return;
    }
    server.send(200, "text/plain", "clear");
}

void handleOverwriteSequence() {
    stopSequenceInternal();
    recordedSequence.clear();
    if (!recordCurrentPosition()) {
        server.send(400, "text/plain", "sequence limit reached");
        return;
    }
    if (!saveSequenceToStorage()) {
        server.send(500, "text/plain", "persist failed");
        return;
    }
    server.send(200, "text/plain", "overwritten");
}

void handleSequenceStatus() {
    char json[96];
    const int currentStep = sequencePlaying ? static_cast<int>(sequenceIndex) : -1;
    snprintf(
        json,
        sizeof(json),
        "{\"count\":%u,\"playing\":%s,\"currentStep\":%d}",
        static_cast<unsigned>(recordedSequence.size()),
        sequencePlaying ? "true" : "false",
        currentStep);
    server.send(200, "application/json", json);
}

void handleSequenceData() {
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "application/json", "");

    char header[96];
    const int currentStep = sequencePlaying ? static_cast<int>(sequenceIndex) : -1;
    snprintf(
        header,
        sizeof(header),
        "{\"count\":%u,\"playing\":%s,\"currentStep\":%d,\"steps\":[",
        static_cast<unsigned>(recordedSequence.size()),
        sequencePlaying ? "true" : "false",
        currentStep);
    server.sendContent(header);

    for (size_t stepIndex = 0; stepIndex < recordedSequence.size(); stepIndex++) {
        if (stepIndex > 0) {
            server.sendContent(",");
        }

        char row[64];
        snprintf(
            row,
            sizeof(row),
            "[%d,%d,%d,%d,%d,%d]",
            recordedSequence[stepIndex].positions[0],
            recordedSequence[stepIndex].positions[1],
            recordedSequence[stepIndex].positions[2],
            recordedSequence[stepIndex].positions[3],
            recordedSequence[stepIndex].positions[4],
            recordedSequence[stepIndex].positions[5]);
        server.sendContent(row);
    }

    server.sendContent("]}");
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

void handleSpeeds() {
    char json[32];
    snprintf(
        json,
        sizeof(json),
        "[%u,%u,%u,%u,%u,%u]",
        servoSpeeds[0],
        servoSpeeds[1],
        servoSpeeds[2],
        servoSpeeds[3],
        servoSpeeds[4],
        servoSpeeds[5]);
    server.send(200, "application/json", json);
}

}  // namespace

void setup() {
    Serial.begin(kSerialBaudRate);

    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
        return;
    }

    WiFi.softAP(kApName, kApPassword);

    for (uint8_t i = 0; i < kServoCount; i++) {
        servoPositions[i] = constrain(kServoHome[i], kServoMin[i], kServoMax[i]);
        servoTargets[i] = servoPositions[i];
        servos[i].attach(kServoPins[i], kServoPulseMinUs, kServoPulseMaxUs);
        servos[i].write(servoPositions[i]);
    }

    if (loadSequenceFromStorage()) {
        Serial.printf("Sequenz aus Flash geladen (%u Schritte)\n", static_cast<unsigned>(recordedSequence.size()));
    } else {
        Serial.println("Keine gueltige gespeicherte Sequenz gefunden");
    }

    server.on("/", handleRoot);
    server.on("/startMove", handleStartMove);
    server.on("/stopMove", handleStopMove);
    server.on("/setSpeed", handleSetSpeed);
    server.on("/record", handleRecord);
    server.on("/overwriteSequence", handleOverwriteSequence);
    server.on("/playSequence", handlePlaySequence);
    server.on("/clearSequence", handleClearSequence);
    server.on("/sequenceStatus", handleSequenceStatus);
    server.on("/sequenceData", handleSequenceData);
    server.on("/home", handleHome);
    server.on("/stop", handleStop);
    server.on("/positions", handlePositions);
    server.on("/speeds", handleSpeeds);
    server.serveStatic("/style.css", LittleFS, "/style.css");
    server.serveStatic("/script.js", LittleFS, "/script.js");
    server.begin();
}

void loop() {
    server.handleClient();
    updateServosSmoothly();
    updateSequence();
}
