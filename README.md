# ESP32 Web Robot Arm

Ein ESP32-basiertes Projekt zur Steuerung eines 6-Achs-Roboterarms ueber eine Weboberflaeche im Browser.

Der ESP32 startet einen eigenen WLAN-Access-Point und stellt die Bedienoberflaeche direkt selbst bereit. Die Steuerung erfolgt ohne zusaetzlichen Server oder Cloud-Dienst ueber HTML, CSS, JavaScript und eine kleine REST-aehnliche API auf dem Geraet.

## Navigation

- [Dokumentationsuebersicht](docs/README.md)
- [Architektur](docs/architecture.md)
- [API-Dokumentation](docs/api.md)
- [Hardware](docs/hardware.md)
- [Deployment](docs/deployment.md)
- [Troubleshooting](docs/troubleshooting.md)
- [Projektplan](docs/project-plan.md)

## Funktionen

- Steuerung von 6 Servos ueber eine Weboberflaeche
- Bewegung pro Servo ueber Plus-/Minus-Tasten
- Individuelle Geschwindigkeitssteuerung pro Servo
- Anfahren einer Grundstellung
- Speichern einzelner Positionen als Sequenz
- Ueberschreiben, Abspielen und Loeschen von Sequenzen
- Persistente Speicherung der Sequenz im Flash
- Auslieferung der Webdateien ueber LittleFS

## Projektaufbau

- `firmware/` Firmware-Projekt auf Basis von PlatformIO
- `firmware/src/main.cpp` Hauptlogik fuer WLAN, Webserver, Servo-Steuerung und Sequenzverwaltung
- `firmware/data/` Weboberflaeche (`index.html`, `style.css`, `script.js`) fuer LittleFS
- `docs/` technische Dokumentation
- `hardware/` Hardware-Notizen und Schaltungsinformationen

## Verwendete Technologien

- ESP32
- Arduino Framework
- PlatformIO
- C++
- HTML / CSS / JavaScript
- LittleFS
- `WebServer`, `Preferences`, `ESP32Servo`

## Hardware

Geplant bzw. verwendet:

- ESP32 Dev Board
- 6 Servomotoren
- Externe 5V-Stromversorgung fuer die Servos
- Roboterarm-Gestell, z. B. 3D-gedruckt

Optional:

- PCA9685 fuer erweiterte Servo-Ansteuerung

## Voraussetzungen

Fuer Build und Upload wird benoetigt:

- [PlatformIO](https://platformio.org/)
- Ein per USB angeschlossener ESP32
- Python bzw. PlatformIO-CLI in einer funktionierenden lokalen Umgebung

## Konfiguration

Die wichtigsten Projektparameter stehen in `firmware/platformio.ini`.

Standardmaessig startet der ESP32 mit:

- WLAN-Name: `ESP32-Roboterarm`
- Passwort: `robotarm123`

Diese Werte koennen ueber `build_flags` angepasst werden.

## Build

```powershell
cd firmware
pio run
```

## Firmware und Webdateien auf den ESP32 laden

Da die Weboberflaeche in `firmware/data/` liegt und ueber LittleFS ausgeliefert wird, muessen Firmware und Dateisystem getrennt uebertragen werden.

### 1. Firmware flashen

```powershell
cd firmware
pio run -t upload
```

### 2. LittleFS-Dateien hochladen

```powershell
cd firmware
pio run -t uploadfs
```

Empfehlung: Nach Aenderungen an `index.html`, `style.css` oder `script.js` immer `uploadfs` ausfuehren.

## Nutzung

1. ESP32 per USB mit Strom versorgen oder normal starten.
2. Mit einem Smartphone, Tablet oder Laptop in das WLAN `ESP32-Roboterarm` wechseln.
3. Im Browser `http://192.168.4.1/` aufrufen.
4. Servos manuell bewegen, Geschwindigkeiten anpassen und Sequenzen speichern oder abspielen.

## Bedienkonzept

- Plus-/Minus-Tasten bewegen den jeweiligen Servo schrittweise
- Ein Slider pro Servo stellt die Bewegungsgeschwindigkeit ein
- `Grundstellung` faehrt alle Servos in ihre Home-Position
- `Position speichern` haengt die aktuelle Stellung an die Sequenz an
- `Sequenz ueberschreiben` ersetzt die bestehende Sequenz durch die aktuelle Position
- `Sequenz abspielen` faehrt alle gespeicherten Schritte nacheinander ab
- `Sequenz loeschen` entfernt die gespeicherten Schritte
- `STOP` beendet laufende Bewegungen bzw. Sequenzwiedergabe

## Persistenz

Gespeicherte Sequenzen werden ueber `Preferences` im Flash des ESP32 abgelegt und nach einem Neustart automatisch wieder geladen.

## Bekannte Hinweise

- Die Servos sollten nicht direkt ueber den 5V-Pin eines USB-Anschlusses versorgt werden.
- Fuer stabile Bewegungen ist eine separate Stromversorgung fuer die Servos empfehlenswert.
- Nach Aenderungen an der Weboberflaeche reicht ein normales Firmware-Flashen nicht aus, solange `uploadfs` nicht ebenfalls ausgefuehrt wurde.

## Entwicklungsziel

Das Projekt dient als Lern- und Praxisprojekt fuer:

- Embedded-Programmierung mit ESP32
- Ansteuerung mehrerer Servos
- Webserver auf Mikrocontrollern
- Zustandsverwaltung und persistente Speicherung
- Trennung von Firmware und Weboberflaeche

## Weiterfuehrende Dokumentation

Fuer technische Details und Projektpflege:

- [Dokumentationsuebersicht](docs/README.md)
- [Architektur](docs/architecture.md)
- [API-Dokumentation](docs/api.md)
- [Projektplan](docs/project-plan.md)

## Lizenz

Dieses Projekt steht unter der MIT-Lizenz. Details stehen in `LICENSE`.

