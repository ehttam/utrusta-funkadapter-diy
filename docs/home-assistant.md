# Home Assistant und Sprachsteuerung

## 1. Einordnung ins Homelab

Der erprobte Aufbau nutzte **Home Assistant 2026.10.1** als Container im Heimnetz. Die Firmware wurde über ESPHome Device Builder auf einem Intel-Mac kompiliert und dort per USB installiert. Zum Build und USB-Flash siehe [Inbetriebnahme](inbetriebnahme.md).

Home Assistant verbindet sich im Heimnetz direkt mit der IP des NodeMCU über **TCP 6053**. Die IP-Adresse und der Gerätename stehen im seriellen ESPHome-Log. In dieser Dokumentation dienen `192.0.2.25` und `BEISPIEL-WLAN` als Platzhalter. Verwende zum Nachbauen die Werte deines eigenen Netzes. [ESPHome-Integration](https://www.home-assistant.io/integrations/esphome/) · [Native API](https://esphome.io/components/api/)

## 2. Den NodeMCU in Home Assistant hinzufügen

1. Den NodeMCU mit der [Basiskonfiguration](inbetriebnahme.md#2-konfiguration-zum-nachbauen-vorbereiten) starten und die zugewiesene IP im seriellen Log notieren.
2. In Home Assistant **Einstellungen → Geräte & Dienste** öffnen. Unter **Entdeckt** das Gerät **utrusta-kueche** auswählen und **Hinzufügen** drücken.
3. Falls es dort fehlt, **Integration hinzufügen → ESPHome** auswählen. Die IP des NodeMCU als **Host** und **6053** als Port eingeben.
4. Im Dialog den vorhandenen **ESPHome-API-Schlüssel** einsetzen. Im Device Builder steht er unter `api → encryption → key`. Bei `!secret …` den passenden Wert aus den lokalen Secrets kopieren. Dieser Schlüssel ist vom WLAN- und OTA-Passwort getrennt.
5. Prüfen, ob das Gerät **utrusta-kueche** unter ESPHome geladen ist. Bei Bedarf das Gerät einem Bereich zuordnen. Verwende die Entitäts-IDs deiner Installation. Nach dem USB-Flash der IKEA-Firmware verbindet sich das vorhandene Gerät mit unverändertem Namen und API-Schlüssel wieder. Die vier Tasten kommen über die Integration hinzu.

## 3. Tasten und direkter Funkbetrieb

| Taste auf dem Gerät | Beispiel-Entitäts-ID | Wirkung |
|---|---|---|
| Anlernen | `button.utrusta_kueche_anlernen` | Sendet die feste Senderkennung als IKEA-Anlerntelegramm. Die Taste steht bei der Gerätekonfiguration. |
| Licht aus | `button.utrusta_kueche_licht_aus` | Sendet Aus. |
| Licht mittel | `button.utrusta_kueche_licht_mittel` | Sendet die mittlere feste Stufe. |
| Licht voll | `button.utrusta_kueche_licht_voll` | Sendet die volle feste Stufe. |

Zum Anlernen erst den außen zugänglichen IKEA-Sensorknopf am Empfänger drücken und danach in Home Assistant **Anlernen** betätigen. Der komplette Ablauf mit Prüfung der Leuchte steht in [Inbetriebnahme](inbetriebnahme.md#7-vor-ort-anlernen-und-abnehmen). Home Assistant hat die Tastenbetätigungen registriert. Die physische Leuchtenreaktion auf alle drei Stufen hat der Betreiber separat bestätigt. Die ESPHome-Tasten melden einen Betätigungszeitpunkt und der CC2500 prüft den lokalen Sendeablauf. Der IKEA-Empfänger liefert dem Adapter keinen Lichtzustand zurück.

## 4. Eine Taste für Aus, Mittel und Voll wiederherstellen

Die Rotation ist als **Home-Assistant-Skript** umgesetzt. Der Dropdown-Helfer speichert den *nächsten Befehl*. Im Home Assistant des Projekts wurden beide Objekte über die Konfigurationsschnittstelle angelegt und anschließend zurückgelesen. Das lässt sich auch manuell über die Oberfläche wiederholen.

1. In **Einstellungen → Geräte & Dienste → Helfer** auf **Helfer erstellen** klicken und **Dropdown** wählen.
2. Den Helfer **UTRUSTA nächster Lichtbefehl** nennen. Die Optionen in genau dieser Reihenfolge eintragen: **Aus**, **Mittel**, **Voll**. In der Detailansicht die Entitäts-ID `input_select.utrusta_nachster_lichtbefehl` prüfen und die Auswahl für den ersten Durchlauf auf **Aus** setzen.
3. **Einstellungen → Automatisierungen & Szenen → Skripte → Skript erstellen** öffnen. Ein leeres Skript anlegen und über das Dreipunkt-Menü **In YAML bearbeiten** wählen.
4. Den Inhalt der [gesicherten Skript-YAML](../home-assistant/utrusta_licht_weiterschalten.yaml) vollständig in diesen Editor einfügen. Die Datei gehört in den Editor **eines einzelnen Skripts**. Falls Home Assistant beim Nachbau andere Entitäts-IDs vergeben hat, vorher alle drei Ziel-IDs und die Helfer-ID in der YAML anpassen.
5. Speichern. In den Entitätseinstellungen die ID `script.utrusta_licht_weiterschalten` prüfen. Bei **Ausführen** geht jeweils ein Befehl an den ESP. Optional dieselbe Skriptentität auf eine Schaltflächenkarte im Dashboard legen und die Tippaktion auf `script.turn_on` setzen.

Das Skript prüft zuerst die Verfügbarkeit des Adapters, wählt nach dem Helfer die passende Lichttaste und schiebt den Helfer über `input_select.select_next` mit `cycle: true` weiter. Nach **Voll** ist der nächste Befehl wieder **Aus**. Der Modus `queued` verarbeitet schnelle Mehrfachklicks nacheinander, mit 750 ms Zeitabstand. So bekommt der CC2500 Zeit für seinen Paketblock. Die drei einzelnen Lichttasten funktionieren weiterhin direkt und ändern die gespeicherte Position der Rotation nicht. Die Rotation zählt ihre eigenen Aufrufe. [Dropdown-Helfer](https://www.home-assistant.io/integrations/input_select/) · [Skript-Modi](https://www.home-assistant.io/integrations/script/)

### Vorlage für einen Klick auf dem Dashboard

Im gewünschten Dashboard den Bearbeitungsmodus öffnen und eine Schaltflächenkarte hinzufügen. Im YAML-Editor der Karte kann diese Vorlage eingesetzt werden. Sie ruft das bereits eingerichtete Skript auf.

```yaml
type: button
name: UTRUSTA weiterschalten
icon: mdi:lightbulb-on-outline
entity: script.utrusta_licht_weiterschalten
show_state: false
tap_action:
  action: perform-action
  perform_action: script.turn_on
  target:
    entity_id: script.utrusta_licht_weiterschalten
```

Die Karte ist eine Vorlage zum manuellen Einbauen. Im Projekt wurde das Skript live eingerichtet und die gespeicherte Konfiguration geprüft. Ein vollständiger Rotationsdurchlauf an der Leuchte wurde noch nicht dokumentiert.

## 5. Alexa über dasselbe Skript

Beim Abgleich war keine Alexa-Integration sichtbar. Eine Kontoverknüpfung und ein Sprachtest sind für dieses Projekt nicht protokolliert. Mit **Home Assistant Cloud von Nabu Casa** kann Alexa dieselbe Skriptentität aufrufen. Damit verwenden Sprachbefehl und Home-Assistant-Schaltfläche dieselbe Reihenfolge.

1. Home Assistant Cloud einrichten und in **Einstellungen → Sprachassistenten** Alexa aktivieren.
2. Auf dem Reiter für die Freigabe von Entitäten `script.utrusta_licht_weiterschalten` für Alexa freigeben.
3. In der Alexa-App den Skill **Home Assistant Smart Home** hinzufügen, das Nabu-Casa-Konto verknüpfen und nach Geräten bzw. Szenen suchen lassen.
4. Für **„Alexa, Küchenlicht weiter“** eine Alexa-Routine mit dem Sprachtrigger **Küchenlicht weiter** anlegen. Als Aktion unter **Smart Home → Szenen** das freigegebene Skript auswählen.
5. Nach dem Speichern mit einem Sprechbefehl testen. Die Sprachroutine führt die Rotation aus. Die Gerätefunktion **Anlernen** bleibt eine eigene Taste in Home Assistant. [Nabu Casa: Alexa einrichten](https://support.nabucasa.com/hc/en-us/articles/25619363899677-Configuring-Amazon-Alexa-to-work-with-Home-Assistant) · [Alexa Smart Home: Skripte](https://www.home-assistant.io/integrations/alexa.smart_home/#script)
