# Measurement method

## TL;DR

Use a **self-contained 3-point deflection fixture**: two outer anvils, one
center plunger, a mechanical hard stop that sets the deflection `d`, and a load
cell on the plunger. Read force at a fixed deflection and convert it to
tension in newtons. Report Hz only as a derived "equivalent" for people who
think in Hz.

> **Current probe (v1, [cad/](../cad/)):** the same 3-point method with fixed geometry. The two
> "anvils" are bearing posts (3× MR105ZZ each), and the "plunger" is a nose sleeve on the load
> cell. The "hard stop" is the frame itself: `d` follows from the frame geometry, with nothing to
> adjust. The math below doesn't change.

## Options considered

| Method | How | + | − | Verdict |
|---|---|---|---|---|
| **3-point, force at fixed deflection** | Fixture pushes belt a set `d` between anvils `s` apart, load cell reads `F` | Independent of printer span and belt mass. Static reading, quiet. Cheap parts. Two channels are easy | Fixture has to fit on the belt. Deflection adds a little tension. Belt bending stiffness gives a small offset | **Chosen** |
| 3-point, deflection at fixed force | Spring pushes with a set force, measure `d` (Gates "Krikit"-style) | Simple mechanics | Needs a displacement sensor. Low resolution. Hard to automate | No |
| Frequency / pluck (Hz) | Pluck the span, measure the fundamental (mic, accelerometer, phone app) | No contact. Community standard (Voron: 110 Hz @ 150 mm) | Depends on span length **and** belt linear mass. Needs a pluck. Noisy rooms. One belt at a time | Later as a **cross-check mode** |
| Inline load cell | Load cell in the tensioner/idler | True tension, continuous | Different for every printer, invasive | No (not portable) |
| Printer's own span as the 3 points | Push the belt mid-span relative to the frame | No fixture span needed | `s` becomes the printer's span. You need a reference to the frame. Idler compliance comes in | No |

## The math

### Force to tension (3-point)

Each half of the deflected belt makes an angle θ with the straight line. The
plunger supports the vertical components of both halves:

```
F = 2·T·sinθ,     sinθ = d / sqrt(d² + (s/2)²)

=>  T = F / (2·sinθ)                 exact for an ideal flexible string
    T ≈ F·s / (4·d)                  small angle (d << s), error < 0.3% at d/s = 1/30
```

`s` is the **fixture's** anvil spacing, not the printer's span. That is why
this method carries over between printers.

Worked numbers (from `tools/tension.py design`):

| T (N) | s (mm) | d (mm) | F on cell | F/T | extra path |
|---|---|---|---|---|---|
| 10 | 80 | 2.5 | 1.25 N (127 g) | 0.125 | 0.16 mm |
| 15 | 80 | 2.5 | 1.87 N (191 g) | 0.125 | 0.16 mm |
| 15 | 60 | 3.0 | 2.99 N (304 g) | 0.199 | 0.30 mm |
| 15 | 100 | 2.0 | 1.20 N (122 g) | 0.080 | 0.08 mm |

**Starting design point: s = 80 mm, d = 2.5 mm.** At normal tensions the load
is about 100–300 g, which leaves a 1 kg cell lots of overload margin for
clumsy hands. The HX711 resolves well below 0.1 g, so resolution isn't the
limit. Fixture repeatability is.

### How errors in `s` and `d` show up

Small-angle form, with `F` measured:

```
dT/T ≈ ds/s − dd/d
```

- A **0.1 mm** error in `d` (2.5 mm) gives a **4 %** tension error. **`d` is the critical
  dimension.** Fix it mechanically (in probe v1, by the frame geometry, so print accuracy
  matters) and calibrate the rest out (stage 2, below).
- A 0.5 mm error in `s` (80 mm) gives 0.6 %. Not critical.
- Load-cell beam compliance (a 1 kg bar cell deflects roughly 0.1–0.3 mm at full scale) eats into
  `d` in proportion to the force. That is linear, so end-to-end calibration absorbs it.

For **Δ between A and B**, any error that is the same in both probes cancels.
What matters is that the two probes **match**. See the swap test in
[risks.md](risks.md).

