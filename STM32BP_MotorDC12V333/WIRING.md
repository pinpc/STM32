# Verdrahtung: Blue Pill, IBT-2 und Encoder-Motor

Diese Anleitung entspricht der Pinbelegung in `Core/Src/main.c` und dem
IBT-2/BTS7960-H-Brueckentreiber aus dem Projekt-README. Vor dem Anschliessen
der Motorversorgung zuerst alle Verbindungen bei ausgeschalteter Versorgung
herstellen.

## Signale zwischen Blue Pill und IBT-2

| Blue Pill STM32F103C8T6 | IBT-2 | Funktion im Programm |
|---|---|---|
| PA0 | LPWM | TIM2_CH1, PWM fuer Drehrichtung 1 |
| PA1 | RPWM | TIM2_CH2, PWM fuer Drehrichtung 2 |
| PB0 | R_EN und L_EN | Gemeinsamer Enable-Ausgang; wird beim Start auf HIGH gesetzt |
| PB1 | — | Nicht angeschlossen und nicht von der Firmware verwendet |
| GND | GND | Gemeinsame Signalmasse |
| — | R_IS, L_IS | Strommess-Ausgaenge; derzeit nicht verwendet, isoliert lassen |

PA0/PA1 liefern ein PWM-Signal mit etwa 1 kHz. Die Firmware haelt beide
PWM-Ausgaenge beim Start auf 0. PB0 und PB1 gehen dagegen auf HIGH. Daher
muessen EN-Leitungen vor dem Einschalten des Motors passend zum IBT-2-Modul
angeschlossen sein.

## Motor und Versorgung

| Anschluss | Verbindung |
|---|---|
| IBT-2 B+ / VM / +12 V | Plus der passenden Motorversorgung (Motor-Nennspannung beachten) |
| IBT-2 B- / GND | Minus der Motorversorgung |
| IBT-2 M+ und M- | Die beiden Motorleitungen |
| IBT-2 VCC | 5-V-Logikversorgung gemaess Beschriftung/Datenblatt des Moduls |
| IBT-2 GND | Gemeinsame Masse mit Blue Pill und Motorversorgung |

Die Motorversorgung nicht an den 3.3-V-Pin des Blue Pill anschliessen. Die
Motorversorgung muss den Anlauf- und Blockierstrom des Motors liefern koennen.
Die Masse des Blue Pill, die IBT-2-Logikmasse und B- der Motorversorgung
muessen verbunden sein. Den Blue Pill separat per USB oder geregelter 5-V-
Versorgung betreiben. Nicht gleichzeitig mehrere Versorgungen auf einen Pin
schalten.

## Encoder zum Blue Pill

| Encoder-Signal | Blue Pill STM32F103C8T6 | Funktion |
|---|---|---|
| Kanal A / C1 | PA8 | TIM1_CH1 |
| Kanal B / C2 | PA9 | TIM1_CH2 |
| Encoder-GND | GND | Gemeinsame Masse |
| Encoder-VCC | 3.3 V vom Blue Pill | Encoder-Versorgung |

PA8 und PA9 sind im Programm als Eingänge mit Pull-up konfiguriert und werden
von TIM1 im Quadratur-Encoder-Modus gelesen. Vor dem Verbinden der Encoder-
Signale sicherstellen, dass deren HIGH-Pegel maximal 3.3 V betraegt. Der
Encoder-VCC wird hier mit 3.3 V vom Blue Pill verbunden; das ist nur zulaessig,
wenn die konkrete Encoder-Variante fuer 3.3 V Versorgung spezifiziert ist.
Falls der Encoder 5 V Versorgung benoetigt oder 5 V-Signale ausgibt, einen
geeigneten Pegelwandler bzw. eine passende Versorgung verwenden. Die
Kabelfarben und die Encoder-Versorgung unterscheiden sich je nach JGB37-520-
Variante; Pinbelegung am konkreten Motor/Datenblatt pruefen.

Die Firmware berechnet den vorzeichenbehafteten RPM-Wert alle etwa 10 ms.
Im Debugger kann `g_motor_rpm` beobachtet werden. In `Core/Src/main.c` ist
`ENCODER_COUNTS_PER_OUTPUT_REV` derzeit auf 1320 gesetzt (Annahme: 11 Encoder-
Impulse pro Motorumdrehung, 30:1 Getriebe und x4-Quadraturzaehlung). Diesen
Wert anhand des Motordatenblatts oder einer Kalibrierung fuer die tatsaechliche
Variante anpassen; andernfalls ist der angezeigte RPM-Wert nicht genau.

## ST-Link V2 zum Flashen und Debuggen

| ST-Link V2 | Blue Pill |
|---|---|
| SWDIO | PA13 / SWDIO |
| SWCLK | PA14 / SWCLK |
| GND | GND |
| 3.3 V / VTref | 3.3-V-Referenz des Targets; nur als Versorgung nutzen, wenn der ST-Link-Ausgang dafuer vorgesehen ist |

Fuer den normalen Flash-Vorgang genuegen SWDIO, SWCLK und GND; der Blue Pill
kann separat versorgt werden. Niemals 5 V an den 3.3-V-Pin des Boards legen.

## Erstinbetriebnahme

1. Motorversorgung ausgeschaltet lassen und alle Masseverbindungen herstellen.
2. Pinbelegung des konkreten IBT-2 und Encoders anhand der Beschriftung bzw. Datenblaetter pruefen.
3. Firmware flashen und pruefen, dass der Motor im Stillstand bleibt.
4. Motorversorgung einschalten und den Antrieb zuerst ohne mechanische Last testen.

Die aktuelle `MotorTask` setzt den PWM-Sollwert dauerhaft auf 0. Der Motor
dreht sich daher erst, wenn die Regel-/Steuerlogik spaeter einen anderen Wert
an `Motor_SetSignedPwm()` uebergibt.
