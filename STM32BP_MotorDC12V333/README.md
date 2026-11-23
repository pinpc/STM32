# STM32BP_MotorDC12V333

HAL + FreeRTOS starter project for an STM32F103C8T6 Blue Pill, an IBT-2/BTS7960 H-bridge and a JGB37-520 encoder motor.

## Initial pin plan

| Function | STM32 pin |
|---|---|
| IBT-2 RPWM | PA0 / TIM2_CH1 |
| IBT-2 LPWM | PA1 / TIM2_CH2 |
| IBT-2 R_EN | PB0 |
| IBT-2 L_EN | PB1 |
| Encoder C1 | PA6 / TIM3_CH1 |
| Encoder C2 | PA7 / TIM3_CH2 |

The motor is deliberately stopped at startup. Verify the motor and encoder wiring before replacing `Motor_SetSignedPwm(0)` in `Core/Src/main.c` with a non-zero command.

## Build

```powershell
.\build.ps1
```

Open this folder in VS Code and start `STM32BP Motor – ST-Link Debug` with F5.
