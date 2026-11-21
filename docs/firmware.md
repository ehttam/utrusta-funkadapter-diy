# Firmware, Software und Programmierung

Die Firmware läuft auf dem **ESP8266**. Sie verbindet sich über WLAN mit Home Assistant und steuert den **CC2500** über SPI. Der ESP8266 läuft nach dem Flashen allein am USB-Netzteil weiter. Der Mac wird zum Bearbeiten, Bauen, Aufspielen und zur Diagnose gebraucht.

```text
YAML und C++ auf dem Mac → ESPHome erzeugt C++ → Compiler für ESP8266 → .bin
.bin → esptool über USB-Seriell → Flash-Speicher des NodeMCU → Firmware startet
```

## 1. Verwendete Werkzeugkette

| Bestandteil | Version und Aufgabe |
|---|---|
| **macOS auf Intel** | 15.7.4, `x86_64`. Rechner für Entwicklung und USB. |
| **ESPHome Device Builder** | Mac-App **1.1.3**, mit Backend-Paket `esphome-device-builder` **1.12.2**. Die App öffnet den Editor und die Build- und Log-Oberfläche im Browser. |
| **ESPHome** | **2026.8.0**. Prüft die YAML, löst Pakete und Secrets auf und erzeugt das eigentliche C++-Programm. |
| **Python** | **3.14.7** im Backend der Mac-App. Führt ESPHome und die Entwicklungswerkzeuge auf dem Mac aus. |
| **PlatformIO** | Core **6.1.19**, Plattform `platformio/espressif8266@4.2.1`. Ruft den Compiler auf und baut mit dem **Arduino-Framework für ESP8266**. |
| **esptool** | **5.3.1**, von ESPHome für den seriellen Upload aufgerufen. Verifizierte das geschriebene Programm per Hash. |
| **USB-Seriell** | `/dev/cu.usbserial-XXXX` als Beispielpfad, Logs bei **115200 Baud**. |

**YAML** beschreibt das Gerät und seine Abläufe. Ein Abschnitt `lambda: |-` in YAML enthält **C++**. Die Funklogik steht in einer C++-Headerdatei. Beim Kompilieren wird beides zusammen mit ESPHome und Arduino in ein Programm für den ESP8266 übersetzt. **Python** ist Teil der Werkzeugkette auf dem Mac.

