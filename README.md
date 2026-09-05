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

## Actual operation video

[YouTube demonstration video](https://youtu.be/bXDWdd7FUCw)

## Hardware wiring

The project uses a **NUCLEO-F411RE** development board and an **EZ Motor R300** module.

![NUCLEO-F411RE and EZ Motor R300 wiring](docs/nucleo-f411re-ez-r300-wiring.svg)

### Pin mapping used by the STM32 code

| Function | NUCLEO-F411RE Arduino pin | STM32 GPIO | R300 connection |
|---|---|---|---|
| Push button | A0 | PA0 | Button input |
| Direction 1 | A1 | PA1 | IN1 |
| Direction 2 | A2 | PA4 | IN2 |
| Speed PWM | A3 | PB0 / TIM3_CH3 | ENA |
| UART transmit | D1 | PA2 / USART2_TX | USB-UART RX |
| UART receive | D0 | PA3 / USART2_RX | USB-UART TX |
| Ground | GND | GND | R300 GND and external power GND |

Connect the R300 motor output to the DC motor. Supply the motor from an external power source appropriate for the motor and connect all grounds together. Do not power the motor directly from the Nucleo board.

> The diagram follows the GPIO assignments in `stm32/main.c`. Confirm the exact R300 terminal labels and motor supply voltage printed on the module before wiring.

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

## Source folders

- `qt/` — Qt Widgets UI and UART communication
- `stm32/` — STM32 GPIO, PWM, UART, button, and motor-control code
