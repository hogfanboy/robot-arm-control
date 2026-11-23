# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

RobotArmControl: new brains and a new remote for a LewanSoul/Hiwonder **LeArm 6DOF** robot arm (Amazon B074T6DPKX,
blue aluminum, 6 PWM servos, stock 6-channel servo controller with DX-BT24 Bluetooth and a PS2 receiver port).
The original PS2 handle is lost. Plan: drive the arm from the club's **ESP-NOW Controller** (ESP32-C3, OLED, joystick,
5 buttons) through a small ESP32 "arm bridge", then add teach-and-repeat, a web app and smarter motion.
No firmware written yet. If this becomes a public GitHub repo: keep personal and school details out of committed files.

**Read [PROGRESS.md](PROGRESS.md) first.** [docs/FSD.md](docs/FSD.md) is the plan (what we will build, phases,
requirements). [PROJECT-NOTES.md](PROJECT-NOTES.md) is settled truth (only things checked on the real arm); read only
the part you need.

## Docs (one job each)
- `CLAUDE.md`: permanent rules and commands only. No brainstorming.
- `PROGRESS.md`: current target, what just broke, next step. If an idea changes direction, replace the old target.
- `docs/FSD.md`: the functional spec and roadmap (planned behavior, requirement IDs, open questions marked **[?]**).
  When a requirement is built and verified, mark it done there and copy the facts to PROJECT-NOTES.md.
- `PROJECT-NOTES.md`: settled truth (verified hardware facts, pins, protocol bytes that worked, decisions, gotchas).
  Add only things that are built and verified; no history or failed attempts.
- `IDEAS.md`: backlog and parking lot for untested ideas. Don't read it unless asked or when picking the next task.
  When the user says "park this", append one line under "Parking Lot" and go back to the current task without
  discussing it. At session end, if asked, promote verified items to PROJECT-NOTES.md and remove them.

## Rules
- The user is a self-described beginner with Arduino code and toolchain settings: explain in plain-language steps.
  The user does the hardware (wiring, multimeter, plugging in); Claude writes the code and says exactly what to check.
- Safety first with the arm: every new motion feature starts slow, with joint limits on, and the user near the power
  switch. Never send a full-speed move to an untested position. Measure voltages before wiring two boards together.
- The stock controller board is the only one we have: probe it read-only first (listen before sending), no
  reflashing it unless the user agrees.
- The ESP-NOW Controller firmware lives in the club repo `Documents\Arduino\ESP_NOW_Controller` (sister car repo
  `ESP_NOW_CAR`). Its `Protocol.h` must stay identical to the car's. Arm support goes in a separate `ArmProtocol.h`
  shared byte-for-byte between the controller and `firmware/arm_bridge`. Don't push to the club's `main` without asking.
- Don't read `firmware/*/build/`, `~/.arduino15`, Arduino `libraries/` or `media/` unless asked.
- Never commit `media/` (photos, videos). Commit as hogfanboy (noreply), never the Gmail.
- After any feature or bug fix, update PROGRESS.md (under 60 lines): working state, active bug or next task, key
  decisions (no failed attempts). Then say: "Task complete and PROGRESS.md updated. Ready for /clear."

## Layout
- `docs/FSD.md`: spec and roadmap. `docs/` may later hold the GitHub Pages site / web flasher / web app.
- `firmware/arm_bridge/`: ESP32 that plugs into the arm controller's `5V RX TX GND` header (not started).
- `firmware/probes/`: one-off tests (UART sniffer, servo sweep).
- `tools/`: PC-side helpers (serial probe scripts).
- `media/`: photos and videos (git-ignored).

## Build (Arduino CLI; board settings TBD per board)
```bash
A="/c/Users/hogfa/AppData/Local/Programs/arduino-cli/arduino-cli.exe"   # not on PATH
```
ESP-NOW Controller build command: see its own CLAUDE.md (esp32 core 3.3.12 in CI, 3.3.13 on this PC).
