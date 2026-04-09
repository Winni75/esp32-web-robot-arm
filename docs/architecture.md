# Architektur

Zur Dokumentationsuebersicht: [docs/README.md](README.md)

## Ziel des Systems

Das Projekt steuert einen 6-Achs-Roboterarm direkt ueber einen ESP32. Der Mikrocontroller uebernimmt gleichzeitig:

- WLAN-Access-Point
- HTTP-Webserver
- Auslieferung der Weboberflaeche
- Servo-Ansteuerung
- Sequenzverwaltung
- persistente Speicherung der Sequenz

Die Architektur ist bewusst kompakt gehalten: Firmware, Webserver und UI liegen nah beieinander, damit das System ohne externe Infrastruktur betrieben werden kann.

## Hauptkomponenten

### ESP32-Firmware

Die zentrale Logik liegt in `firmware/src/main.cpp`.

Verantwortlichkeiten:

- Initialisierung von LittleFS
- Start des WLAN-Access-Points
- Initialisierung der sechs Servos
- Registrierung aller HTTP-Endpunkte
- zyklische Servo-Bewegung im `loop()`
- Verwaltung und Wiedergabe gespeicherter Sequenzen
- Speicherung der Sequenz ueber `Preferences`

### Weboberflaeche

Die Weboberflaeche liegt in `firmware/data/` und wird ueber LittleFS ausgeliefert.

Dateien:

- `index.html` strukturiert die Bedienoberflaeche
- `style.css` definiert Layout und Darstellung
- `script.js` enthaelt die gesamte Browserlogik fuer Bedienung, Polling und API-Aufrufe

### Persistenz

Gespeicherte Sequenzen werden nicht im Dateisystem, sondern ueber `Preferences` im Flash des ESP32 abgelegt.

Dabei werden gespeichert:

- ein Header mit Magic-Value und Versionsnummer
- die Anzahl der gespeicherten Schritte
- die Servopositionen aller Schritte

## Laufzeitverhalten

### Startphase

Beim Start passiert in dieser Reihenfolge:

1. Serielle Schnittstelle wird initialisiert
2. LittleFS wird gemountet
3. der ESP32 startet den Access-Point
4. alle Servos werden auf ihre Home-Position gesetzt
5. eine zuvor gespeicherte Sequenz wird aus `Preferences` geladen
6. der HTTP-Server und die statischen Dateien werden registriert

### Regelbetrieb

Im `loop()` laufen drei zentrale Aufgaben:

1. `server.handleClient()` verarbeitet eingehende HTTP-Anfragen
2. `updateServosSmoothly()` bewegt Servos schrittweise in Richtung Zielposition
3. `updateSequence()` verwaltet die Wiedergabe gespeicherter Sequenzen

## Bewegungsmodell

Die Firmware unterscheidet zwischen:

- aktueller Position `servoPositions`
- Zielposition `servoTargets`
- manueller Bewegungsrichtung `servoMoveDirection`
- Geschwindigkeit `servoSpeeds`

Manuelle Bewegung:

- Beim Gedrueckthalten eines Buttons wird fuer einen Servo `-1` oder `1` als Richtung gesetzt.
- Der Servo bewegt sich daraufhin schrittweise innerhalb seiner Min-/Max-Grenzen.
- Beim Loslassen wird die Bewegung gestoppt und die aktuelle Position als Ziel uebernommen.

Automatische Bewegung:

- Bei Grundstellung oder Sequenzwiedergabe werden Zielpositionen gesetzt.
- Die eigentliche Bewegung erfolgt weiterhin schrittweise im Hintergrund.

## Geschwindigkeitsmodell

Jeder Servo besitzt einen eigenen Geschwindigkeitswert von `1` bis `10`.

- `1` bedeutet langsame Bewegung
- `10` bedeutet schnelle Bewegung

Intern wird der Wert auf ein Zeitintervall zwischen einzelnen Servo-Schritten abgebildet.

## Sequenzmodell

Eine Sequenz besteht aus mehreren Schritten. Jeder Schritt enthaelt genau sechs Zielwerte, einen pro Servo.

Eigenschaften:

- maximal `128` gespeicherte Schritte
- zwischen zwei Schritten gibt es eine Haltezeit von `1000 ms`
- die Sequenz wird nur abgespielt, wenn mindestens ein Schritt vorhanden ist

Wichtige interne Funktionen:

- `recordCurrentPosition()`
- `applySequenceStep()`
- `startRecordedSequenceInternal()`
- `stopSequenceInternal()`
- `updateSequence()`

## Netzwerkmodell

Der ESP32 arbeitet als eigener Access-Point.

Standardwerte:

- SSID: `ESP32-Roboterarm`
- Passwort: `robotarm123`
- Weboberflaeche: `http://192.168.4.1/`

Die Werte koennen in `firmware/platformio.ini` ueber Build-Flags angepasst werden.

## Speicher- und Dateimodell

### LittleFS

LittleFS wird fuer die statischen Webdateien verwendet:

- `/index.html`
- `/style.css`
- `/script.js`

Wichtig:

- Nach Aenderungen an den Dateien in `firmware/data/` muss `pio run -t uploadfs` ausgefuehrt werden.

### Preferences

`Preferences` speichert die Sequenz dauerhaft unter dem Namespace `robotarm`.

Verwendete Schluessel:

- `seqHdr`
- `seqData`

## Servo-Konfiguration

Der aktuelle Code definiert folgende Servo-Pins:

- Servo 0: GPIO 13
- Servo 1: GPIO 12
- Servo 2: GPIO 14
- Servo 3: GPIO 27
- Servo 4: GPIO 26
- Servo 5: GPIO 25

Zusatzlich besitzt jeder Servo:

- eine Minimalposition
- eine Maximalposition
- eine Home-Position

Diese Werte sind fest in `main.cpp` hinterlegt und sollten bei Aenderungen an der Mechanik entsprechend angepasst werden.

## Aktuelle Architekturgrenzen

Die aktuelle Architektur ist fuer ein kleines Embedded-Projekt passend, hat aber bewusst einfache Grenzen:

- Backend-Logik liegt noch komplett in einer Datei
- keine Trennung in Module fuer Netzwerk, Servos und Persistenz
- keine Authentifizierung fuer die Weboberflaeche
- kein Konfigurationsmenue fuer Kalibrierung im Browser
- kein formales Fehler- oder Ereignislogging ausser serieller Ausgabe

## Sinnvolle naechste Ausbaustufen

- Aufteilung von `main.cpp` in mehrere Module
- zentrale Konfigurationsstruktur fuer Servo-Parameter
- API-Dokumentation weiter ausbauen
- zusaetzliche Diagnose-Endpunkte
- Import/Export von Sequenzen
- Bearbeiten einzelner Sequenzschritte in der Weboberflaeche

Weiter zu:

- [API-Dokumentation](api.md)
- [Hardware](hardware.md)
- [Projektplan](project-plan.md)
