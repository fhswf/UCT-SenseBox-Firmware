# SenseBox Firmware - Urban Climate Twin Soest

Firmware  für die senseBox im Projekt UCT. Die Firmware erfasst Umweltdaten, verarbeitet die
Messwerte und überträgt sie per LoRaWAN.

## Urban Climate Twin Soest

 [Urban Climate Twin (UCT) Soest](https://www.fh-swf.de/de/forschung___transfer_4/forschungsprojekte_1/forschungsprojekt_uct.php) ist ein digitaler Zwilling des Soester Stadtklimas. Er führt Messdaten, Geodaten und weitere Klimadaten zusammen und macht das Stadklima räumlich nachvollziehbar. Bürger und die Stadtverwaltung können so die Auswirkungen von Klimaveränderungen auf das Stadtgebiet besser verstehen und Maßnahmen zur Anpassung an den Klimawandel entwickeln.

## Funktionen

- Erfassung von Temperatur, Luftfeuchtigkeit, Luftdruck, Helligkeit und UV-Wert
- Periodische Verarbeitung der Messungen und Zusammenfassung der Messwerte
- Versand von Mess- und Statusdaten über LoRaWAN
- Protokollierung, Zeitverwaltung und Überwachung der Tasks mit einem Watchdog

Die Firmware ist für das senseBox-SAMD-Board ausgelegt. Welche Sensorvarianten
unterstützt werden, ist in den jeweiligen Modulen unter `Software` und in
den Anforderungen dokumentiert.

## Voraussetzungen

- Arduino-Core für das senseBox-SAMD-Board (`sensebox:samd`)
- Arduino-Bibliotheken, insbesondere LMIC/LoRaWAN,
  `LoraMessage`, `senseBoxIO`, Adafruit-Sensorbibliotheken, `LTR329`,
  `VEML6070`, `Adafruit_SleepyDog` und `FlashStorage`

Die benötigten Bibliotheken müssen in der Arduino-Entwicklungsumgebung
installiert sein, bevor der Sketch kompiliert werden kann.


