# PROGRESS

## Working state (2026-11-22: project started, nothing built)
- Docs set up: CLAUDE.md, docs/FSD.md (plan with 8 rungs), PROJECT-NOTES.md, IDEAS.md. Photos in media/photos.
- Arm identified: LewanSoul/Hiwonder LeArm 6DOF with an LSC-6 style 6-channel controller V2.0 (STM32, DX-BT24
  Bluetooth LE, PS2 receiver port, `5V RX TX GND` serial header). PS2 handle lost.

## Current target: Rung 0 "Wake-up" - map the joints, then move from an ESP32
- ✅ Phone (nRF Connect) talks to the arm: "Hiwonder", service FFF0, characteristic FFE1. SERVO_MOVE moved
  servo 1 (claw); battery read 7707 mV. Details in PROJECT-NOTES.md.

## Next (the user can answer with one of these)
1. **"Joint map: 1=..., 2=..."** - in nRF Connect write each line to FFE1, one at a time, and say which joint
   moved (each sends servo N to center, 1500 us, over 2 s; all-center = arm standing straight up):
   `5555080301D00701DC05` (1) `5555080301D00702DC05` (2) `5555080301D00703DC05` (3)
   `5555080301D00704DC05` (4) `5555080301D00705DC05` (5) `5555080301D00706DC05` (6)
2. **"Write the ESP32 probe"** - Claude writes `firmware/probes/ble_probe`: the ESP32-C3 finds "Hiwonder",
   connects, and moves joints from serial-monitor commands. Say which board to flash (the controller itself or a
   spare C3).

## Key decisions
- Plan A: controller talks BLE directly to the BT24 (no extra hardware). The arm "brains" (poses, tasks,
  limits, easing) live in the controller's Arm mode. Car mode stays unchanged.
- Plan B (optional later): ESP32 bridge on the serial header, for tasks with the controller off or a Wi-Fi app.
- Plan C: PCA9685 drives the servos directly if the stock board can't be talked to.
- Safety: limits enforced in the sending code, slow first moves, disconnect = hold pose.
- Repo: public, on the user's own GitHub account.
