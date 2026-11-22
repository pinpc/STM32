# Blue Pill + ST-Link V2 Test

Kleines Testprojekt für den STM32F103C8T6 („Blue Pill“). Die LED an PC13 blinkt im Sekundentakt.

Das Repository enthält die benötigten CMSIS- und Standard-Peripheral-Library-Dateien im Ordner `Drivers` und ist damit eigenständig.

## Build (PowerShell)

```powershell
.\BluePill_STLink_Test\build.ps1
```

Die erzeugte Datei liegt unter `BluePill_STLink_Test/build-direct/BluePill_STLink_Test.hex`. Alternativ ist eine CMake-Konfiguration vorhanden.

## Flashen mit ST-Link V2

ST-Link anschließen: **SWDIO → PA13**, **SWCLK → PA14**, **GND → GND**, optional **3V3 → 3V3**. NRST kann zusätzlich mit NRST verbunden werden.

```powershell
.\BluePill_STLink_Test\flash.ps1 -Image .\BluePill_STLink_Test\build-direct\BluePill_STLink_Test.hex
```

Das Skript verwendet STM32CubeProgrammer über SWD, programmiert, verifiziert und startet den Controller neu. Der ST-Link darf die Platine versorgen; nicht gleichzeitig eine weitere 5-V-Versorgung einspeisen.
