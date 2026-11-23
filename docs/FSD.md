# RobotArmControl: Functional Specification and Roadmap

Nov 22, 2026 · Draft 0.1 (planning; nothing built yet)

This is the plan for giving a LeArm 6DOF robot arm a new remote and new abilities. It says what each part
will do, in what order we build it, and how we know a step is done. Unknowns are marked **[?]**. When a
requirement is built and checked on the real arm, mark it ✅ here and copy the facts to `PROJECT-NOTES.md`.

## 1. Goal

Control the arm from the club's **ESP-NOW Controller** (the handheld we built for the ESP NOW CAR), then make it
smart: **teach it a task by moving it, save the task, and replay it** (pick up a block, stack, wave, sort).
Later add a phone/computer web app, smooth motion, "move the hand in a straight line" mode, and a camera.

Ambition ladder (each rung works on its own):

| Rung | Name | What you can do when it's done |
| --- | --- | --- |
| 0 | Wake-up | Arm moves from a PC over USB-serial. We know the stock board's language. |
| 1 | Remote | Drive every joint from the ESP-NOW Controller over Bluetooth. |
| 2 | Teach & Repeat | Save poses, string them into a task, replay it once or in a loop. |
| 3 | Smooth & Safe | Eased motion, speed control, joint limits, soft start, stall/brown-out guard. |
| 4 | Web App | Sliders, task editor, 3D preview, from a phone over Wi-Fi. Shareable task files. |
| 5 | Hand Mode | Joystick moves the gripper in X/Y/Z (inverse kinematics), not one joint at a time. |
| 6 | Eyes | Camera (ESP32-CAM) finds a colored block and the arm picks it up. |
| 7 | Show-off | Two arms mirror each other, dance to music, student coding (Blockly), demo mode. |

## 2. Hardware we have (from the photos, Nov 22 2026)

| Item | What it is | Status |
| --- | --- | --- |
| Arm | LewanSoul/Hiwonder LeArm 6DOF, blue aluminum, rotating base on bearing plates | In hand |
| Servos | 6 PWM servos, several labeled **LDX-218** (17 kg·cm, metal gears); gripper/wrist models **[?]** | In hand |
| Stock controller | Hiwonder 6-channel servo controller V2.0: 6 servo ports, **DX-BT24** BLE module, **PS2** receiver port, `5V RX TX GND` 4-pin header, Run button, buzzer, LED1/LED2, ON/OFF switch, DC jack + screw terminal, crystal marked "8.00" | In hand |
| PS2 receiver | Black dongle on a cable to the PS2 port | In hand |
| PS2 handle | Original wireless remote | **Lost** |
| Power supply | Stock adapter was 7.5 V 3 A (per a retail listing). Replacement: 7.5 V (6-8.4 V ok), 5 A or more, 5.5 x 2.1 mm barrel, center positive **[?]**, or wires into the screw terminal | **Need one** |
| ESP-NOW Controller | ESP32-C3 SuperMini + 1.3" SH1106 OLED, 2-axis joystick + press, Menu/Select/Up/Down buttons, piezo, 1S LiPo | Built (club) |
| Spare ESP32 boards | Optional: a spare C3 for a BLE probe, or Plan B bridge | **[?]** |

## 3. System design

**Plan A (no extra hardware): the controller talks Bluetooth straight to the arm.**

```
 ESP-NOW Controller (ESP32-C3)  ──BLE──►  DX-BT24 module ──UART──►  Stock LSC-6 board (STM32) ──PWM──► 6 servos
   joystick, OLED, buttons,                (on the arm's board)       same serial commands as the
   piezo; saved poses & tasks                                         5V RX TX GND header
   in its own flash; the brains
```

The stock board is a Hiwonder **LSC-6** style controller: an STM32 microcontroller (not an Arduino) that already
knows how to drive the servos. The phone app works by connecting to the BT24 Bluetooth module, which passes
bytes straight through to the STM32's serial port. The ESP32-C3 in our controller has Bluetooth LE built in, so
it can pretend to be the phone app: connect to the BT24 and send the same command bytes (section 4).

- In Arm mode the controller uses BLE instead of ESP-NOW (one radio job at a time keeps it simple).
- The brains (saved poses, tasks, easing, limits) live in the controller, since the stock board can't be
  reprogrammed easily. Tasks replay while the controller is on and connected.
- Same controller, same buttons, no wiring. Car mode is unchanged.

**Plan B (optional later, if we want tasks to run with the controller off or want a web app):** an ESP32
"arm bridge" on the `5V RX TX GND` header, talking ESP-NOW to the controller and serial to the board.

**Plan C (if the stock board can't be talked to at all):** the ESP32 drives the servos itself through a PCA9685
16-channel PWM board (~$3-5).

## 4. Bluetooth link and serial protocol (to be verified in Rung 0)

**Bluetooth (✅ verified 2026-11-22):** advertises as `Hiwonder`; service `FFF0`, characteristic `FFE1`
(notify + write). Bytes written to `FFE1` reach the STM32; its replies come back as notifications.

**Commands:** Hiwonder's LSC-series controllers use this frame at **9600 baud, 8N1** (from Hiwonder's published
LSC protocol; ✅ SERVO_MOVE and GET_BATTERY_VOLTAGE confirmed over BLE). The same bytes work over the header or over Bluetooth:

