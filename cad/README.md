# cad/

Probe models, made in [SolidCraft](https://github.com/tangency-labs/solidcraft) 0.42.0.
Use that version or newer, because older versions refuse files saved by newer ones.

![HX2 probe v1](../docs/img/hx2-probe-v1.png)

| File | What |
|---|---|
| `hx2-probe.sc` | Parametric source: frame, nose, load cell, bearings, axle spacers |
| `export/hx2-probe-v1.step` | All bodies as a STEP assembly, for any CAD |
| `export/hx2-probe-v1-frame.stl` | Print: frame |
| `export/hx2-probe-v1-nose.stl` | Print: nose sleeve |
| `export/hx2-probe-v1-spacer.stl` | Print ×2, or cut from 5 × 3.2 mm brass/steel tube (better) |

The bearings in the model are drawn from the MR105ZZ datasheet dimensions (`MR105ZZ`
component), not a vendor file. Vendor STEP models go in `cad/parts/`, which git ignores.

## Probe v1: frame with belt channel

```
 side view, belt runs along X (into the page)
 ┌──roof: 2× M3 heat-set inserts──┐
 │  ▼post ▼post                   │  posts locate in the cell's Ø5.35 counterbores
 │ ┌─cell (flipped)─────────────────┐  thin-end step faces the belt
 │ └──────┐ slot      [══nose══════]┘  sleeve on the thin end, crest pushes the belt down by d
 │        │ (open        ═══belt═══
 │        │  below)    ◯ MR105ZZ ×3    bearings on an M3 axle in a U-pocket, bumps on both walls
 └────────┘────────────────────┘
```

How it works:

- **Fixed geometry, no moving parts.** The bearings sit on our side of the belt and the nose on the
  far side (between the belts). Deflection `d` comes from the geometry:
  `z_post = z_n + probe_d − belt_t − brg_r` (cell thin-end underside = 0, nose crest `z_n = −nose_h`).
- **Belt entry.** The belt slides in along −Y over the low front wall. The rounded front of the nose
  (fillet R = `probe_d + 0.3`) pushes it down by `d` as it goes in.
- **Load cell.** A flipped spoon-scale cell (double-bending beam), so the belt's position across the
  nose doesn't matter. It goes in from below through the slot at about 30–45°, with the nose off.
  Push it up onto the two posts and fix it with 2× M3 into heat-set inserts in the roof.
- **Nose.** A sleeve that slides onto the cell's thin end from the front, until its cap touches the
  cell tip. 2× M3×6 countersunk screws go through the 1 mm top flange into the cell's own threads.
  The crest is one smooth surface with no holes or screw heads under the belt.
- **Bearings.** 3× MR105ZZ per post, on a 4.9 mm printed or metal tube over an M3×25 countersunk
  screw. The screw goes through the front boss, the tube and the back bump, then self-taps 10 mm into
  the frame. 1 mm Ø6 bumps on both walls touch only the inner race.
- **Far side of the belt (between the belts):** crest to flange top is 7.65 mm, plus countersunk heads
  about 0.5 mm, so about 8.2 mm in total.

## Assembly

1. Press 2× M3 heat-set inserts into the roof from the top.
2. Insert the cell from below at 30–45°, thin end first. Swing it level, push it up onto the posts,
   then put in 2× M3 from below.
3. Slide the nose sleeve onto the thin end until it stops. Put in 2× M3×6 countersunk.
4. For each post: slide 3× MR105ZZ onto the tube, set the stack in the pocket, and drive the M3×25
   countersunk screw in from the front.

## Parameters (all in the `.sc` file)

| Parameter | Value | What |
|---|---|---|
| `probe_s` | 80 | bearing spacing `s` |
| `probe_d` | 2.5 | belt deflection `d` |
| `belt_t`, `belt_w` | 1.4, 9 | belt |
| `brg_r`, `brg_w` | 5, 4 | MR105ZZ |
| `nose_h` | 3 | nose crest below the cell |
| `brg_bump`, `bump_d` | 1, 6 | inner-race bumps |
| `post_d`, `ins_d`, `ins_l` | 5.15, 4, 3 | cell posts and heat-set inserts |
| `m3_clr`, `m3_pilot`, `m3_csk` | 3.2, 2.5, 1.4 | screw holes |
| `sleeve_t`, `flange_t`, `nose_cap`, `nose_r` | 1.2, 1, 1.5, 1.4 | nose sleeve |

Derived values (`z_n`, `z_post`, `y_brg`, `ch_y0`/`ch_y1`, `brick_top`/`brick_bot`, `floor_z`,
`boss_top`, `pocket_bot`) follow from these.

**Checked:** rebuilds with no errors with `probe_s` 60–100, `probe_d` 2–3, `nose_h` 2.5–4 and
`flange_t` 1–1.5. No interference between any bodies.

**Known limit:** `bump_d` must be less than the bearing's inner-race land. If you change bearing size
(`brg_r`), change `bump_d` too.

## Open items

- Overload stop for the cell's thin end (for example a ledge on the channel floor ~0.3 mm under the nose)
- HX711 pocket, cable exit and strain relief, and a grip
- Lighten the frame (it's solid, about 86 cm³)
- Print orientation: back face (−Y) on the bed is the likely choice, so that axle holes and pockets print vertically
- Belt-path data from more printers (see [docs/risks.md §2](../docs/risks.md)): free span,
  clearance on both faces, belt width, gap between stacked belts
