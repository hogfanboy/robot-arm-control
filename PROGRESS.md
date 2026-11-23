# PROGRESS

## Working state (2026-11-22: project started, nothing built)
- Docs set up: CLAUDE.md, docs/FSD.md (plan with 8 rungs), PROJECT-NOTES.md, IDEAS.md. Photos in media/photos.
- Arm identified: LewanSoul/Hiwonder LeArm 6DOF with an LSC-6 style 6-channel controller V2.0 (STM32, DX-BT24
  Bluetooth LE, PS2 receiver port, `5V RX TX GND` serial header). PS2 handle lost.

## Current target: Rung 0 "Wake-up" - find each joint's safe range from the ESP32
- ✅ Phone (nRF Connect) and ✅ ESP32-C3 SuperMini (COM14, `firmware/probes/ble_probe`) both talk to the arm over
  BLE: "Hiwonder", service FFF0, char FFE1. Battery read 7.75 V. All 6 joints mapped (PROJECT-NOTES.md).
- 2026-11-22: `c` checked: arm was already straight up (all 1500), so nothing moved; that is correct.
  Type ONE command per line (`j 1 -100 / j 1 100` only ran the first part).
- 2026-11-22: ✅ Servo 6 (base) turns with `m 6 1200 2000` / `m 6 1500 2000`. Its plug had come out of port 6.
- 2026-11-22: Claw (1) safe range 1300-2450 (backed off 50 from each end). 1300 = open, 2450 = closed.
- 2026-11-22: Wrist roll (2), wrist pitch (3), elbow (4), shoulder (5) safe range 550-2450
  (each alone, ends at the probe's 500/2500 cap). Shoulder was tested with the elbow at 550: some shoulder +
  elbow combinations may still reach the table, so the Arm mode needs a pose check, not only per-joint limits.
- Serial Monitor without the IDE: `arduino-cli monitor -p COM14 -c baudrate=115200` (Ctrl+C to quit; close it
  before uploading). Typed letters may not echo.
- Probe commands in the Serial Monitor: `m ID POS [MS]`, `j ID STEP`, `c`, `b`, `t MS`, `p`, `raw`.

## Next (the user can answer with one of these)
1. **"Limits: ..."** - for each joint, jog with `j ID 100` / `j ID -100` until it nears a hard stop or hits the
   table, back off 50, and note the number (`p` shows it). Fill the joint table in docs/FSD.md section 5.
2. Then Rung 1: Arm mode in the ESP-NOW Controller (stick drives joints over BLE).

## Key decisions
- Plan A: controller talks BLE directly to the BT24 (no extra hardware). The arm "brains" (poses, tasks,
  limits, easing) live in the controller's Arm mode. Car mode stays unchanged.
- Plan B (optional later): ESP32 bridge on the serial header, for tasks with the controller off or a Wi-Fi app.
- Plan C: PCA9685 drives the servos directly if the stock board can't be talked to.
- Safety: limits enforced in the sending code, slow first moves, disconnect = hold pose.
- Repo: public, on the user's own GitHub account.
