# Hardware

Zur Dokumentationsuebersicht: [docs/README.md](README.md)

## Ziel

Diese Datei beschreibt die aktuell verwendete bzw. vorgesehene Hardware fuer den ESP32-Web-Roboterarm.

## Kernkomponenten

- ESP32 Dev Board
- 6 Servomotoren
- externe 5V-Stromversorgung fuer die Servos
- Roboterarm-Gestell
- USB-Verbindung fuer Flash und serielle Diagnose

## Servo-Anschluesse

Der aktuelle Firmware-Stand verwendet folgende GPIO-Pins:

- Servo 0: GPIO 13
- Servo 1: GPIO 12
- Servo 2: GPIO 14
- Servo 3: GPIO 27
- Servo 4: GPIO 26
- Servo 5: GPIO 25

Wichtig:

- Die reale Verkabelung muss zu diesen Pins passen.
- Wenn Servos auf andere Pins gelegt werden, muessen die Werte in `firmware/src/main.cpp` angepasst werden.

## Logische Servo-Grenzen

Die Firmware arbeitet aktuell mit festen Grenzwerten pro Servo:

- Minimalposition
- Maximalposition
- Home-Position

Diese Grenzen schuetzen die Mechanik vor ungueltigen oder unguenstigen Bewegungen. Sie sollten bei Aenderungen am Aufbau oder an der Mechanik neu geprueft werden.

## Stromversorgung

### ESP32

Der ESP32 kann waehrend Entwicklung und Flashen ueber USB versorgt werden.

### Servos

Die Servos sollten ueber eine separate, ausreichend dimensionierte 5V-Stromversorgung betrieben werden.

Empfehlung:

- gemeinsame Masse zwischen ESP32 und externer Servo-Versorgung
- Servo-Strom nicht direkt ueber den USB-Port oder den 5V-Pin des Boards ziehen, wenn mehrere Servos gleichzeitig arbeiten

## Sicherheitshinweise

- Mechanische Endanschlaege vor dem ersten Test pruefen
- Servos zuerst mit vorsichtigen Grenzwerten betreiben
- bei Blockieren oder starkem Zittern Spannungsversorgung und Grenzwerte pruefen
- Arm bei ersten Tests nicht unbeaufsichtigt laufen lassen

## Typische Risiken

- Spannungseinbruch bei gleichzeitiger Servo-Bewegung
- unruhige Bewegung durch zu schwache Stromversorgung
- falsche Servo-Richtung oder unpassende Nullstellung
- Kollisionen durch unguenstige Min-/Max-Werte

## Empfohlene spaetere Ergaenzungen

- Schaltplan oder Verdrahtungsbild
- Foto des realen Aufbaus
- genaue Angaben zu Servotypen
- Angaben zu Stromaufnahme und Netzteil
- Kalibrierhinweise pro Servo

Weiter zu:

- [Architektur](architecture.md)
- [Deployment](deployment.md)
- [Troubleshooting](troubleshooting.md)
