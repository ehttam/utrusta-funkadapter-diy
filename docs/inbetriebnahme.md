# Inbetriebnahme und Tests

Der Aufbau wurde auf dem Mac in drei Stufen in Betrieb genommen. Erst lief der NodeMCU allein im WLAN. Danach wurde der Funkchip über SPI geprüft. Zum Schluss kam das IKEA-Funkprofil hinzu und der Betreiber testete die Leuchte. Das Vorgehen trennt Fehler am USB-Kabel, an den Stromleitungen, am SPI-Bus und am Empfänger.

## 1. Entwicklungsumgebung öffnen

1. Den ESPHome Device Builder für das Betriebssystem installieren. Beim Projekt war es die **macOS-App für Intel**, Version **1.1.3**, mit Firmware-Build über **ESPHome 2026.8.0**. Den Device Builder aus **Programme** starten und warten, bis sein Browserfenster geöffnet ist. Den erprobten CLI-Weg und die Versionsdetails findest du unter [Firmware](firmware.md).
2. Im Projekt war **`~/esphome`** der Konfigurationsordner. Home Assistant läuft separat als Container auf der Docker-VM. Es gibt dort keinen Home-Assistant-OS-App-Store für den Device Builder. Der USB-Anschluss war am Mac, und der fertige Adapter wird im LAN von Home Assistant erreicht.
3. Das private Git-Repository als Arbeitsordner bereithalten. Die lokalen Geräte- und Secrets-Dateien aus den Beispielen erzeugen oder beim Restore mit der bestehenden Konfiguration zusammenführen. Die Binärdatei mit Zugangsdaten bleibt auf dem Mac. [ESPHome-Installationswege](https://esphome.io/install/)

## 2. Konfiguration zum Nachbauen vorbereiten

1. [utrusta-kueche.example.yaml](../utrusta-kueche.example.yaml) nach `utrusta-kueche.yaml` und [secrets.yaml.example](../secrets.yaml.example) nach `secrets.yaml` kopieren. Den Inhalt des Repositorys mit `packages/` und `cc2500-spi-test.yaml` im selben Verzeichnis belassen.
2. Die Vorlage für das lokale **2,4-GHz-WLAN** und die Gerätezugänge ausfüllen. Bei einer Wiederherstellung den bestehenden ESPHome-API-Schlüssel und Gerätenamen übernehmen. Einzelheiten zu den Feldern stehen unter [Firmware, Schritt 3](firmware.md#3-firmware-manuell-nachbauen-und-aufspielen).
3. Für den ersten WLAN-Test den ganzen `packages:`-Block auskommentieren oder entfernen. Wenn am Gerät weiterhin der Feldaufbau hängt und nur die Diagnose zurückgespielt werden soll, stattdessen das Diagnosepaket wählen.
4. Der Geräteassistent wurde beim ersten Versuch über **Create device → New Device Setup**, Gerätename **`utrusta-kueche`**, Plattform **ESP8266** verwendet. Die Vorlage liefert den gleichwertigen manuellen Einstieg mit dem tatsächlich gebauten Boardprofil `esp01_1m`.

## 3. Erster USB-Flash und WLAN-Test

1. Nur den **NodeMCU** per Micro-USB-**Datenkabel** anschließen. Mit `ls /dev/cu.*` vor und nach dem Einstecken lässt sich der neue Port erkennen. Ein typischer macOS-Pfad sieht so aus: **`/dev/cu.usbserial-XXXX`**. `cu.Bluetooth-Incoming-Port` ist der Bluetooth-Port des Macs.
2. Im Device Builder **Install → Manual download** öffnen. ESPHome baut die YAML in eine Binärdatei. Die Variante für USB-Erstinstallation wählen. In Chrome oder Edge [ESPHome Web](https://web.esphome.io/) öffnen, **Connect** drücken, den seriellen Anschluss auswählen und die Datei über **Install** übertragen. Alternativ dieselben Schritte per `esphome compile` und `esphome upload` ausführen. [ESPHome: Firmware installieren](https://esphome.io/guides/faq/#how-do-i-install-esphome-onto-my-device)
3. **Logs** für den USB-Port öffnen. Bei Bedarf am NodeMCU kurz **RST** drücken. So sieht ein erfolgreicher Start aus. IP und SSID sind hier Beispiele:

   ```text
   Connected
   IP Address: 192.0.2.25
   SSID: 'BEISPIEL-WLAN'
   Hostname: 'utrusta-kueche'
   Boot seems successful; resetting boot loop counter
   ```

   Der ESP verband sich mit dem 2,4-GHz-Heimnetz. Bei DHCP ist die aktuelle IP im Log maßgeblich. Die Erreichbarkeit des ESPHome-Ports `6053` wurde anschließend vom Mac aus bestätigt.
4. In Home Assistant unter **Einstellungen → Geräte & Dienste** das entdeckte Gerät **utrusta-kueche** hinzufügen. Wenn es nicht erscheint, ESPHome mit der IP aus den Logs und Port `6053` hinzufügen. Den vorhandenen **API-Verschlüsselungsschlüssel** aus der Device-Builder-YAML beziehungsweise den Secrets eingeben. Home Assistant meldete den Eintrag anschließend als geladen. [Home-Assistant-Integration](home-assistant.md#2-den-nodemcu-in-home-assistant-hinzufügen)

**Ergebnis.** Flash, ESP8266-Start, WLAN und die Verbindung zu Home Assistant funktionieren. Zu dieser Zeit enthält die Firmware noch keine Funktasten.

## 4. Stromversorgung und Verdrahtung prüfen

Die genaue Bauanleitung steht unter [Hardware](hardware.md). Für die Reihenfolge der Inbetriebnahme sind diese Prüfungen wichtig:

1. Bei abgezogenem USB den **200-Ω-Bereich** verwenden. Referenz für direkt zusammengehaltene Spitzen **`00.7` Ω**, offene Spitzen zeigen **`1`** für Messbereichsüberschreitung. Beim losen Funkmodul wurde zwischen den richtigen Versorgungspins **1 und 6** die Folge **`1 → 60,1 → 1`** beobachtet. Die erste Messung 1–2 erfolgte vor der Korrektur des Modulplans und erfasste VCC zu SI.
2. Den nackten NodeMCU kurz einschalten und im Bereich **Gleichspannung `V⎓`** von `G` zu `3V` messen. Referenz aus dem Projekt **+3,31 V**. Danach USB abziehen.
3. Die verlötete Kondensatorplatine getrennt prüfen. Die beiden Enden jeder einzelnen Drahtbrücke zeigen Durchgang. Rot und Schwarz dürfen sich nicht dauerhaft niederohmig verbinden.
4. Die Versorgung mit zwei roten und zwei schwarzen Enden anschließen. Rot verbindet `3V` über die Brücke mit CC2500-Pin **1**. Schwarz verbindet `G` über die Brücke mit Pin **6**. Gegenprobe auf Kurzschluss im stromlosen Zustand. Danach USB verbinden und die 3,3 V **am Funkmodul** zwischen 1 und 6 prüfen. Der Betreiber meldete diese Messung als passend. Der exakte Zahlenwert am CC2500 ist nicht protokolliert.
5. USB wieder abziehen und die Signalwege **CC2500-Pin 2 → D7**, **3 → D5**, **4 → D6** und **8 → D1** verbinden. Die vier Leitungen gehen direkt zwischen den Modulen. Stromlos vom Stift des NodeMCU bis zum jeweiligen Modulpin messen und auf Zinnbrücken schauen. [Korrigierter Schaltplan](../utrusta-schaltplan.png)

## 5. SPI-Diagnose und der Fehler D8 statt D1

1. Unter `packages:` das IKEA-Paket durch die Diagnose ersetzen:

   ```yaml
   packages:
     cc2500_spi_test: !include cc2500-spi-test.yaml
   ```

   Im Projektgerät lag die Kopie historisch unter `~/esphome/packages/cc2500-spi-test.yaml`, daher war dort der Include-Pfad `packages/cc2500-spi-test.yaml`. Im Git-Klon liegt die Diagnose auf der obersten Ebene. Ein Include bezeichnet die Datei im gewählten Layout.
2. Kompilieren, über USB flashen und das Log bei 115200 Baud öffnen. `esphome logs … --reset` oder ein Druck auf **RST** löst den Test beim Neustart aus. Es wird nur die Chipkennung gelesen.
3. **Erster Befund.** Hardware-SPI mit 1 MHz meldete `PARTNUM=0x00 VERSION=0x00`. Der NodeMCU selbst war weiterhin im WLAN.
4. **Zweiter Befund.** Software-SPI mit **125 kHz**, Pull-up und Bereitschaftsprüfung meldete `SO bleibt HIGH bei CSn=LOW`. Die angezeigten `FF/FF` waren Ersatzwerte nach einem Timeout. Die Diagnose hatte zu diesem Zeitpunkt keine Kennregister ausgelesen.
5. **Ursache und Korrektur.** CC2500-Pin **8** war versehentlich mit dem NodeMCU-Aufdruck **D8** verbunden. Der ESP sendet das Auswahlsignal laut YAML über **D1** (GPIO5). Bei abgezogenem USB von D8 auf D1 umstecken. Ein Neustart mit derselben bereits aufgespielten Firmware reichte.
6. **Bestätigtes Log nach der Korrektur:**

   ```text
   SO bei CSn=HIGH: HIGH
   PARTNUM=0x80 VERSION=0x03
   SPI-TEST OK: CC2500 antwortet
   Connected
   ```

   Damit sind Versorgung und Kommunikation mit dem CC2500 im Aufbau nachgewiesen. [TI-Datenblatt und Statusregister](https://www.ti.com/lit/ds/symlink/cc2500.pdf)

## 6. IKEA-Firmware flashen und Start prüfen

1. Unter `packages:` wieder auf Produktion umstellen. WLAN, Verschlüsselung und Gerätename bleiben aus derselben Basis-YAML erhalten.

   ```yaml
   packages:
     utrusta_radio: !include packages/utrusta-radio.yaml
   ```

2. Bauen und per USB aufspielen. Beim Projekt bestätigte esptool den Flash mit `Hash of data verified.` und ESPHome mit `Successfully uploaded program.`. Der Treiber meldete die Identität des CC2500. Beispiel mit der Senderkennung `0x1234`:

   ```text
   CC2500 bereit: PARTNUM=0x80 VERSION=0x03
   Senderkennung 0x1234; warte auf Tastendruck
   Connected
   ```

3. Das bereits hinzugefügte Gerät **utrusta-kueche** in Home Assistant öffnen. Nach der Wiederverbindung waren automatisch **Anlernen**, **Licht aus**, **Licht mittel** und **Licht voll** zu sehen. Die später gemeldeten Zeitstempel der vier Entitäten belegen Tastenbetätigungen in Home Assistant. Die Funkwirkung wurde am Empfänger separat bestätigt. Das [Home-Assistant-Kapitel](home-assistant.md#3-tasten-und-direkter-funkbetrieb) enthält die Tasten und Beispiel-Entitäts-IDs.

## 7. Vor Ort anlernen und abnehmen

1. Einen kompatiblen UTRUSTA-Empfänger auswählen und den Adapter zunächst in der Nähe betreiben. Zum Nachbau kann er am USB-Netzteil laufen. Mit USB am Mac ist zusätzlich ein serielles Log möglich.
2. Den **von außen zugänglichen Sensorknopf** an der Leuchte oder am OMLOPP-Empfänger einmal drücken. Laut IKEA leuchtet der Sensor im Lernmodus auf. Wenn der Knopf unklar ist, das sichtbare Typenschild mit der Anleitung abgleichen. Die genaue Artikelnummer des eingebauten Empfängers wurde im Projekt nicht erfasst. [IKEA-Anleitung zum Anlernen, Seite 5](https://www.ikea.com/de/de/manuals/utrusta-fernbedienung__AA-906988-3_pub.pdf)
3. Unmittelbar danach in Home Assistant am Gerät **Anlernen** drücken. Der Adapter sendet die eigene Kennung aus `secrets.yaml`. Ein Blinksignal ist laut Handbuch der erwartete Hinweis. Ob der Sensor im Projekt genau dieses Blinksignal zeigte, wurde nicht gesondert protokolliert. Der Betreiber bestätigte das erfolgreiche Anlernen ohne die alte Fernbedienung.
4. Nacheinander **Licht aus → Licht mittel → Licht voll → Licht aus** betätigen und zwischen den Klicks die Leuchte beobachten. Der Betreiber bestätigte **Aus, Mittel und Voll** am realen Empfänger. Die mittlere Stufe ist eine feste Helligkeit und kein stufenloser Dimmer.
5. Bei weiteren kompatiblen Leuchten jeweils am einzelnen Empfänger erneut den Lernmodus und **Anlernen** auslösen. Für gemeinsam geschaltete Leuchten dieselbe Senderkennung beibehalten.

| Prüfung | Nachweis oder offener Punkt |
|---|---|
| USB-Flash | Serienport erkannt, Schreiben und Hash bestätigt. |
| WLAN und Home Assistant | Startlog mit IP, API-Port erreichbar, Integration geladen. |
| Hardwareversorgung | +3,31 V am NodeMCU gemessen, Wert am CC2500 als passend gemeldet. |
| SPI | `0x80/0x03`, erfolgreich nach Korrektur von D8 auf D1. |
| Produktionsfirmware | Installiert, Funkprofil initialisiert, vier Tasten registriert. |
| Anlernen und drei Lichtstufen | Betreiber bestätigte alle vier Funktionen am Empfänger am **21. November 2026**. |
| Ein-Tasten-Rotation | Skript und Helfer gespeichert und zurückgelesen. Vollständiger Rotationsdurchlauf an der Leuchte ist nicht protokolliert. |
| Dauerbetrieb | Neustart am endgültigen Aufstellort, Reichweite und Steckdosen bei per Funk ausgeschaltetem Licht separat prüfen. |

## 8. Fehler eingrenzen und einen funktionierenden Stand wiederherstellen

| Beobachtung | Nächste Prüfung |
|---|---|
| Kein neuer USB-Port | Datenkabel und USB-Port wechseln. Liste vor und nach dem Anstecken vergleichen. |
| Port da, Flash bleibt bei `Connecting` stehen | Serielles Log und Browser-Port schließen. Manuell **FLASH halten → RST kurz drücken → FLASH loslassen**. Nach Flash für Normalbetrieb nur **RST** drücken. [Espressif zum Bootloader](https://docs.espressif.com/projects/esptool/en/latest/esp8266/advanced-topics/boot-mode-selection.html) |
| WLAN verbindet sich nicht | SSID, Passwort und 2,4 GHz prüfen. Neu bauen und flashen. Die IP aus dem neuen Log nehmen. |
| WLAN da, Home Assistant offline | Erreichbarkeit der ESP-IP auf Port 6053 und den API-Schlüssel prüfen. Bei manueller Einrichtung die aktuelle IP verwenden. |
| SPI `00/00` oder Timeout | USB abziehen. Zuerst Durchgang von D1 zu Modulpin 8 sowie D6 zu Pin 4 prüfen. Danach SCLK, MOSI und die Spannung direkt am CC2500 prüfen. |
| Tasten da, Lampe reagiert nicht | Lernmodus neu starten, mit kurzem Abstand und unveränderter Senderkennung wiederholen. Start- und Sendelog mit der beobachteten Lampenreaktion abgleichen. Die Leuchte selbst liefert dem Adapter keinen Empfangsstatus zurück. |
| Firmware oder neue Konfiguration fehlerhaft | USB verwenden und die passende Stufe erneut bauen und flashen. Der WLAN-Basistest grenzt den Fehler auf den ESP ein, der SPI-Test auf Verdrahtung und Chip, die Produktionsfirmware auf den Funkpfad. Gerätename, API-Schlüssel und Senderkennung für die Wiederherstellung behalten. |

USB ist der am Gerät erprobte Installations- und Rückweg. Die Vorlage enthält OTA. Für ein Update über WLAN braucht der ESP eine im Netz erreichbare Adresse und die passende OTA-Konfiguration. Ein OTA-Upload für diesen Hardwarestand wurde nicht erprobt. [ESPHome OTA](https://esphome.io/components/ota/esphome/)
