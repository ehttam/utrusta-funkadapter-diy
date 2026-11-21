# DIY-Funkadapter für IKEA UTRUSTA und Home Assistant

Ersatz für eine defekte IKEA-UTRUSTA-Fernbedienung **E1205**. Ein USB-versorgter **NodeMCU V3 mit ESP8266** empfängt Befehle von Home Assistant per WLAN. Über vier SPI-Leitungen steuert er ein **TinyTronics CC2500, Artikel 002914**. Der Funkchip sendet die IKEA-Befehle direkt an die Leuchte. Zwei Kondensatoren sitzen auf einer kleinen Lochrasterplatine an der 3,3-V-Versorgung. Dies ist ein unabhängiges DIY-Projekt.

```text
Home Assistant ── WLAN / verschlüsselte ESPHome-API ── NodeMCU V3
NodeMCU V3 ── vier SPI-Leitungen ── CC2500 ── IKEA-Funk ── UTRUSTA
```

![Verdrahteter Prototyp](docs/bilder/adapter-verdrahtet.jpg)

## Nachbauen und verstehen

| Schritt | Anleitung |
|---|---|
| 1. Teile identifizieren, Kondensatoren verlöten, Spannung messen und SPI verdrahten | [Hardware und Aufbau](docs/hardware.md) |
| 2. Quellcode, YAML, C++, Register und den Weg vom Build bis zum Flash verstehen | [Firmware und Werkzeugkette](docs/firmware.md) |
| 3. WLAN, SPI und Funk in der erprobten Reihenfolge in Betrieb nehmen | [Inbetriebnahme](docs/inbetriebnahme.md) |
| 4. Adapter, Tasten und Ein-Tasten-Rotation in Home Assistant einrichten | [Home Assistant und Alexa](docs/home-assistant.md) |

[Schaltplan als PNG](utrusta-schaltplan.png) · [Editierbarer Schaltplan als SVG](utrusta-schaltplan.svg) · [Lochrasterzeichnung](docs/bilder/kondensatoren-lochraster.png)

**Anschlussbelegung für Artikel 002914.** Mit sichtbarem Chip, Antenne oben und Stiften unten ist **Pin 1 links VCC** und **Pin 6 GND**. **CC2500-Pin 8 geht an NodeMCU D1**. Die Belegung gilt für dieses konkrete Modul.

## Dateien für die Wiederherstellung

- [ESPHome-Gerätevorlage](utrusta-kueche.example.yaml) als `utrusta-kueche.yaml` speichern und die [Secrets-Vorlage](secrets.yaml.example) als `secrets.yaml` lokal ausfüllen.
- [SPI-Testpaket](cc2500-spi-test.yaml) zum Auslesen der Chipkennung ohne Funk.
- [IKEA-Firmwarepaket](packages/utrusta-radio.yaml) mit [C++-Funklogik](packages/utrusta_radio.h) und eigener Senderkennung aus `secrets.yaml`.
- [Home-Assistant-Skript](home-assistant/utrusta_licht_weiterschalten.yaml) für **Aus → Mittel → Voll**. Der Helfer und die Importreihenfolge sind im [Home-Assistant-Kapitel](docs/home-assistant.md) beschrieben.
- [Eigene Zeichnungen und Fotos mit Quellen](docs/bilder/README.md).

Die persönliche `secrets.yaml`, die aktive Geräte-YAML und Firmware-Binärdateien bleiben lokal. Zum Restore der bestehenden Installation den bisherigen Gerätenamen, den API-Schlüssel und die Senderkennung übernehmen. Die Firmware enthält die Zugangsdaten aus dem Build und wird daher aus den Vorlagen und dem Quellcode lokal neu erzeugt.

## Lizenz

Der eigene Firmwarecode und die YAML-Konfigurationen stehen unter [GNU GPLv3](LICENSE). Das umfasst die ESPHome-Pakete, die ESPHome-Vorlagen und das Home-Assistant-Skript. Für Fotos, Zeichnungen und Markdown-Text gilt diese Lizenzfreigabe nicht. Verlinkte Projekte behalten ihre eigenen Rechte und Lizenzbedingungen. Der Treiber implementiert das dokumentierte Protokoll und nutzt technische Register- und Telegrammwerte aus den verlinkten Referenzen. [Lizenz der ESPHome-Laufzeit](https://github.com/esphome/esphome/blob/2026.8.0/LICENSE)
