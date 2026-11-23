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
- Power: original adapter found, 7.5 V 3 A.
- Bluetooth: the board advertises as "Hiwonder" (nRF Connect, 2026-11-22). Service **0xFFF0** (not FFE0), with
  characteristic **0xFFE1** (NOTIFY, READ, WRITE, WRITE NO RESPONSE; notifications carry the board's replies) and a
  second write-only characteristic (UUID not read yet). Also has Device Information (0x180A).

## Command protocol (verified over BLE from a phone, 2026-11-22)
Hiwonder LSC frame `55 55 <len> <cmd> <params>`, written to 0xFFE1; len = bytes after the 55 55.
- SERVO_MOVE (3): `55 55 08 03 01 E8 03 01 DC 05` moved servo 1 (sounded like the claw) to 1500 us over 1000 ms.
- GET_BATTERY_VOLTAGE (15): `55 55 02 0F` -> reply `55 55 04 0F 1B 1E` = 0x1E1B = 7707 mV (7.5 V adapter).
- ESP32-C3 SuperMini as BLE client (`firmware/probes/ble_probe`, core 3.3.13 built-in BLE library, no pairing or
  PIN needed): finds "Hiwonder" by name, connects, writes FFE1 without response, gets replies by notify. Battery
  read 7750 mV. Signal -50 dBm at desk range. The phone must be disconnected first (one connection at a time).

## Joint map (verified 2026-11-22, each servo sent to 1500 us)
| ID | Joint |
| --- | --- |
| 1 | Claw (gripper) |
| 2 | Wrist spin (turns like a doorknob) |
| 3 | Next joint down (wrist pitch) |
| 4 | Next joint down (elbow) |
| 5 | Shoulder (on the base, not the rotation) |
| 6 | Base rotate (moved 1500 -> 1200 -> 1500 once its plug was put back in port 6) |
All of 1-5 at 1500 = arm standing straight up.

## Decisions
- Keep the stock controller. Plan A: the ESP-NOW Controller connects over BLE to the BT24 and sends Hiwonder serial commands (no extra hardware). Plan B: ESP32 bridge on the serial header. Plan C: PCA9685 direct drive.
- The car's `Protocol.h` stays untouched by arm work.

## Gotchas
(none yet)
