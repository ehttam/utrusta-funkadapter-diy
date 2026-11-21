# Inbetriebnahme

Installiere zuerst die Basisfirmware und verbinde den NodeMCU mit dem WLAN. Danach prüfst du den CC2500 über SPI und spielst die IKEA-Firmware für das Anlernen und Schalten auf.

## 1. Entwicklungsumgebung öffnen

1. Den ESPHome Device Builder für das Betriebssystem installieren. Beim Projekt war es die **macOS-App für Intel**, Version **1.1.3**, mit Firmware-Build über **ESPHome 2026.8.0**. Den Device Builder aus **Programme** starten und warten, bis sein Browserfenster geöffnet ist. Den erprobten CLI-Weg und die Versionsdetails findest du unter [Firmware](firmware.md).
2. Im Projekt war **`~/esphome`** der Konfigurationsordner. Home Assistant läuft separat als Container auf der Docker-VM. Es gibt dort keinen Home-Assistant-OS-App-Store für den Device Builder. Der USB-Anschluss war am Mac, und der fertige Adapter wird im LAN von Home Assistant erreicht.
3. Das private Git-Repository als Arbeitsordner bereithalten. Die lokalen Geräte- und Secrets-Dateien aus den Beispielen erzeugen oder beim Restore mit der bestehenden Konfiguration zusammenführen. Die Binärdatei mit Zugangsdaten bleibt auf dem Mac. [ESPHome-Installationswege](https://esphome.io/install/)

## 2. Konfiguration zum Nachbauen vorbereiten

1. [utrusta-kueche.example.yaml](../utrusta-kueche.example.yaml) nach `utrusta-kueche.yaml` und [secrets.yaml.example](../secrets.yaml.example) nach `secrets.yaml` kopieren. Den Inhalt des Repositorys mit `packages/` und `cc2500-spi-test.yaml` im selben Verzeichnis belassen.
2. Die Vorlage für das lokale **2,4-GHz-WLAN** und die Gerätezugänge ausfüllen. Bei einer Wiederherstellung den bestehenden ESPHome-API-Schlüssel und Gerätenamen übernehmen. Einzelheiten zu den Feldern stehen unter [Firmware, Schritt 3](firmware.md#3-firmware-manuell-nachbauen-und-aufspielen).
3. Für den ersten WLAN-Test den ganzen `packages:`-Block auskommentieren oder entfernen. Wenn am Gerät weiterhin der Feldaufbau hängt und nur die Diagnose zurückgespielt werden soll, stattdessen das Diagnosepaket wählen.
4. Wenn du den Geräteeintrag über den Assistenten anlegst, wähle **Create device → New Device Setup**, Gerätename **`utrusta-kueche`**, Plattform **ESP8266**. Die Vorlage verwendet das Boardprofil `esp01_1m`.

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

   Bei DHCP steht die zugewiesene IP im Log. Die ESPHome-API ist auf TCP-Port `6053` erreichbar.
4. In Home Assistant unter **Einstellungen → Geräte & Dienste** das entdeckte Gerät **utrusta-kueche** hinzufügen. Wenn es nicht erscheint, ESPHome mit der IP aus den Logs und Port `6053` hinzufügen. Den vorhandenen **API-Verschlüsselungsschlüssel** aus der Device-Builder-YAML beziehungsweise den Secrets eingeben. [Home-Assistant-Integration](home-assistant.md#2-den-nodemcu-in-home-assistant-hinzufügen)

## 4. Stromversorgung und Verdrahtung prüfen

Die genaue Bauanleitung steht unter [Hardware](hardware.md). Für die Reihenfolge der Inbetriebnahme sind diese Prüfungen wichtig:

1. Bei abgezogenem USB den **200-Ω-Bereich** verwenden. Zuerst die Messspitzen zusammenhalten, um den Vergleichswert für Durchgang zu bestimmen. Am losen Funkmodul zwischen **Pin 1 und Pin 6** auf dauerhaften Kurzschluss prüfen.
2. Den nackten NodeMCU kurz einschalten und im Bereich **Gleichspannung `V⎓`** von `G` zu `3V` messen. Referenz aus dem Projekt **+3,31 V**. Danach USB abziehen.
3. Die verlötete Kondensatorplatine getrennt prüfen. Die beiden Enden jeder einzelnen Drahtbrücke zeigen Durchgang. Rot und Schwarz dürfen sich nicht dauerhaft niederohmig verbinden.
4. Die Versorgung mit zwei roten und zwei schwarzen Enden anschließen. Rot verbindet `3V` über die Brücke mit CC2500-Pin **1**. Schwarz verbindet `G` über die Brücke mit Pin **6**. Gegenprobe auf Kurzschluss im stromlosen Zustand. Danach USB verbinden und die 3,3 V **am Funkmodul** zwischen 1 und 6 prüfen. Der Wert sollte bei etwa **+3,3 V** liegen.
5. USB wieder abziehen und die Signalwege **CC2500-Pin 2 → D7**, **3 → D5**, **4 → D6** und **8 → D1** verbinden. Die vier Leitungen gehen direkt zwischen den Modulen. Stromlos vom Stift des NodeMCU bis zum jeweiligen Modulpin messen und auf Zinnbrücken schauen. [Schaltplan](../utrusta-schaltplan.png)

## 5. SPI-Diagnose

1. Unter `packages:` das IKEA-Paket durch die Diagnose ersetzen:

   ```yaml
   packages:
     cc2500_spi_test: !include cc2500-spi-test.yaml
   ```

   Der Include-Pfad bezieht sich auf den Konfigurationsordner.
2. Kompilieren, über USB flashen und das Log bei 115200 Baud öffnen. `esphome logs … --reset` oder ein Druck auf **RST** löst den Test beim Neustart aus. Es wird nur die Chipkennung gelesen.
3. Im Startlog die Chipkennung prüfen:

   ```text
   SO bei CSn=HIGH: HIGH
   PARTNUM=0x80 VERSION=0x03
   SPI-TEST OK: CC2500 antwortet
   Connected
   ```

   Die Kennung zeigt, dass der ESP8266 mit dem CC2500 kommuniziert. [TI-Datenblatt und Statusregister](https://www.ti.com/lit/ds/symlink/cc2500.pdf)

## 6. IKEA-Firmware flashen und Start prüfen

1. Unter `packages:` wieder auf Produktion umstellen. WLAN, Verschlüsselung und Gerätename bleiben aus derselben Basis-YAML erhalten.

   ```yaml
   packages:
     utrusta_radio: !include packages/utrusta-radio.yaml
   ```

2. Bauen und per USB aufspielen. Nach dem USB-Upload die Meldung `Successfully uploaded program.` abwarten und das Startlog des CC2500 prüfen. Beispiel mit der Senderkennung `0x1234`:

   ```text
   CC2500 bereit: PARTNUM=0x80 VERSION=0x03
   Senderkennung 0x1234; warte auf Tastendruck
   Connected
   ```

3. In Home Assistant das ESPHome-Gerät **utrusta-kueche** öffnen. Nach der Wiederverbindung stehen **Anlernen**, **Licht aus**, **Licht mittel** und **Licht voll** zur Verfügung. [Home-Assistant-Tasten und Entitäts-IDs](home-assistant.md#3-tasten-und-direkter-funkbetrieb)

## 7. Leuchte anlernen

1. Einen kompatiblen UTRUSTA-Empfänger auswählen und den Adapter zunächst in der Nähe betreiben. Zum Nachbau kann er am USB-Netzteil laufen. Mit USB am Mac ist zusätzlich ein serielles Log möglich.
2. Den **von außen zugänglichen Sensorknopf** an der Leuchte oder am OMLOPP-Empfänger einmal drücken. Laut IKEA leuchtet der Sensor im Lernmodus auf. Wenn der Knopf unklar ist, das sichtbare Typenschild mit der Anleitung abgleichen. [IKEA-Anleitung zum Anlernen, Seite 5](https://www.ikea.com/de/de/manuals/utrusta-fernbedienung__AA-906988-3_pub.pdf)
3. Unmittelbar danach in Home Assistant am Gerät **Anlernen** drücken. Der Adapter sendet die eigene Kennung aus `secrets.yaml`. Ein Blinksignal ist laut Handbuch der erwartete Hinweis. Zum Anlernen genügt der Adapter mit seiner eigenen Kennung.
4. Nacheinander **Licht aus → Licht mittel → Licht voll → Licht aus** betätigen und zwischen den Klicks die Leuchte beobachten. Die mittlere Stufe ist eine feste Helligkeit und kein stufenloser Dimmer.
5. Bei weiteren kompatiblen Leuchten jeweils am einzelnen Empfänger erneut den Lernmodus und **Anlernen** auslösen. Für gemeinsam geschaltete Leuchten dieselbe Senderkennung beibehalten.
