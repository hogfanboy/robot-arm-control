# IDEAS.md (parking lot)

Raw, untested ideas parked so they don't derail the current task. Not settled truth.
At the end of a session: promote what was built and verified to PROJECT-NOTES.md, move a new direction to PROGRESS.md, delete the rest.

## Big features (ambitious, roughly in order; the plan for each is in docs/FSD.md)
1. **Teach & Repeat**: save poses with the handheld, chain them into tasks, replay/loop.
2. **Puppet recording**: record live stick motion as a smooth path.
3. **Web app**: sliders, 3D preview, task editor, JSON sharing, Blockly for students.
4. **Hand mode (inverse kinematics)**: joystick moves the gripper in X/Y/Z.
5. **Camera (ESP32-CAM)**: find a colored block on the table and pick it up; sort blocks by color into cups.
6. **Mirror mode**: a second arm (or a cardboard "master arm" with potentiometers) copies the first.

## Fun demos
- Dance to music (beat from the controller's piezo song list); wave hello when someone walks up (distance sensor).
- Stack 3 blocks; pour a cup of beads; ring a bell; play tic-tac-toe with a marker against a person.
- "Attract mode" for club open house: loops a show task until someone touches the controller.

## Hardware upgrades
- Current sensor (INA219) on the servo supply: detect a jammed/stalled joint and stop.
- Limit/home switches or potentiometer feedback hack for true position readback.
- Big physical e-stop button wired to the bridge.

## Parking Lot
- Reuse the phone's Bluetooth: BT24 is BLE; a web page using Web Bluetooth (Android Chrome) might drive the stock board directly with no bridge.
- Mustang Way Robots web flasher for the arm bridge, same `build.yml` / `flash/index.html` as the car and controller.
