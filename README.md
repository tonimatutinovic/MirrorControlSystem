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

## Next Steps

- Implement auto-sleep for servos after inactivity
- Wake-up logic on joystick movement or switch press
