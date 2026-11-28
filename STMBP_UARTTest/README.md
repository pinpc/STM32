# Blue Pill UART-Test

## Anschluss

| Blue Pill | USB-TTL-Adapter |
|---|---|
| PA2 / USART2_TX | RX |
| PA3 / USART2_RX | TX |
| GND | GND |

Der Adapter muss mit **3,3 V Pegel** arbeiten. 5-V-TTL kann den STM32 beschädigen.

## Flashen mit ST-LINK V2

ST-LINK V2 wird über SWD angeschlossen:

| ST-LINK V2 | Blue Pill |
|---|---|
| SWDIO | PA13 / SWDIO |
| SWCLK | PA14 / SWCLK |
| GND | GND |
| 3.3V | 3.3V (nur bei Versorgung über ST-LINK) |
| NRST | NRST (optional, empfohlen) |

In PlatformIO ist `upload_protocol = stlink` bereits eingestellt. Nach Anschluss des
ST-LINK V2 flashen mit:

```text
pio run -t upload
```

Oder in VS Code: PlatformIO-Seitenleiste -> Project Tasks -> bluepill_f103c8 -> Upload.
Für den UART-Test danach PA9/PA10 mit einem 3,3-V-USB-TTL-Adapter verbinden.

## Test

Seriellen Monitor mit **115200 Baud, 8 Datenbits, keine Parität, 1 Stopbit** öffnen.
Die aktuelle Firmware sendet nach dem Start `UART READY - send PING` und danach jede
Sekunde `CYCLIC TEST`.

Für den `PING`/`PONG`-Test `ENABLE_UART_RESPONSE` in `src/main.c` auf `1U` setzen und
neu flashen. Dann `PING` mit Zeilenende senden; das Board antwortet mit `PONG` und
blinkt die Onboard-LED auf PC13.

Ein UART kann keine eingesteckte PC-Verbindung elektrisch erkennen. Der `PING`/`PONG`-
Handshake dient deshalb als Verbindungsnachweis.

