# hardware/

Wiring, schematic and, after the PoC, the custom PCB. v0 is a breadboard
build on an RP2040 dev board.

## Wiring (v0, Pi Pico or RP2040-Zero)

```
HX711 A  DOUT  → GP2        SCK → GP3
HX711 B  DOUT  → GP6        SCK → GP7
OLED     SDA   → GP4        SCL → GP5       (I2C0)
Button   TARE  → GP14 ─┤├─ GND             (internal pull-up)
Power    3V3(OUT) → HX711 VCC/VDD, OLED VCC;   GND common
```

Load cell to HX711: **E+ red, E− black, A− white, A+ green** (the usual
colors, but check your cell, because some swap A+/A− and that just flips the
sign). If tension reads negative, swap A+/A−. `cal` also handles the sign
automatically.

Pins are defined in [`firmware/include/config.h`](../firmware/include/config.h).
All of them exist on both the Pico and the RP2040-Zero.

"A" and "B" above are the two **probes**, each with its own HX711 board. On every HX711
the firmware reads only **input channel A** (pins A+/A−, gain 128). Input B is unused.

### Half-bridge cells (3 wires)

Many spoon-scale cells are half bridges (E+, E−, signal). Complete the bridge with two
matched resistors at the HX711 board:

```
cell E+ (red)    ─┬──────────── HX711 E+
                  R1 1k
                  ├──────────── HX711 A−     mid-point reference
                  R2 1k
cell E− (black)  ─┴──────────── HX711 E−
cell S  (signal) ────────────── HX711 A+
```

- **R1 = R2:** a matched pair (0.1 % / 25 ppm metal film, or neighbours from the same reel).
  Keep them on the HX711 board with short leads.
- **Check with a meter first:** E+ to E− should be about twice one gauge (e.g. ~2 kΩ), and
  E+ to S about the same as S to E− (~1 kΩ each).
- **What you lose:** about half the signal of a full bridge, and more zero drift with temperature.
  Calibrate with `cal`, and tare before each measurement (button).

## Notes

- Run the HX711s from 3.3 V (same logic level as the RP2040). Put 10 µF + 100 nF at each module.
- **Decided:** each HX711 sits **inside its probe head**, a few cm from the load cell.
  The cable to the main unit carries only VCC, GND, DOUT, SCK (GX12/JST connector).
  Put 10 µF + 100 nF at the HX711 end of the cable. Keep cables ≤1 m, because SCK pulses are
  ~1 µs and long cables round them off.
- PCB notes (RP2354A minimal circuit, hand assembly, analog layout) are in [docs/mcu.md](../docs/mcu.md).
