# MirrorControlSystem

## Project Overview

MirrorControlSystem is an embedded project for controlling a mirror (or anything similar) using a joystick. The Arduino firmware reads joystick positions, applies a dead-zone and movement limits, and smoothly moves servo motors accordingly. The system also supports returning the mirror to a central position or a user-selected position on command..

## Folder Structure

```
MirrorControlSystem/
├── firmware/
│   └── src/
│       └── MirrorControlSystem/
│           └── MirrorControlSystem.ino
├── python/
│   ├── mirrorGUI.py
│   └── users.json
├── LICENSE
└── README.md

```

## Arduino Firmware Features

- Read joystick X/Y inputs
- Dead-zone for static servo positions
- Smooth servo movement with deltaX/deltaY
- Servo boundaries enforced
- Smooth movement to central or user-selected position
- Auto-sleep mode after inactivity (servo detach)
- Wake-up on joystick movement or switch press
- EEPROM persistence of last mirror position
- Automatic position restore on power-up
- Serial communication with Python GUI (INIT / SAVE / DELETE)

---

### Python GUI Features

- User management (create, switch, delete)
- Real-time mirror position monitoring
- Sends commands to Arduino to update or save positions

---

## Current Status

- Arduino firmware now supports serial commands from the Python GUI.
- Mirror positions can be controlled either manually with the joystick or via the GUI.
- Last mirror position is saved in EEPROM and restored on startup.

---
## Next Steps

- Refactor moveToCentralPosition → moveToTarget for more general movement logic.
- Add further GUI controls and optimizations.
- Enhance serial command interface for more advanced features.

---
