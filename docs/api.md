# API-Dokumentation

Zur Dokumentationsuebersicht: [docs/README.md](README.md)

## Uebersicht

Die Firmware stellt eine einfache HTTP-basierte API zur Steuerung des Roboterarms bereit. Alle Endpunkte werden direkt vom ESP32 bedient.

Basisadresse im Standardfall:

`http://192.168.4.1`

Die Weboberflaeche selbst nutzt diese Endpunkte direkt ueber `fetch()`.

## Statische Dateien

### `GET /`

Liefert die Weboberflaeche aus `index.html`.

Antwort:

- `200 OK` mit `text/html`
- `404 Not Found`, wenn die Datei auf LittleFS fehlt

### `GET /style.css`

Liefert das Stylesheet der Weboberflaeche.

### `GET /script.js`

Liefert die Browserlogik der Weboberflaeche.

## Servo-Bewegung

### `GET /startMove?servo=<id>&dir=<direction>`

Startet die manuelle Bewegung eines Servos.

Parameter:

- `servo`: Servo-ID von `0` bis `5`
- `dir`: `-1` fuer negative Bewegung oder `1` fuer positive Bewegung

Antworten:

- `200 OK` mit `move start`
- `400 Bad Request` bei fehlenden oder ungueltigen Parametern

Beispiel:

`/startMove?servo=2&dir=1`

### `GET /stopMove?servo=<id>`

Stoppt die manuelle Bewegung eines Servos.

Parameter:

- `servo`: Servo-ID von `0` bis `5`

Antworten:

- `200 OK` mit `move stop`
- `400 Bad Request` bei ungueltiger Servo-ID

Beispiel:

`/stopMove?servo=2`

## Geschwindigkeiten

### `GET /setSpeed?servo=<id>&value=<speed>`

Setzt die Bewegungsgeschwindigkeit eines Servos.

Parameter:

- `servo`: Servo-ID von `0` bis `5`
- `value`: Geschwindigkeitswert von `1` bis `10`

Antworten:

- `200 OK` mit `speed updated`
- `400 Bad Request` bei fehlenden oder ungueltigen Parametern

Beispiel:

`/setSpeed?servo=4&value=8`

### `GET /speeds`

Liefert die aktuellen Geschwindigkeitswerte aller Servos als JSON-Array.

Antwortbeispiel:

```json
[5,5,5,5,5,5]
```

## Positionen

### `GET /positions`

Liefert die aktuellen Positionen aller sechs Servos als JSON-Array.

Antwortbeispiel:

```json
[18,90,180,96,0,90]
```

## Grundfunktionen

### `GET /home`

Faehrt alle Servos in die definierte Grundstellung.

Antworten:

- `200 OK` mit `home`

### `GET /stop`

Stoppt die aktuelle Sequenzwiedergabe und beendet laufende Bewegungen.

Antworten:

- `200 OK` mit `stop`

## Sequenzverwaltung

### `GET /record`

Speichert die aktuelle Servo-Position als neuen Sequenzschritt.

Verhalten:

- stoppt zunaechst eine laufende Sequenz
- haengt die aktuelle Stellung an die gespeicherte Sequenz an
- speichert die Sequenz anschliessend persistent im Flash

Antworten:

- `200 OK` mit `recorded`
- `400 Bad Request` mit `sequence limit reached`
- `500 Internal Server Error` mit `persist failed`

### `GET /overwriteSequence`

Loescht die bestehende Sequenz und ersetzt sie durch genau einen neuen Schritt mit der aktuellen Position.

Antworten:

- `200 OK` mit `overwritten`
- `400 Bad Request` mit `sequence limit reached`
- `500 Internal Server Error` mit `persist failed`

### `GET /playSequence`

Startet die Wiedergabe der gespeicherten Sequenz.

Antworten:

- `200 OK` mit `play sequence`
- `400 Bad Request` mit `no recorded sequence`

### `GET /clearSequence`

Loescht die gespeicherte Sequenz und speichert den leeren Zustand persistent.

Antworten:

- `200 OK` mit `clear`
- `500 Internal Server Error` mit `persist failed`

## Sequenzstatus

### `GET /sequenceStatus`

Liefert einen kompakten Status der Sequenzwiedergabe.

Antwortbeispiel:

```json
{
  "count": 4,
  "playing": true,
  "currentStep": 1
}
```

Bedeutung:

- `count`: Anzahl gespeicherter Schritte
- `playing`: gibt an, ob gerade abgespielt wird
- `currentStep`: aktueller Schritt oder `-1`, wenn nichts aktiv ist

### `GET /sequenceData`

Liefert den kompletten Sequenzstatus inklusive aller gespeicherten Schritte.

Antwortbeispiel:

```json
{
  "count": 2,
  "playing": false,
  "currentStep": -1,
  "steps": [
    [18,90,180,96,0,90],
    [30,80,150,96,10,100]
  ]
}
```

## Fehlerverhalten

Typische Fehlerursachen:

- fehlender Query-Parameter
- ungueltige Servo-ID
- ungueltiger Richtungswert
- ungueltiger Geschwindigkeitswert
- keine gespeicherte Sequenz vorhanden
- Persistenzfehler beim Schreiben in den Flash

Die API verwendet derzeit einfache Textantworten fuer Fehler und kein einheitliches JSON-Fehlerformat.

## Hinweise fuer Weiterentwicklung

Sinnvolle Erweiterungen der API:

- `POST` statt `GET` fuer schreibende Operationen
- einheitliche JSON-Fehlerobjekte
- Endpunkt fuer Systemstatus
- Endpunkt fuer WLAN- und Speicherinformationen
- Import/Export kompletter Sequenzen

Weiter zu:

- [Architektur](architecture.md)
- [Deployment](deployment.md)
- [Troubleshooting](troubleshooting.md)
