# Warnhinweise und Anschlussplan

## Anschlussplan

**Aktuelle Pinbelegung:** IBT-2: PA0 = RPWM, PA1 = LPWM, PB0 = R_EN,
PB1 = L_EN. Motor-Encoder: PA8 = Kanal A und PA9 = Kanal B.

```text
       12-V-Netzteil
       +12 V ───────────────┬────────────── IBT-2 B+
                            │
                            └── L7805CV IN (Pin 1)
                                L7805CV OUT (Pin 3) ── +5 V ── IBT-2 VCC
                                L7805CV GND (Pin 2) ── GND ─── IBT-2 GND
       0 V / Minus ─────────┴───────────────────────── IBT-2 B-
                            └────────────────────────── Blue Pill GND

       L7805CV: 330 nF von IN nach GND, 100 nF von OUT nach GND,
                 nahe am Regler platzieren.

       IBT-2                         Blue Pill (STM32F103C8T6)
       RPWM  ─────────────────────── PA8 / TIM1_CH1
       LPWM  ─────────────────────── PA9 / TIM1_CH2
       R_EN  ─────────────────────── PB0
       L_EN  ─────────────────────── PB1
       GND   ─────────────────────── GND (gemeinsame Masse)
       R_IS, L_IS: unbeschaltet lassen

       IBT-2                         JGB37-520 Encoder-Motor
       M+    ─────────────────────── Motor Rot
       M-    ─────────────────────── Motor Weiß

       Motor Encoder                 Blue Pill
       Blau (+3,3 V) ─────────────── 3V3
       Schwarz (GND) ─────────────── GND
       Gelb (Signal A) ───────────── PA6 / TIM3_CH1
       Grün (Signal B) ───────────── PA7 / TIM3_CH2
```

## Wichtige Hinweise

**Aktuelle Belegung fuer die Hardware-Anordnung:** IBT-2 = PA0 (RPWM), PA1 (LPWM), PB0 (R_EN), PB1 (L_EN). Motor-Encoder = PA8 (Kanal A) und PA9 (Kanal B), gelesen mit TIM1.

- **B+ und B-** sind die 12-V-Motoreingänge des IBT-2. **M+ und M-** gehen zum Motor.
- **IBT-2 VCC muss angeschlossen werden:** VCC benötigt die geregelten +5 V für die Logik des Treibers. Ohne VCC funktionieren RPWM, LPWM, R_EN und L_EN nicht zuverlässig. **Niemals 12 V an VCC anschließen.**
- Encoder Blau/Schwarz sind die Encoder-Versorgung (3,3-5 V laut Anschlussbild); hier ist 3,3 V gewählt, damit A/B-Signale zur STM32-Logik passen.
- L7805CV ist ein Abwärtsregler: Er erzeugt 5 V aus einer höheren Eingangsspannung, nicht 12 V aus 5 V. Pinbelegung in TO-220, Beschriftung nach vorn: 1=IN, 2=GND, 3=OUT. Kühlung kann bei nennenswertem Strom erforderlich sein.
- Verbinde die Masse des 12-V-Netzteils, des IBT-2, des Reglers und des Blue Pill miteinander. Prüfe die tatsächliche Pinbeschriftung des verwendeten IBT-2-Moduls vor dem Einschalten.
- Der Motor zieht laut gezeigter 12-V/333-RPM-Tabelle etwa 1 A unter Nennlast und bis zu 2,3 A bei blockierter Welle. Blockieren vermeiden und Netzteil/Leitungen entsprechend auslegen.
- Im aktuellen Firmwarestand wird PWM auf 0 gesetzt, der Motor sollte also nicht anlaufen. PB0/PB1 werden beim Start auf HIGH gesetzt (Treiber-Enable); Motorversorgung erst nach Prüfung der Verdrahtung einschalten.
