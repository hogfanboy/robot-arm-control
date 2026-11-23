# RobotArmControl: project notes (settled truth)

Verified facts, decisions and gotchas. Add only things that are built and verified on the real arm.
The plan is in docs/FSD.md; current work is in PROGRESS.md; untested ideas are in IDEAS.md. Last updated 2026-11-22.

## What it is
A LewanSoul/Hiwonder LeArm 6DOF desktop arm (Amazon ASIN B074T6DPKX), bought several years ago. The original PS2
wireless handle is lost; the PS2 receiver dongle is still on its cable.

## Hardware facts (from photos, 2026-11-22)
- Stock controller silkscreen: "V2.0" (Hiwonder LSC-6 style; STM32 per Hiwonder listings, chip not yet seen). Parts seen: DX-BT24 BLE module (FCC ID 2AKS8DX-BT24), PS2 port (pins
  labeled 3 6 9), servo ports with numbers 1-5 visible (6 assumed), 4-pin header labeled `5U RX TX GND`, "Run"
  push button, piezo buzzer, LED1/LED2, OFF/ON slide switch, DC barrel jack and a 2-pin screw terminal (+ -),
  220 µF 16 V capacitor (so the supply is under 16 V), crystal marked "8.00".
- Servos: several labeled LDX-218.

## Decisions
- Keep the stock controller. Plan A: the ESP-NOW Controller connects over BLE to the BT24 and sends Hiwonder serial commands (no extra hardware). Plan B: ESP32 bridge on the serial header. Plan C: PCA9685 direct drive.
- The car's `Protocol.h` stays untouched by arm work.

## Gotchas
(none yet)
