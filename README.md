# Qt Hairdryer Motor Control

Qt Widgets application for controlling an STM32 motor through UART.

## Features

- Hairdryer-shaped Qt Widgets UI
- Motor ON/OFF control
- 0–100% motor speed slider
- Wind icon emphasis changes with motor speed
- STM32 motor state and speed feedback
- UART serial communication at 115200 baud
- STM32 source for PWM/UART motor control

## Qt build

Open `qt/MotorControl.pro` with Qt Creator.

Required Qt modules:

- Qt Widgets
- Qt SerialPort
- C++17

The project currently creates the UI in C++ code, so no separate `.ui` file is required.

## UART commands

Qt sends newline-terminated commands:

```
ON
OFF
SPEED:0
SPEED:50
SPEED:100
```

STM32 can return messages such as:

```
MOTOR:ON
MOTOR:OFF
SPEED:50
```

## Hardware

STM32 → motor driver PWM/control input → DC motor

Use a separate motor power supply and connect the STM32 and motor-driver grounds.
