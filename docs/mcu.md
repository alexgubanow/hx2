# MCU

**Decision:**

- **Proof of concept:** RP2040 (Raspberry Pi Pico).
- **Custom PCB:** **RP2354A**, the RP2350A with 2 MB of flash stacked inside the package. Raspberry Pi SC1511. As of 2026-11, DigiKey has it at €1.02 for one and €0.89 on a 3,400-unit reel.
- **Assembly:** the PCB is hand-assembled, so parts can come from DigiKey as well as LCSC.

## Why RP2354A

| Need | RP2354A |
|---|---|
| **Same firmware as the PoC** | Arduino-Pico core, board `rpipico2` or a generic RP2350 target. The HX711, U8g2 and storage code is unchanged |
| **No external flash or EEPROM** | 2 MB flash in the package holds the firmware and the calibration (EEPROM emulation in the last sector, versioned struct with CRC) |
| **USB** | USB 1.1 device. UF2 drag-and-drop bootloader in ROM, so no programmer is needed. CDC serial console for calibration and CSV logging |
| **Headroom** | 2× Cortex-M33 at 150 MHz (or the Hazard3 RISC-V cores), 520 KB SRAM, 30 GPIO (needs about 9) |
| **Battery** | Much better low-power modes than the RP2040, and a switching core regulator |
| **Cost** | ≈ €1 for the chip, a crystal and an inductor. Cheaper than any crystal-less-USB STM32 with similar headroom |

### Parts it still needs

Copy Raspberry Pi's *Hardware design with RP2350* minimal design, RP2354 variant:

- **Crystal:** 12 MHz with its load capacitors. USB needs an accurate clock, which the internal ring oscillator isn't.
- **Core regulator:** a switching regulator, which needs the specified inductor plus input and output capacitors. Use exactly the inductor and layout from the reference design.
- **3.3 V supply:** an LDO, plus decoupling on each IOVDD pin and USB_OTP_VDD. Follow the reference design for the ADC AVDD supply.
- **USB:** 27 Ω series resistors on D+/D−. USB-C needs 5.1 k pull-downs on CC1/CC2, plus ESD protection.
- **Buttons:** BOOTSEL (QSPI_SS to GND through a resistor and a button) and RUN/reset.
- **SWD:** SWDIO/SWCLK/GND pads for debugging with a second Pico running debugprobe.

### Hand assembly

The 60-pin QFN (7 × 7 mm, 0.4 mm pitch) has an exposed centre pad. It needs a stencil
and reflow or hot air, not an iron. Add vias in the centre pad, and include a test point for 3.3 V and the core voltage.

## Considered and dropped

- **Bare RP2040:** needs an external QSPI flash chip as well as the crystal. The RP2354A removes the flash chip.
- **STM32 with crystal-less USB** (F042, F072, C071): has no crystal, flash chip or inductor, but in stock it costs more for less headroom,
  and it can't do UF2 drag-and-drop flashing. Cheap STM32s in stock (G031, C011, F030) have **no USB**.
- **ESP32:** dropped early. No wireless needed for v1.

## HX711 timing (any MCU)

Each HX711 runs on its own oscillator at about 10 SPS and has its **own SCK pin**
(4 GPIO total). The firmware polls both and reads whichever is ready, so A and
B are both updated within one ~100 ms conversion period. Each read is 25 SCK
pulses, about 50 µs, with interrupts masked. If SCK stays high for more than 60 µs, the HX711
powers down. Use **10 SPS** mode (RATE pin low): much lower noise than 80 SPS,
and it rejects 50/60 Hz mains.

Keep the firmware on the plain Arduino API (no PIO, no second core) unless there is a real need.
The same code then runs on the Pico, Pico 2 and the custom board.

## PCB analog notes

- **HX711 placement:** each HX711 sits in its probe head. Only VCC, GND, DOUT and SCK go down the cable.
- **On the main board:** give each HX711 connector its own decoupling (10 µF + 100 nF), and keep the
  cable connectors away from USB, the switching core regulator's inductor and the OLED lines.
