# GasSeeker visual wiring files

These files are the visual companion to `docs/WIRING.md`.

## Files
- `esp32-pinout.svg` — board pin map based on your exact pink ESP32-S3 board labels.
- `power-wiring.svg` — battery, fuse, switch, buck converters, power rails and ground.
- `signal-wiring.svg` — GPIO-to-module signal connections.

## Suggested usage order
1. Open `esp32-pinout.svg` to find the physical pins on the board.
2. Open `power-wiring.svg` and finish only the power wiring first.
3. Open `signal-wiring.svg` and connect only GPIO / signal wires after power is stable.

## Important
- These files are aligned with the current `WIRING.md` assumptions:
  - 4 motors, 2 TB6612
  - 1 encoder HC-020K on GPIO1
  - MPU6050 on GPIO8/9
  - MQ-3 on GPIO4 through the 10k/20k divider
  - 1 bumper on GPIO38