Hiwonder's LSC-series servo controllers use this frame at **9600 baud, 8N1** (from Hiwonder's published
LSC protocol; **not yet confirmed on our V2.0 board**):

`0x55 0x55 | Length | Command | Parameters...`  (Length = number of bytes after the two header bytes)

| Command | Value | Parameters | Use |
| --- | --- | --- | --- |
| SERVO_MOVE | 3 | count, time_lo, time_hi, then per servo: id, pos_lo, pos_hi | Move 1-6 servos to positions (500-2500 µs) over `time` ms |
| ACTION_GROUP_RUN | 6 | group, times_lo, times_hi | Play a stored action group (built-in moves) |
| ACTION_GROUP_STOP | 7 | none | Stop the action group |
| ACTION_GROUP_SPEED | 11 | group, speed% lo/hi | Speed up / slow down a group |
| GET_BATTERY_VOLTAGE | 15 | none (reply: mV lo/hi) | Supply voltage |

Example, servo 1 to 1500 µs in 1000 ms: `55 55 08 03 01 E8 03 01 DC 05`.

Rung 0 has to answer: the BT24's advertised name and services, whether writing a SERVO_MOVE frame to `FFE1`
moves a servo, whether the board answers GET_BATTERY_VOLTAGE over BLE, and which servo ID is which joint.

## 5. Joints (fill in during Rung 0)

| ID | Joint | Stock-board port | Safe min µs | Safe max µs | Home µs | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | Gripper ✅ | 1 | 1300 | 2450 | 1500 | 1300 = open, 2450 = closed; 2500 is the probe's software cap, not a felt stop |
| 2 | Wrist roll ✅ | 2 | 550 | 2450 | 1500 | both ends were the probe's 500/2500 cap, not a felt stop |
| 3 | Wrist pitch ✅ | 3 | 550 | 2450 | 1500 | both ends were the probe's 500/2500 cap, not a felt stop |
| 4 | Elbow ✅ | 4 | 550 | 2450 | 1500 | both ends were the probe's 500/2500 cap (tested with shoulder at 1500) |
| 5 | Shoulder ✅ | 5 | 550 | 2450 | 1500 | heaviest load; hit the probe's 500/2500 cap; tested with the elbow at 550, so other elbow angles may reach the table **[?]** |
| 6 | Base rotate ✅ | 6 | | | | was unplugged at first |

## 6. Requirements

### 6.1 Arm brains

