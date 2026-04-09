# Deployment

Zur Dokumentationsuebersicht: [docs/README.md](README.md)

## Ziel

Diese Datei beschreibt, wie die Firmware und die Weboberflaeche auf den ESP32 uebertragen werden.

## Voraussetzungen

- PlatformIO ist installiert
- der ESP32 ist per USB verbunden
- das Projekt wurde lokal geoeffnet oder geklont
- die Verbindung zum richtigen COM-Port funktioniert

## Wichtige Dateien

- Firmware-Code: `firmware/src/main.cpp`
- Weboberflaeche: `firmware/data/index.html`, `firmware/data/style.css`, `firmware/data/script.js`
- Projektkonfiguration: `firmware/platformio.ini`

## Build

Vor einem Upload empfiehlt sich ein normaler Build:

```powershell
cd firmware
pio run
```

## Firmware hochladen

```powershell
cd firmware
pio run -t upload
```

Dieser Schritt uebertraegt den kompilierten Firmware-Code auf den ESP32.

## LittleFS-Dateien hochladen

```powershell
cd firmware
pio run -t uploadfs
```

Dieser Schritt erzeugt ein LittleFS-Image aus dem Ordner `firmware/data/` und schreibt es auf das Dateisystem des ESP32.

## Wichtige Regel

Bei Aenderungen an:

- `index.html`
- `style.css`
- `script.js`

reicht ein normaler Firmware-Upload nicht aus. In diesem Fall muss zusaetzlich `pio run -t uploadfs` ausgefuehrt werden.

## Empfohlener Ablauf nach Codeaenderungen

### Nur Firmware geaendert

```powershell
cd firmware
pio run
pio run -t upload
```

### Nur Weboberflaeche geaendert

```powershell
cd firmware
pio run -t uploadfs
```

### Firmware und Weboberflaeche geaendert

```powershell
cd firmware
pio run
pio run -t upload
pio run -t uploadfs
```

## Serielle Pruefung nach dem Upload

Zum Beobachten des Startlogs:

```powershell
cd firmware
pio device monitor --baud 115200
```

Typische serielle Meldungen:

- erfolgreicher Start
- LittleFS-Mount
- Laden gespeicherter Sequenzen

## Funktionstest nach dem Deployment

1. Mit dem WLAN des ESP32 verbinden
2. Browser auf `http://192.168.4.1/` oeffnen
3. Servo-Bewegung testen
4. Grundstellung testen
5. Sequenz speichern und abspielen

## Haeufige Fehlerquellen

- falscher COM-Port
- blockierter serieller Port
- `uploadfs` nach Web-Aenderungen vergessen
- ESP32 zwar gestartet, aber Endgeraet nicht mit dem Access-Point verbunden
- LittleFS-Dateien fehlen oder sind veraltet

Weiter zu:

- [API-Dokumentation](api.md)
- [Hardware](hardware.md)
- [Troubleshooting](troubleshooting.md)
