# Projektplan

Zur Dokumentationsuebersicht: [docs/README.md](README.md)

## Projektziel

Ziel des Projekts ist ein uebersichtlich aufgebauter, browserbasierter Roboterarm-Controller auf ESP32-Basis, der lokal im eigenen WLAN des Geraets betrieben werden kann.

Der aktuelle Stand deckt bereits die Kernfunktionen fuer manuelle Steuerung, Geschwindigkeiten, Grundstellung, Sequenzen und persistente Speicherung ab.

## Bereits umgesetzt

- PlatformIO-Projekt fuer ESP32 eingerichtet
- Webserver auf dem ESP32 umgesetzt
- WLAN-Access-Point fuer den lokalen Zugriff eingerichtet
- Steuerung von sechs Servos implementiert
- Grundstellung fuer alle Servos umgesetzt
- manuelle Bewegung ueber Weboberflaeche realisiert
- individuelle Servo-Geschwindigkeiten umgesetzt
- Sequenzen aufnehmen, loeschen, ueberschreiben und abspielen
- persistente Speicherung von Sequenzen im Flash
- Weboberflaeche aus `main.cpp` in separate Dateien ausgelagert
- Auslieferung der Webdateien ueber LittleFS integriert
- Legacy-Ordner `web/` entfernt
- README und Projektdokumentation begonnen

## Kurzfristige Ziele

Diese Punkte sind als naechste praktische Verbesserungen sinnvoll:

- Dokumentation in `docs/` weiter pflegen und aktuell halten
- Hardware-Dokumentation im Ordner `hardware/` konkretisieren
- Screenshots oder Demo-Bilder fuer README und Dokumentation ergaenzen
- Fehler- und Randfaelle im Browser besser sichtbar machen
- Bedienung auf Mobilgeraeten weiter testen

## Mittelfristige Ziele

- Quellcode in mehrere C++-Module aufteilen
- Kalibrierung der Servo-Grenzen einfacher wartbar machen
- API robuster und einheitlicher gestalten
- einzelne Sequenzschritte im UI bearbeitbar machen
- bessere Statusanzeigen fuer laufende Bewegungen und Speicherzustand
- Export und Import von Sequenzen ermoeglichen

## Langfristige Ziele

- alternative Betriebsarten, z. B. Station-Mode statt nur Access-Point
- Benutzer- oder Zugriffsschutz fuer Steuerfunktionen
- Makros oder komplexere Bewegungsablaeufe
- Telemetrie oder Diagnoseseite
- Konfigurationsspeicherung fuer mehr als nur Sequenzen
- moegliche Erweiterung um PCA9685 oder weitere Hardware

## Offene technische Themen

- bessere Trennung zwischen Bewegungslogik, Netzwerklogik und Persistenz
- saubere Fehlerbehandlung und eventuell Logging-Konzept
- Strategie fuer Tests, besonders fuer API- und Zustandslogik
- Umgang mit Hardwaregrenzen und Kalibrierung bei veraenderter Mechanik

## Vorschlag fuer Dokumentationsstruktur

Die vorhandenen Dateien koennen so genutzt werden:

- `architecture.md`: technischer Aufbau und Zusammenspiel der Komponenten
- `api.md`: HTTP-Endpunkte, Parameter und Antworten
- `project-plan.md`: Roadmap, naechste Schritte und offene Themen

Sinnvolle spaetere Ergaenzungen:

- `hardware.md` fuer Stromversorgung, Servo-Anschluesse und Sicherheitsaspekte
- `deployment.md` fuer Flash- und Update-Prozess
- `troubleshooting.md` fuer typische Fehlerbilder

## Pflegehinweise

Damit die Dokumentation nicht veraltet, sollte sie aktualisiert werden, wenn:

- neue API-Endpunkte hinzukommen
- sich Pins, Limits oder Home-Positionen aendern
- die Weboberflaeche neue Kernfunktionen bekommt
- sich der Flash- oder Upload-Prozess aendert

Weiter zu:

- [Architektur](architecture.md)
- [API-Dokumentation](api.md)
- [Deployment](deployment.md)