In Plan A these run **in the ESP-NOW Controller** (the "bridge" below means the controller's arm code); the
ESP-NOW pairing items (AB-2, AB-4 link loss = BLE disconnect) apply over BLE instead. If Plan B is built later,
the same code moves to `firmware/arm_bridge`.

| ID | Requirement | Rung |
| --- | --- | --- |
| AB-1 | Talk to the stock board over UART using the protocol in section 4; one function `moveServos(list, timeMs)`. | 0 |
| AB-2 | Pair with an ESP-NOW Controller using the same 3-step handshake as the car (bind request / ack / confirm), saved in flash. | 1 |
| AB-3 | Accept ARM_JOG packets (joint speeds) every 20-50 ms; turn them into small position steps, clamped to joint limits. | 1 |
| AB-4 | If no packet arrives for 300 ms, stop moving (hold the current pose). Never "snap" to a default pose on link loss. | 1 |
| AB-5 | Send telemetry every 200 ms: 6 joint positions, supply voltage, task state, free task slots. | 1 |
| AB-6 | Per-joint limits and home pose saved in flash; editable from the controller and the web app. | 1 |
| AB-7 | Soft start: on power-up, read nothing it can't know, so move to home slowly (e.g. 3 s) after a warning beep. | 1 |
| AB-8 | Store poses (6 positions) and tasks (list of steps) in flash (LittleFS), at least 20 tasks × 100 steps. | 2 |
| AB-9 | Task steps: MOVE pose over T ms, WAIT T ms, GRIP open/close, BEEP, LOOP N, WAIT FOR BUTTON. | 2 |
| AB-10 | Play a task once, N times or forever; pause, resume, stop; stop instantly from the controller. | 2 |
| AB-11 | Plan B only: run a task with no controller connected (button on the bridge). | 2+ |
| AB-12 | Eased motion: S-curve between poses, all joints arriving together, global speed 10-200%. | 3 |
| AB-13 | Brown-out guard: if supply voltage dips under a threshold, slow down and warn. | 3 |
| AB-14 | Wi-Fi access point + web app (sliders, pose list, task editor, import/export JSON). | 4 |
| AB-15 | Over-the-air firmware update from the web app. | 4 |
| AB-16 | Inverse kinematics for the 4 main joints: target X/Y/Z + gripper angle → joint positions, rejecting unreachable targets. | 5 |
| AB-17 | USB serial command line (`move`, `pose`, `task`, `limits`) for testing and for a Python/PC hookup. | 0-1 |

### 6.2 ESP-NOW Controller "Arm mode" (club repo `ESP_NOW_Controller`)

| ID | Requirement | Rung |
| --- | --- | --- |
| AC-1 | One firmware for car and arm: at pairing, the device says what it is (car or arm); the controller switches its screens and stick mapping to match. | 1 |
| AC-2 | Joint pairs on the stick: X/Y move pair A (base + shoulder), pair B (elbow + wrist pitch), pair C (wrist roll + gripper). Up/Down change pair; OLED shows the pair and a little arm drawing. | 1 |
| AC-3 | Stick press = toggle gripper open/closed. Select (long) = go home slowly. | 1 |
| AC-4 | OLED live view: 6 bars for joint positions, link, arm voltage, controller battery. | 1 |
| AC-5 | Precision mode: hold a button for 1/4 speed. | 1 |
| AC-6 | Teach mode: Select saves the current pose as the next step (beep + step count on screen); Menu opens Save / Play / Loop / Delete for tasks. | 2 |
| AC-7 | Task list screen: pick a task by name/number, play once, loop, stop. Any button press during playback = stop. | 2 |
| AC-8 | Record mode ("puppet"): records the live stick motion at 10 Hz as a smooth path instead of separate poses. | 2 |
| AC-9 | Hand mode (Rung 5): stick moves the gripper left/right/forward/back, Up/Down = up/down. | 5 |
| AC-10 | Existing car pairing, settings and web flasher keep working unchanged. | all |

### 6.3 Radio protocol (Plan B only: `ArmProtocol.h`, identical in controller and bridge)

| ID | Requirement |
| --- | --- |
| AP-1 | Separate file from the car's `Protocol.h`, with its own `ARM_PROTOCOL_VERSION`; same rule: mismatched packets are ignored. |
| AP-2 | Packets: BIND (reuses the car handshake, adds device type), ARM_JOG (6 signed speeds + buttons), ARM_CMD (home, grip, save pose, play/stop task N, speed), ARM_TELEMETRY (6 positions, mV, state, step), ARM_TASK_LIST (names/numbers for the menu). |
| AP-3 | Every packet fits in one ESP-NOW frame (≤ 250 bytes). |

### 6.4 Web app (Rung 4)

| ID | Requirement |
| --- | --- |
| WA-1 | Plan A: a GitHub Pages site using Web Bluetooth (Android Chrome or a computer) talking to the BT24 directly. Plan B: served by the bridge. |
| WA-2 | Six sliders + a 3D stick-figure preview of the arm that matches the real pose. |
| WA-3 | Task editor: list of steps, drag to reorder, edit times, test one step, play from here. |
| WA-4 | Save/load tasks as JSON files so students can share them. |
| WA-5 | Block coding page (Blockly): "move to pose", "open gripper", "repeat 3 times". |

## 7. Safety

- S-1 Joint limits are checked in the code that sends the commands (controller, web app or bridge), so nothing can push
  past them.
- S-2 First move after power-up is slow. New tasks play at 25% speed the first time.
- S-3 Lost link = hold position. Low battery on the controller = warn, then hold.
- S-4 A big red "stop" everywhere: any controller button during a task, a web button, and the stock ON/OFF switch.
- S-5 Keep fingers out of the base and gripper; LDX-218 servos are strong (17 kg·cm).

## 8. Build order and done tests

| Step | Done when |
| --- | --- |
| R0.1 ✅ 7.5 V 3 A adapter found. Still to do: note servo cable → port → joint. | Joint table (section 5) has ports. |
| ✅ R0.2 Phone + nRF Connect: scan with the arm on, note the name and services. | Hiwonder, FFF0/FFE1. |
| ✅ R0.3 In nRF Connect write hex `5555080301E80301DC05` to FFE1 (servo 1 to center over 1 s). | One servo moves from the phone. |
| ✅ R0.4 ESP32-C3 probe sketch (a spare C3 or the controller itself): scan, connect, send the same frame. | Servo moves from the ESP32. |
| R0.5 Find each joint's safe min/max by jogging slowly from the probe's serial monitor. | Section 5 filled in. |
| R1 Controller Arm mode over BLE. | Every joint driven from the handheld; disconnect holds pose. |
| R2 Teach & repeat. | A taught 10-step pick-and-place runs 20 times in a row. |
| R3+ Later rungs. | Per requirement tables. |

## 9. Open questions

1. Does the board answer the LSC protocol over BLE, or is it a different command set? **[?]**
2. Can the board report servo positions? (PWM servos cannot; the controller tracks the last commanded pose.)
3. Does BLE fit next to the controller's existing code on the C3 (flash/RAM)? Expect yes; NimBLE is ~100 KB.
4. Does a generic PS2 wireless handle pair with the existing receiver? (No-code stop-gap.)
5. Decided: public repo on the user's own GitHub account.
