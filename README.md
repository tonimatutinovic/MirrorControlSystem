# MirrorControlSystem

## Project Overview

MirrorControlSystem is an embedded project for controlling a mirror (or anything similar) using a joystick. The Arduino firmware reads joystick positions, applies a dead-zone and movement limits, and smoothly moves servo motors accordingly. The system also supports returning the mirror to a central position on switch press.

## Folder Structure

```
MirrorControlSystem/
├── firmware/
│   └── src/
│       └── MirrorControlSystem/
│           └── MirrorControlSystem.ino
├── LICENSE
└── README.md
```

## Arduino Firmware Features

- Read joystick X/Y inputs
- Dead-zone for static servo positions
- Smooth servo movement with deltaX/deltaY
- Servo boundaries enforced
- Smooth movement to central position on switch press
- Auto-sleep mode after inactivity (servo detach)
- Wake-up on joystick movement or switch press
- EEPROM persistence of last mirror position
- Automatic position restore on power-up
- Serial state logging (ACTIVE / SLEEP)

## Current Status

The firmware now supports non-volatile storage of the mirror position using EEPROM. The last valid position is automatically saved when the system enters auto-sleep and restored on the next power-up or reset.

## Next Steps

- Add serial command interface (INIT / SAVE / DELETE)
- Introduce external control (PC / Python GUI)