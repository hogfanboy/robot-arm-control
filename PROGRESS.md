# PROGRESS

## Working state (2026-11-22: project started, nothing built)
- Docs set up: CLAUDE.md, docs/FSD.md (plan with 8 rungs), PROJECT-NOTES.md, IDEAS.md. Photos in media/photos.
- Arm identified: LewanSoul/Hiwonder LeArm 6DOF with an LSC-6 style 6-channel controller V2.0 (STM32, DX-BT24
  Bluetooth LE, PS2 receiver port, `5V RX TX GND` serial header). PS2 handle lost.

## Current target: Rung 0 "Wake-up" (make a servo move over Bluetooth)
Plan A needs no extra hardware: the ESP-NOW Controller's ESP32-C3 connects over BLE to the arm's BT24 module and
sends Hiwonder's serial commands. First prove it from a phone, then from an ESP32.

## Next (the user can answer with one of these)
1. **"Here's the power info"** - the arm's adapter label (volts, amps), and which servo cable goes in which
   numbered port (1-6) for which joint (base, shoulder, elbow, wrist pitch, wrist roll, gripper).
2. **"nRF Connect found it"** - install the free "nRF Connect" app on your phone. Arm ON, open the app, Scan.
   Report the device name that appears (likely BT24 or similar), tap Connect, and say which services show up
   (look for one ending in FFE0 with FFE1 inside).
3. **"The servo moved"** - in nRF Connect tap the up-arrow (write) on FFE1, choose BYTE ARRAY, type
   `5555080301E80301DC05`, Send. Servo 1 should move to center over 1 second. Hand on the power switch!
4. Then Claude writes the first ESP32 probe sketch (flash it on the controller or a spare C3).

## Key decisions
- Plan A: controller talks BLE directly to the BT24 (no extra hardware). The arm "brains" (poses, tasks,
  limits, easing) live in the controller's Arm mode. Car mode stays unchanged.
- Plan B (optional later): ESP32 bridge on the serial header, for tasks with the controller off or a Wi-Fi app.
- Plan C: PCA9685 drives the servos directly if the stock board can't be talked to.
- Safety: limits enforced in the sending code, slow first moves, disconnect = hold pose.
- Repo: public, on the user's own GitHub account.
