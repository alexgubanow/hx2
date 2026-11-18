# BOM (v0, first draft)

Prices are rough single-unit guides (AliExpress / local electronics shops, 2026).
Verify locally.

## PoC build (chosen, bought locally in LT)

| Qty | Part | Source | Price | Notes |
|---|---|---|---|---|
| 1 | Raspberry Pi Pico (RP2040, 2 MB flash, micro-USB) | [rcl.lt AKOMRPIPI](https://rcl.lt/products/akomrpipi-raspberry-pi-pico-rp2040-arm-cortex-m0) | 7.50 € | headers probably not soldered. Needs a micro-USB **data** cable |
| 2 (+1 spare) | HX711 module, ~20×16 mm, pins VCC/GND/DT/SCK | [rcl.lt ABA04](https://rcl.lt/products/aba04-svorio-jutiklio-modulis-hx711) | 2.20 € ea (1.96 € @4+) | **module only, no load cell** |
| 2 | Load cell, **salvaged from digital spoon scales** | own stock | — | capacity and output unknown, see checks below |
| 1 | 0.96" 128×64 OLED | own stock | — | almost certainly SSD1306. Must be the **4-pin I²C** version |
| 1 | tactile button, breadboard, jumpers, 2×20 headers | own stock | — | |

**HX711 placement:** one HX711 board **inside each probe head**, right at the
load cell. Only digital lines (VCC, GND, DT, SCK) run down the cable, so the
µV-level bridge signal never leaves the probe.

**Check on each spoon-scale cell before building around it:**
- **Full bridge (4 wires) or half bridge (3 wires)?** The HX711 needs a full bridge. A 3-wire half
  bridge can be completed with two matched resistors (e.g. 1 kΩ 0.1 %), but drift gets worse.
- **Bridge resistance:** measure E+ to E− and A+ to A−, typically ~1 kΩ. This also identifies the wires.
- **Capacity:** spoon scales are usually 300–500 g. The design point needs ~130–300 g at
  10–25 N, so a 300 g cell needs a **mechanical overload stop** on the beam. Read the label on the
  cell or the scale's max rating.
- **Mounting:** sketch the hole positions and threads. The probe head gets designed around them.

**Check on each HX711 when it arrives:**
- **RATE**: the listing says "10 Hz or 80 Hz", depending on the RATE pin (chip pin 15). Confirm with a multimeter that it goes to GND, which is 10 SPS. If it reads 80 SPS, the noise will be much worse.
- **E− to GND continuity.** Some cheap clones leave E− floating. If they aren't connected, bridge them.

## Electronics (generic)

| # | Qty | Part | Example / spec | ~Cost | Notes |
|---|---|---|---|---|---|
| 1 | 1 | MCU | RP2040 dev board: Raspberry Pi Pico or Waveshare RP2040-Zero | $3–5 | see [mcu.md](mcu.md) |
| 2 | 2 | Load cell | 1 kg aluminium bar cell, ~80×12.7×12.7 mm, 4-wire | $1–3 ea | often sold as a kit with an HX711 |
| 3 | 2 | ADC | HX711 module (green or purple) | $0.5–1.5 ea | check the RATE pin is at 10 SPS |
| 4 | 1 | Display | 0.96" SSD1306 128×64 I²C OLED, **white or green** | $2–3 | 1.3" SH1106 if you want bigger digits (one-line change) |
| 5 | 1 | Button | 6×6 tactile switch | <$0.2 | TARE (short) / HOLD (long), on GP14 |
| 6 | 2 | Probe connector | GX12 4-pin aviation pair **or** JST-XH 4-pin | $0.5–1.5 | makes probes replaceable |
| 7 | — | Wire | 4-core shielded or twisted cable, 0.5–1 m per probe | $1 | shield to GND at the HX711 end |
| 8 | — | Misc | perfboard, headers, heat-shrink | $1 | |

**Electronics total: about $10–18.**

### Later (v1 handheld; goes on the custom PCB)

| Part | Example | ~Cost |
|---|---|---|
| 1S LiPo 500–1000 mAh or 18650 + holder | — | $2–4 |
| USB-C charger with protection | TP4056 USB-C module (protected version) | $0.5–1 |
| Slide switch | SS12D00 | $0.1 |
| Battery divider | 2× 100 k resistors to an ADC pin | — |

## Probe v1 (per probe, see [cad/](../cad/))

| Qty | Part | Notes |
|---|---|---|
| 1 | Printed frame | `hx2-probe-v1-frame.stl`. **PETG/ASA/ABS, not PLA** (PLA creeps under steady load) |
| 1 | Printed nose sleeve | `hx2-probe-v1-nose.stl`, same material |
| — | Fit-test coupon (optional) | `hx2-probe-v1-coupon-posts.stl`, about 2 g. Print it first to check the posts, rib and inserts against your cell |
| 6 | MR105ZZ bearing (5×10×4) | 3 per post. Wide enough to bridge several GT2 teeth |
| 2 | Axle tube, 12 mm long, OD 4.9–5.0 / ID 3.2 | Brass/steel tube (best) or print `hx2-probe-v1-spacer.stl` |
| 2 | M3×25 countersunk (DIN 7991) | Bearing axles. Self-tap into the frame |
| 2 | M3×6 countersunk (DIN 7991) | Nose sleeve into the cell's thin-end threads |
| 2 | M3 heat-set insert, short (≈3 mm, Ø4 hole) | Roof, holds the cell |
| 2 | M3 screw, ≈M3×10–12 | Cell thick end into the inserts, from below (the cell sits on the post rib) |
| 2 | M3×6 self-tapping (or M3 screw into the Ø2.5 pilot) | HX711 board onto its standoffs in the bay |
| 1 | Load cell | Spoon-scale cell (PoC), flipped, thin-end step towards the belt |
| 1 | HX711 module | Inside the probe head |
| 2 | 1 kΩ resistor, matched (0.1 % / 25 ppm or same reel) | **Only for a 3-wire half-bridge cell**, completes the bridge (see [hardware/README.md](../hardware/README.md)) |

## Calibration kit

| Item | Why |
|---|---|
| Known masses | US nickel 5.000 g, or water weighed on a kitchen scale, for stage 1 |
| Calibration rig | Belt span over two bearings plus 0.5/1/1.5 kg hanging masses, for stage 2. See [measurement.md](measurement.md) |

## About "spoon scale" donors

Cheap spoon/kitchen scales are tempting but often **not** a shortcut:

- Most use a **chip-on-board ADC** (a black blob under epoxy) driving the LCD directly.
  There's often **no HX711 to reuse**, and the blob can't be read from outside.
- Their load cells are usually small, odd-shaped beams (300–500 g range) with no
  mounting holes you can design around, and they differ between batches. That makes a
  replaceable probe design hard to repeat.
- A new **1 kg bar cell plus HX711 kit costs about the same as a spoon scale** ($2–4).

They're still fine as a source of cells for experiments. For a design other
people can rebuild, **specify the standard bar cell + HX711 kit**.
