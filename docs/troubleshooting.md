# Troubleshooting

Zur Dokumentationsuebersicht: [docs/README.md](README.md)

## ESP32 startet, aber die Webseite ist nicht erreichbar

Pruefen:

- ist das Geraet mit dem WLAN `ESP32-Roboterarm` verbunden
- wurde `http://192.168.4.1/` aufgerufen
- startet der ESP32 als Access-Point
- wurde LittleFS erfolgreich hochgeladen

Moegliche Ursache:

- das Testgeraet ist noch im Heimnetz statt im ESP32-WLAN

## Webseite laedt, aber Aenderungen an HTML, CSS oder JS sind nicht sichtbar

Moegliche Ursache:

- die Dateien in `firmware/data/` wurden geaendert, aber `pio run -t uploadfs` wurde nicht ausgefuehrt

Loesung:

```powershell
cd firmware
pio run -t uploadfs
```

## Firmware laesst sich nicht hochladen

Pruefen:

- ist der ESP32 per USB verbunden
- ist der richtige COM-Port verfuegbar
- blockiert ein serieller Monitor den Port

Typisches Problem:

- `COMx` ist bereits in Benutzung

Loesung:

- seriellen Monitor schliessen
- blockierende Prozesse beenden
- Upload erneut starten

## LittleFS-Upload funktioniert nicht

Pruefen:

- ist in `platformio.ini` `board_build.filesystem = littlefs` gesetzt
- liegen die Webdateien im Ordner `firmware/data/`
- wird wirklich `uploadfs` und nicht nur `upload` ausgefuehrt

## ESP32 bootet, aber im Browser fehlt die Oberflaeche

Moegliche Ursachen:

- `/index.html` fehlt im LittleFS
- das Dateisystem wurde nicht hochgeladen
- LittleFS konnte nicht gemountet werden

Pruefung:

- seriellen Monitor starten
- auf Meldungen wie `LittleFS Mount Failed` achten

## Gespeicherte Sequenz ist nach Neustart weg

Pruefen:

- wurde die Position wirklich ueber die Weboberflaeche gespeichert
- treten serielle Fehlermeldungen zur Persistenz auf
- wurde die Sequenz eventuell geloescht oder ueberschrieben

Hinweis:

- die Sequenz wird ueber `Preferences` gespeichert, nicht ueber LittleFS

## Servos bewegen sich nicht oder falsch

Pruefen:

- stimmen die GPIO-Pins mit der realen Verdrahtung ueberein
- ist eine externe Stromversorgung vorhanden
- sind Min-, Max- und Home-Werte passend fuer die Mechanik
- ist die gemeinsame Masse zwischen ESP32 und Servo-Versorgung verbunden

## Servos zittern oder der ESP32 startet neu

Wahrscheinliche Ursache:

- instabile oder zu schwache Stromversorgung

Loesung:

- separate 5V-Stromversorgung fuer die Servos verwenden
- Masseverbindung zwischen Netzteil und ESP32 sicherstellen
- Lastspitzen durch gleichzeitige Bewegungen beachten

## API-Endpunkt liefert Fehler 400

Moegliche Ursache:

- fehlender oder ungueltiger Query-Parameter

Beispiele:

- falsche Servo-ID
- ungueltiger Richtungswert
- Geschwindigkeitswert ausserhalb des erlaubten Bereichs

## API-Endpunkt liefert Fehler 500

Moegliche Ursache:

- Problem beim persistenten Speichern der Sequenz

Empfohlene Pruefung:

- seriellen Log beobachten
- Speichervorgang erneut testen
- Initialisierung und Flash-Zustand pruefen

## Wenn gar nichts mehr weiterhilft

Sinnvolle Reihenfolge:

1. seriellen Monitor oeffnen
2. Firmware neu flashen
3. LittleFS neu hochladen
4. ESP32 neu starten
5. WLAN-Verbindung und Browser erneut pruefen

Weiter zu:

- [Deployment](deployment.md)
- [Hardware](hardware.md)
- [API-Dokumentation](api.md)