### How it scales with span length

- **3-point fixture:** it doesn't. `F` depends on `T`, `s` and `d`, and the fixture sets `s` and `d`.
  The printer's span only has to be **at least `s` + clearance** and straight.
- **Second-order effect: added tension.** Pushing the belt in lengthens its path by
  `ΔL = 2·sqrt(d² + (s/2)²) − s ≈ 2d²/s` (0.16 mm at the design point).
  That stretch spreads over the belt length between the idlers that hold it, `L_eff`,
  and raises tension by about `ΔT ≈ EA·ΔL / L_eff`. GT2 fiberglass belts are stiff, so
  on a **short printer span** ΔT can be several % of T. That makes the reading depend a
  little on the printer's span after all. Mitigations: keep `d²/s` small, and
  measure it (read at two different `d` values on the same belt: if T changes, the
  stretch is significant). EA for real belts is **unknown here, so measure it on
  the calibration rig**.
- **Hz method:** scales strongly with span (next section).

### Hz method and how to compare

An ideal string fixed at both ends:

```
f = (1 / 2L) · sqrt(T / μ)      =>     T = 4 · L² · f² · μ
```

`L` = free span (m), `μ` = linear mass (kg/m). Weigh 1 m of your actual belt.
Rough values are about 7.7 g/m (6 mm GT2) and about 11.5 g/m (9 mm GT2), but
brands vary.

| f | L | μ | T |
|---|---|---|---|
| 110 Hz | 150 mm | 7.7 g/m | 8.4 N (Voron guide value) |
| 120 Hz | 150 mm | 7.7 g/m | 10.0 N |
| 120 Hz | 150 mm | 11.5 g/m | 14.9 N |

Notes:

- `T ∝ L²·f²`. **A Hz number means nothing without the span it was measured on.**
  10 % error in `L` gives 21 % error in T.
- Hz also depends on `μ`, so 120 Hz on a 9 mm belt is ~50 % more tension than 120 Hz on 6 mm.
- *Open question for the Trident AWD:* what span was the 120 Hz measured on, and is it 6 or 9 mm belt?
  Stock Trident A/B belts are 9 mm.

**How to compare HX2 with Hz in practice**

1. Pick one belt span. Measure its free length `L` and weigh 1 m of the same belt for `μ`.
2. Pluck and measure `f` with a phone spectrum app. Repeat 5×.
3. Measure the same span with HX2, giving `T_hx2`.
4. Compute `T_hz = 4·L²·f²·μ`. Do this at 3–4 tensions.
5. Plot `T_hx2` against `T_hz`. You expect a straight line. The slope is the
   systematic error of one method versus the other, and the intercept is mostly the
   belt's bending stiffness.

The firmware can later show an **"equivalent Hz at L mm"** using a stored `L`
and `μ`, so users can follow existing guides.

## Calibration (per channel, stored on device)

Two stages:

1. **Load cell (counts → newtons).** Turn the probe over, tare, put a known mass on the
   plunger, and run `cal a <grams>`. Good cheap references are coins (a US nickel is 5.000 g)
   or water weighed on a kitchen scale.
2. **Fixture (end-to-end correction `k`).** Calibration rig: a belt span over two
   bearings, one end fixed, with a known mass hanging from the other end (1 kg is about 9.8 N).
   Clamp the probe on, read T, and set `k = T_known / T_read`. This absorbs `d`
   tolerance, cell compliance, belt bending and stretch at that span. Repeat at 2–3
   masses to check linearity.

Stage 1 lets you swap load cells. Stage 2 is what makes the tension values
correct. For **Δ alone**, stage 1 plus matched probes already gets you most of
the way.

## Measurement protocol (to be validated)

- Use the same toolhead position every time. Center it so the spans are comparable.
- Keep motors **consistently** unpowered (or powered). On AWD, unpowered is easier to repeat.
- Measure **corresponding spans** of A and B (same side, same section of the run).
  Tension isn't uniform around a loop (idler friction), so Δ is only meaningful
  between equivalent spans.
- Read at a fixed time after clamping (firmware "stable" flag) so creep doesn't
  depend on the operator.
