# Assumptions & design risks

Ranked roughly by how likely each is to hurt the project. Each risk has a cheap
test to run **before** committing to a design.

## 1. Fixture repeatability (highest risk)

The reading is only as good as how repeatably the probe sits on the belt.
Placement position, tilt, belt not perpendicular to the anvils, the hard stop
not fully seated, and grip force leaking into the load cell all change `F`.

- `d` is the critical dimension: 0.1 mm off gives 4 % off (see [measurement.md](measurement.md)).
- **The grip force must go into the frame and the hard stop, never through the load cell.**
  The plunger should reach a mechanical stop so that squeezing harder changes nothing.
  *Probe v1:* there is no grip or plunger stroke. `d` is fixed by the frame (bearing posts and the
  nose on the cell), so the risk moves to how well the belt seats against the posts and the nose,
  and to print tolerances on the frame.
- **Test:** clamp, read, unclamp, repeat 10× on the same span. Target spread < 2 %.
  Try different people and slightly different positions.

## 2. Belt-path geometry differs between printers

- Printers need a **free straight span ≥ s + margin** (~100 mm for s = 80). Short spans
  (some Bambu-style and enclosed designs) may need an s ≈ 50–60 mm head.
- **Clearance:** the anvils go on one face of the belt and the plunger on the other. Many printers
  have only a few mm between the belt and an extrusion or panel.
- **Stacked belts** (Voron-style A over B): the gap between the two belts limits how thick the
  jaw can be. It's also a chance for a double-decker head that reads A and B at the same spot.
- **6 mm vs 9 mm:** the jaw must take 9 mm and still center a 6 mm belt.
- **Test (cheap and needed now):** collect free span, clearance on both faces, belt width
  and stack gap from 4–5 printer models (start with the Trident AWD). Probe v1 assumes a free
  span ≥ 100 mm, about 8.1 mm on the far side of the belt and 24 mm on our side
  ([cad/README.md](../cad/README.md)). Check these before printing a set of probes.

## 3. Tension isn't one number per belt

Idler friction and toolhead position mean different spans of the **same** belt
can read differently. On AWD, the four motors (detent torque, powered or not)
add to that. Δ is only meaningful between **corresponding** spans measured
under the same conditions.

- **Test:** measure all accessible spans of belt A at three toolhead positions. If the spread is
  bigger than the A/B difference you care about, the protocol has to fix the
  position and spans.

## 4. Channel mismatch (bias in Δ)

Δ is the main output, so a constant offset between probe A and probe B is
worse than an absolute error shared by both.

- **Test: swap test.** Measure, swap the probes between belts, measure again. Δ should
  flip sign with the same magnitude. Half the difference is the channel bias.
  The firmware could store a per-channel trim from this.

## 5. Creep (load cell, plastic, belt)

- **Load cell:** cheap cells creep about 0.05–0.1 % of full scale over minutes. At 200 g on a 1 kg cell
  that's up to ~0.5 % of the reading. Minor.
- **Printed fixture:** PLA creeps noticeably under constant load. Use PETG/ASA, keep plastic
  out of the load path where possible, and use metal for the hard stop.
- **Belt:** relaxes a little right after deflection.
- **Mitigation:** read at a **fixed time** after the reading settles (firmware stable flag),
  not "whenever you look". **Test:** log 60 s after clamping (`stream` command) and see how far it drifts.

## 6. Belt isn't an ideal string

- **Bending stiffness** adds force that doesn't come from tension, so there's an offset at low T.
  It's larger for short `s` and wide belts.
- **Added tension from deflection** (`ΔL ≈ 2d²/s`) depends on the printer's span, so it's
  a small span dependence after all.
- **Tooth side vs smooth side:** an anvil touching the teeth gives a reading that varies with
  tooth phase unless it bridges several teeth.
- **Mitigation:** end-to-end calibration on a known-tension rig, small `d²/s`, and
  wide rollers or flat pads on the tooth side.
- **Test:** on the rig, read at 2–3 values of `d` and 2–3 masses, then fit the model.

## 7. HX711 noise and drift

Resolution isn't the problem (noise < 0.1 g against a ~150 g signal). The
risks are elsewhere:

- **Temperature drift** of zero. Tare right before measuring. Let things warm up ~1 min.
- **Supply noise** from OLED refresh or the dev board's switching regulator (Pico) coupling
  into the excitation voltage. Add bulk + 100 nF caps at each HX711. On the custom PCB, give
  the HX711s a separate low-noise LDO.
- **Long probe cables.** Use twisted/shielded cable and keep the HX711 **in the probe or at the
  connector**, so only digital lines run down the cable.
- Clone HX711 boards vary (wrong RATE strapping, missing caps). Check each one.
- **Test:** both cells unloaded for 10 min. Log noise (p-p over 1 s) and drift.

## 7b. Salvaged spoon-scale load cells (PoC)

The PoC uses cells salvaged from spoon scales. They are an unknown quantity:
capacity, creep, temperature drift and how closely the two cells match are
all unspecified, and replacements won't be identical.

- Fine for proving the method, but each probe must be calibrated with weights (stage 1) **and** on the rig (stage 2).
- Small cells mean a small overload margin. Every probe head needs a hard stop that protects the beam.
- **Test:** the 10 min noise/drift log from §7 plus a 60 s creep log under a constant ~200 g, for each cell.
  If one cell is clearly worse, that sets the Δ resolution.
- For a design other people can rebuild, standardize on one documented cell later ([bom.md](bom.md)).

## 8. Calibration & storage

- Calibration with a weight (counts → N) is easy to get wrong: wrong orientation, or a
  cable tugging on the probe. Do tare and calibration in the same pose.
- Flash wear isn't a concern (calibration saves are rare).
- Storage has to survive firmware updates. The firmware keeps a versioned struct with CRC
  (`firmware/src/storage.*`).

## 9. Things this design assumes and you should confirm

- [ ] Typical target tensions are roughly 5–25 N, so a 1 kg cell at s=80/d=2.5 suits all printers
- [ ] Users accept a clip-on probe (vs a contactless pluck)
- [ ] Stock Trident A/B belts are 9 mm. Confirm on the Siboor AWD kit
- [ ] What span the 120 Hz figure was measured on
