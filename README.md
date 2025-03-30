#

## Prerequisites

1. Arduino-IDE should be installed on your system.
2. In `Tools`/`Boards Manager` select and install `Arduino Mbed OS GIGA Boards` package
3. (The pre-set essence of `Arduino_H7_Video V1.0.0` & `Arduino_GigaDisplayTouch V1.0.1` with the display and touch driver is included in the local `libraries` folder of this board-template.)

## Board settings

1. In `Tools` select `Arduino Mbed OS GIGA Boards` / `Arduino Giga R1` as board for the UI or `Arduino Nicla Sense ME` for the send-device.

## Compilation

1. Open the `ui.ino` or the `sense.ino` file in Arduino
2. Go to File/Preferences and set Sketchbook location to the path of your UI project (where this README is located). The setting is retained between Arduino-IDE sessions.
3. Build the project

## Purpose

Project for collecting the current angle of the Nicla Sense ME board and send it through BLE to the Arduino Giga R1 and visualizing it there through the Giga Shield.