Der tatsächlich geflashte Build verwendete `board: esp01_1m` und Flashmodus `dout`. Das **1-MB-Buildprofil** kam aus der bestehenden Geräte-YAML. Beim USB-Flash erkannte esptool am realen NodeMCU **4 MB** physischen Flash. Die Vorlage behält das getestete Profil bei. Die SPI-Pins verwenden die expliziten GPIO-Nummern und hängen dadurch nicht von Board-Kurzbezeichnungen ab. [ESP8266 in ESPHome](https://esphome.io/components/esp8266/) · [ESPHome CLI](https://esphome.io/guides/cli/)

## 2. Dateien und ihre Rollen

| Datei | Aufgabe |
|---|---|
| [utrusta-kueche.example.yaml](../utrusta-kueche.example.yaml) | Gerätevorlage für Name, Board, WLAN, Logger, verschlüsselte API, OTA und Paketwahl. |
| [secrets.yaml.example](../secrets.yaml.example) | Leere Vorlage für WLAN, API, OTA und Fallback-AP. |
| [cc2500-spi-test.yaml](../cc2500-spi-test.yaml) | Nur lesende SPI-Diagnose. Die beim Aufbau erfolgreiche Version mit Pull-up und Timeout. |
| [packages/utrusta-radio.yaml](../packages/utrusta-radio.yaml) | Pinbelegung, eigene Senderkennung, Boot-Initialisierung, Tasten und 50-facher Sendeablauf. |
| [packages/utrusta_radio.h](../packages/utrusta_radio.h) | C++-Funklogik mit Chipprüfung, RF-Profil, SPI-Zugriff und Telegrammen. |

Die lokale Gerätekonfiguration heißt **`utrusta-kueche.yaml`**. `!secret` holt einen Wert aus `secrets.yaml`, `!include` liest ein YAML-Paket ein, und `esphome.includes` bindet den C++-Header ein. Die echte Geräte-YAML, `secrets.yaml`, der Buildordner und die fertigen Binärdateien gehören zum lokalen Arbeitsstand. Die Zugangsdaten werden beim Build in das Gerät eingebracht. [ESPHome-Pakete](https://esphome.io/components/packages/) · [ESPHome-YAML und Secrets](https://esphome.io/guides/yaml/)

## 3. Firmware manuell nachbauen und aufspielen

1. In einem **frischen Klon** des Repositories die Vorlagen kopieren. Kopiere bei einer bestehenden Installation die Vorlagen in einen separaten Arbeitsordner oder vergleiche sie mit der Geräte-YAML, damit vorhandene Schlüssel und Einstellungen erhalten bleiben.

   ```sh
   cp utrusta-kueche.example.yaml utrusta-kueche.yaml
   cp secrets.yaml.example secrets.yaml
   ```

2. In `secrets.yaml` WLAN-Name und Passwort des **2,4-GHz-Heimnetzes** einsetzen. Bei einem neuen Aufbau den API-Schlüssel mit `openssl rand -base64 32` erzeugen und eintragen. Eine eigene Funkkennung wählen. `openssl rand -hex 2` erzeugt vier Hex-Ziffern. Diese mit `0x` und in Anführungszeichen als `utrusta_sender_address` eintragen. Für OTA und den Fallback-AP jeweils einen eigenen Wert erzeugen, etwa mit je einem Aufruf von `openssl rand -hex 16`. Beim Wiederherstellen des bestehenden Geräts den damaligen API-Schlüssel übernehmen, damit die bestehende Home-Assistant-Einbindung weiter funktioniert. WLAN-Passwort, API-Schlüssel und OTA-Passwort erfüllen verschiedene Aufgaben.
3. Eine der drei Stufen wählen. **Nur ein Paket pro Build** verwenden. Beide Pakete belegen dieselben SPI-Leitungen.

   | Stufe | Einstellung in `utrusta-kueche.yaml` |
   |---|---|
   | Erster WLAN- und API-Test | Den gesamten Block `packages:` entfernen oder auskommentieren. CC2500 bleibt abgesteckt. |
   | SPI-Diagnose | Unter `packages:` nur `cc2500_spi_test: !include cc2500-spi-test.yaml` verwenden. |
   | IKEA-Firmware | Unter `packages:` nur `utrusta_radio: !include packages/utrusta-radio.yaml` verwenden. Dies ist die Vorlageinstellung. |

4. Für den Device Builder das Projektlayout im ausgewählten Konfigurationsordner erhalten. Auf dem Projekt-Mac war das **`~/esphome`**. Dort lag die Diagnose historisch unter `packages/cc2500-spi-test.yaml` und wurde über diesen Pfad eingebunden. Im Repository liegt sie im Hauptordner. Das Produktionspaket liegt in beiden Fällen zusammen mit dem Header unter `packages/`.
5. Im Device Builder beim Gerät **Edit** und dann **Validate** verwenden. Für den USB-Flash **Install** auswählen. Der nachvollziehbare Download-Weg ist **Install → Manual download → Erstinstallationsdatei**. Diese Datei in Chrome oder Edge über [ESPHome Web](https://web.esphome.io/) mit **Connect → USB-Port → Install** in den NodeMCU schreiben. Ein laufender serieller Logleser muss vor dem Flash geschlossen werden. [ESPHome-Einstieg](https://esphome.io/install/getting-started/)

### Dieselben Schritte im Terminal

Der Pfad entspricht der lokal installierten Mac-App. Führe die Befehle im Verzeichnis mit `utrusta-kueche.yaml` aus.

```sh
ESPHOME_BIN="$HOME/Library/Application Support/io.esphome.builder/python/bin/esphome"
"$ESPHOME_BIN" version
"$ESPHOME_BIN" config utrusta-kueche.yaml
"$ESPHOME_BIN" compile utrusta-kueche.yaml

ls /dev/cu.*
"$ESPHOME_BIN" upload utrusta-kueche.yaml --device /dev/cu.usbserial-XXXX
"$ESPHOME_BIN" logs utrusta-kueche.yaml --device /dev/cu.usbserial-XXXX --reset
```

Die Portnummer durch den auf dem eigenen Mac gefundenen Anschluss ersetzen. `config` validiert, `compile` baut und `upload` schreibt den letzten gebauten Stand. Nach einer YAML-Änderung erneut kompilieren. `logs --reset` liest ab dem Neustart, mit **Ctrl+C** beenden. Die Dateien liegen in `.esphome/build/utrusta-kueche/.pioenvs/utrusta-kueche/`. Beim Projekt wurden `firmware.bin`, `firmware.factory.bin` für die Serieninstallation und `firmware.ota.bin` für OTA erzeugt. Per **USB** wurde der Aufbau überprüft. OTA ist in der Geräte-YAML vorbereitet und wurde für diesen Projektstand nicht als Uploadweg erprobt. [ESPHome CLI](https://esphome.io/guides/cli/) · [ESPHome OTA](https://esphome.io/components/ota/esphome/)

Für einen Build ohne passende Mac-App lässt sich ESPHome 2026.8.0 mit Python **3.12 bis 3.14** in einer eigenen Umgebung installieren. Das entspricht dem ESPHome-Paket, während die Device-Builder-Oberfläche aus der separaten Mac-App kommt. Der erste Build bezieht die ESP8266-Entwicklungswerkzeuge aus dem Internet.

```sh
python3.14 -m venv .venv
. .venv/bin/activate
python -m pip install "esphome==2026.8.0"
esphome config utrusta-kueche.yaml
esphome compile utrusta-kueche.yaml
```

## 4. Was die SPI-Diagnose tut

| SPI-Leitung | GPIO | NodeMCU | CC2500-Modulstift |
|---|---:|---|---:|
| SI / MOSI | 13 | D7 | 2 |
| SCLK | 14 | D5 | 3 |
| SO / MISO | 12 | D6 | 4 |
| CSn | 5 | D1 | 8 |

Beide Pakete verwenden **Software-SPI**, `MODE0`, MSB zuerst und **125 kHz** als eingestellte Rate. Der ESP8266 erzeugt den Takt. `CSn` ist aktiv bei LOW. Der Pull-up am ESP-Eingang `SO` hält die Leitung im Leerlauf auf HIGH. Wenn der ausgewählte CC2500 bereit ist, zieht er `SO` auf LOW.

Der Boot-Hook wartet eine Sekunde. Dann liest er `PARTNUM` und `VERSION` über die SPI-Header **`0xF0` und `0xF1`**. Der erste Transfer gibt ein Statusbyte zurück, der zweite den Registerwert. Die Diagnose erwartet im Statusbyte das Bereitschaftsbit auf 0. Bleibt SO länger als **5 ms** HIGH, beendet sie den einzelnen Zugriff und protokolliert `0xFF` als Ersatzwert für den Timeout. Eine echte Modulantwort sah so aus:

```text
SO bei CSn=HIGH: HIGH
PARTNUM=0x80 VERSION=0x03
SPI-TEST OK: CC2500 antwortet
```

Die erste Fassung mit Hardware-SPI las bei falsch gestecktem CS-Kabel `00/00`. Die zweite, langsamere Fassung unterschied eine fehlende Antwort vom gelesenen Datenbyte. Nach Korrektur von **D8 auf D1** lief die zweite Fassung ohne erneutes Flashen erfolgreich durch. Details zu den Messungen und Logs stehen in der [Inbetriebnahme](inbetriebnahme.md#5-spi-diagnose-und-der-fehler-d8-statt-d1). [TI-Datenblatt, Statusregister](https://www.ti.com/lit/ds/symlink/cc2500.pdf) · [ESPHome-SPI](https://esphome.io/components/spi/)

## 5. Wie die IKEA-Firmware entwickelt wurde

Das RF-Profil und der Datenrahmen wurden aus dem dokumentierten Reverse Engineering von NDBCK und dem ANSLUTA-ESPHome-Referenzprojekt abgeleitet. Die eigene Implementierung benutzt ESPHomes generisches `spi_device` und einen kleinen C++-Treiber. Sie beschränkt sich auf Senden an die eigene Leuchte. Eine lauffähige Originalfernbedienung zum Auslesen der Adresse ist damit unnötig.

Beim Boot ruft die YAML `utrusta::radio.setup(...)` auf. Der Treiber

1. führt die CC2500-Resetfolge über `CSn` aus und sendet **SRES** (`0x30`),
2. wartet begrenzt auf SO und prüft die beiden Kennregister,
3. schreibt Kanal, Trägerfrequenz, Modulation, Präambel, Syncwort, Paketformat, CRC, Kalibrierung und Leistung,
4. liest `FREQ1` und `MDMCFG2` zurück und akzeptiert das Profil bei `0x93` und `0x73`,
5. protokolliert `CC2500 bereit` und wartet auf einen Tastendruck.

Das Profil verwendet **MSK**, **variable Paketlänge**, Basis-Syncwort **`0xD391`**, CRC und nominell **2436,20 MHz**. Das sind Profilwerte. Eine Messung von Frequenz oder Feldstärke mit HF-Messtechnik wurde nicht durchgeführt. `PATABLE[0] = 0xFF` entspricht laut Datenblatt einem typischen Wert von **+1 dBm** unter seinen Referenzbedingungen. [Reverse Engineering und Registerwerte](https://github.com/NDBCK/Ansluta-Remote-Controller/blob/dda687e9ef6a10365e73807c39ff6529c36d6916/AnslutaDemoCode/cc2500_VAL.h) · [ESPHome-Referenz für direktes Anlernen](https://github.com/torrottum/ikea-ansluta-esphome/blob/8811df3978861c85a830b52ef3110eae265b5562/example/direct.yaml) · [TI-Datenblatt](https://www.ti.com/lit/ds/symlink/cc2500.pdf) · [IKEA-Funkdaten, Seite 5](https://www.ikea.com/de/de/manuals/utrusta-fernbedienung__AA-906988-3_pub.pdf)

**Build-Fehler und Korrektur.** Im ersten Entwurf hatte ESPHome unter `globals:` ein C++-Objekt vom Typ `utrusta::Radio` generiert. Im erzeugten `main.cpp` stand die Deklaration vor dem Header, der den Typ definiert. Dadurch kannte der Compiler den Typ noch nicht. Die funktionierende Fassung definiert `inline Radio radio;` direkt im Header. Die YAML-Lambdas greifen nach der Einbindung über `utrusta::radio` darauf zu. Damit ließ sich der Build mit ESPHome 2026.8.0 kompilieren und aufspielen.

## 6. Wie aus einem Klick ein Funksignal entsteht

Die vier Tasten starten `utrusta_send_command` mit einem Befehlsbyte. Die eigene Senderkennung liegt unter `utrusta_sender_address` in der lokalen `secrets.yaml`. Die Beispiele zeigen `0x1234`. Sie bleibt für den vorhandenen Empfänger auch nach einem Update oder nach einem Austausch des Adapters gleich. Wird sie geändert, muss der Empfänger erneut angelernt werden.

| Taste | Befehlsbyte |
|---|---|
| **Licht aus** | `0x01` |
| **Licht mittel** | `0x02` |
| **Licht voll** | `0x03` |
| **Anlernen** | `0xFF` |

Für **Licht voll** schreibt der ESP die Bytes `7F 06 55 01 12 34 03 AA FF` über SPI. `7F` ist der Schreibheader für den TX-FIFO des CC2500. `06` gibt die Länge der Nutzdaten `55 01 12 34 03 AA` an. Darin stehen Kennung `12 34` und Befehl `03`. Das letzte `FF` folgt dem Referenzcode und liegt außerhalb der sechs Nutzbytes. Beim Anlernen lautet zusätzlich das **Befehlsbyte** `FF`. [Referenzcode für FIFO und Funkbefehle](https://github.com/NDBCK/Ansluta-Remote-Controller/blob/dda687e9ef6a10365e73807c39ff6529c36d6916/AnslutaDemoCode/AnslutaDemoCode.ino)

Pro Telegramm setzt der Treiber den Chip per **SIDLE** (`0x36`) in IDLE, leert den FIFO per **SFTX** (`0x3B`), schreibt die Daten und löst **STX** (`0x35`) aus. Nach **8 ms** prüft er über `MARCSTATE` wieder IDLE und über `TXBYTES` das Unterlaufbit. Das Skript wiederholt diesen Ablauf **50-mal**. Die Wartezeiten ergeben rund 400 ms zuzüglich SPI und Scheduling. ESPHome kann während der Wartezeit andere Aufgaben ausführen.

Bei einer Störung bricht das Skript mit einer Logmeldung ab. Der Modus `restart` erlaubt, dass ein neuer Tastendruck einen laufenden Sendeblock ersetzt. Die Ein-Tasten-Rotation in Home Assistant lässt deswegen zwischen ihren Befehlen **750 ms** Platz. `50 Telegramme gesendet` würde den Abschluss im lokalen Chip belegen. Ob die Leuchte reagiert, wird vor Ort beobachtet. [ESPHome-Skripte](https://esphome.io/components/script/) · [Home-Assistant-Rotation](home-assistant.md#4-eine-taste-für-aus-mittel-und-voll-wiederherstellen)

## 7. Geprüfter Stand

Der Firmware-Upload per USB wurde per Hash bestätigt. Die Produktionsfirmware meldete `CC2500 bereit: PARTNUM=0x80 VERSION=0x03`, `Senderkennung […]; warte auf Tastendruck` und den erneuten WLAN-Verbindungsaufbau. Home Assistant lud die vier Tasten. Der Betreiber bestätigte danach erfolgreiches Anlernen sowie **Aus, Mittel und Voll** an der Leuchte. Das belegt die praktische Funktion der Funkstrecke für den verwendeten Empfänger. Schrittfolge, Fehlersuche und getrennte Abnahmepunkte stehen in [Inbetriebnahme und Tests](inbetriebnahme.md).

Für die Repository-Abnahme wurden Basis, SPI-Diagnose und IKEA-Variante mit dem installierten ESPHome **2026.8.0** und künstlichen Zugangsdaten validiert. Außerdem wurde in einem frischen temporären Ordner aus der Gerätevorlage und dem IKEA-Paket `firmware.factory.bin` erfolgreich gebaut. Dieser Reproduktionsbuild wurde nicht geflasht. Die am Gerät getestete Firmware stammt aus dem ursprünglichen lokalen Build mit identischem IKEA-Paket und den echten lokalen Zugangsdaten.
