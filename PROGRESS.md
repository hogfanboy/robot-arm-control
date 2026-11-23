# PROGRESS

## Working state (2026-11-22: project started, nothing built)
- Docs set up: CLAUDE.md, docs/FSD.md (plan with 8 rungs), PROJECT-NOTES.md, IDEAS.md. Photos in media/photos.
- Arm identified: LewanSoul/Hiwonder LeArm 6DOF with the 6-channel servo controller V2.0 (BT24 Bluetooth,
  PS2 port, `5V RX TX GND` serial header). PS2 handle lost.

## Current target: Rung 0 "Wake-up" (make the arm move from the PC)
We need to learn the stock board's serial language before writing any bridge or controller code.

## Next (the user can answer with one of these)
1. **"Here's the power info"** - read the arm's power adapter label (volts, amps) and say which servo cable goes
   in which numbered port (1-6) for which joint (base, shoulder, elbow, wrist pitch, wrist roll, gripper).
   A photo of the back of the stock board helps too.
2. **"Header voltage is X"** - with the arm switched ON and nothing plugged into the 4-pin header, put the
   multimeter black probe on GND and red on TX, then RX, then 5V. Tells us if it's safe to connect an ESP32 (3.3 V).
3. **"I have these boards"** - list spare ESP32 boards (C3 SuperMini, DevKit, S3) and whether you have a
   USB-to-serial adapter (CP2102/CH340/FTDI). Then Claude writes the first probe: one servo, small slow move.

## Key decisions
- Plan A: ESP32 "arm bridge" on the stock board's serial header; ESP-NOW Controller gets an Arm mode.
- Plan B: bridge drives servos itself via a PCA9685 if the stock board won't talk. Plan C (no code): a generic
  PS2 wireless handle may pair with the existing receiver.
- The car's `Protocol.h` stays untouched; the arm gets its own `ArmProtocol.h`.
- Safety: limits enforced in the bridge, slow first moves, lost link = hold pose.
