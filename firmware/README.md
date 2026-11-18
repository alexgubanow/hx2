# firmware/

PlatformIO + the Arduino-Pico (earlephilhower) core, targeting the RP2040.

```
pio run -t upload       # first time: hold BOOTSEL while plugging in USB
pio device monitor
```

For an RP2040-Zero, set `board = waveshare_rp2040_zero` in `platformio.ini`.
No pin changes are needed.

| File | What |
|---|---|
| `include/config.h` | pins per board, filter and stability constants, default fixture geometry |
| `src/hx711.*` | minimal non-blocking HX711 driver (interrupt-safe bit-bang) |
| `src/channel.*` | one probe: moving window, tare, calibration, force → tension |
| `src/storage.*` | versioned, CRC-checked calibration in flash |
| `src/display.*` | 128×64 OLED: A / B / Δ |
| `src/main.cpp` | loop, button, serial console |

## Display

```
A   14.82 N  =      '=' stable  '~' settling  'H' held
B   15.61 N  =
────────────────
Δ   -0.79    -5%    Δ = A − B, % of the mean
```

## Serial console (115200)

```
tare                  zero both (probes unloaded)
cal a 200             200 g resting on probe A's nose (probe upside down, nose up)
rig a 9.81            probe A clamped on the rig belt, 1 kg hanging
fix a 80 2.5          probe A geometry s=80 mm, d=2.5 mm
show | save | load
stream                CSV for creep/noise logging
```

Button: **short press = tare**, **long press = hold** (freeze the display).

## Status

Skeleton. **Not compiled or run on hardware yet.**
