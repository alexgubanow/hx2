// HX2 - two-channel CoreXY belt tension meter
// Reads two HX711s, shows A / B / Δ on an OLED, serial console for calibration.

#include <Arduino.h>
#include "config.h"
#include "channel.h"
#include "display.h"
#include "storage.h"

static Channel chA('A');
static Channel chB('B');

static bool     hold = false;
static bool     streaming = false;
static uint32_t lastDraw = 0;

// ---- helpers --------------------------------------------------------------

static Channel* pick(const String& s) {
    if (s.equalsIgnoreCase("a")) return &chA;
    if (s.equalsIgnoreCase("b")) return &chB;
    return nullptr;
}

static Channel* otherOf(Channel* c) { return c == &chA ? &chB : &chA; }

static void printCal(const Channel& c) {
    Serial.printf("%c: offset=%ld cpn=%.2f s=%.2fmm d=%.3fmm k=%.4f | raw=%ld F=%.3fN T=%.2fN %s\n",
                  c.name(), (long)c.cal.offset, c.cal.countsPerN, c.cal.spanMm, c.cal.deflMm,
                  c.cal.k, (long)c.rawMean(), c.forceN(), c.tensionN(),
                  c.online() ? (c.stable() ? "stable" : "settling") : "OFFLINE");
}

static void help() {
    Serial.println(F(
        "HX2 commands:\n"
        "  tare [a|b]            zero one or both channels (no load on probe)\n"
        "  cal <a|b> <grams>     known mass resting on the nose -> counts/N\n"
        "  rig <a|b> <newtons>   probe clamped on a belt of known tension -> k\n"
        "  fix <a|b> <s_mm> <d_mm>  set fixture geometry\n"
        "  show                  print calibration and live values\n"
        "  save | load           persist / reload calibration\n"
        "  stream                toggle CSV: ms,rawA,rawB,TA,TB\n"
        "  help"));
}

static void ok(bool r) { Serial.println(r ? "ok" : "FAILED"); }

static void handleCommand(String line) {
    line.trim();
    if (line.isEmpty()) return;

    String tok[4];
    int n = 0;
    while (n < 4 && line.length()) {
        int sp = line.indexOf(' ');
        tok[n++] = sp < 0 ? line : line.substring(0, sp);
        line = sp < 0 ? "" : line.substring(sp + 1);
        line.trim();
    }
    const String& cmd = tok[0];

    if (cmd == "help") {
        help();
    } else if (cmd == "tare") {
        display::message("tare...");
        if (n >= 2) {
            Channel* c = pick(tok[1]);
            ok(c && c->tare(otherOf(c)));
        } else {
            ok(chA.tare(&chB) && chB.tare(&chA));
        }
    } else if (cmd == "cal" && n == 3) {
        Channel* c = pick(tok[1]);
        display::message("cal...");
        ok(c && c->calibrate(tok[2].toFloat() / 1000.0f * GRAVITY, otherOf(c)));
    } else if (cmd == "rig" && n == 3) {
        Channel* c = pick(tok[1]);
        ok(c && c->calibrateTension(tok[2].toFloat()));
    } else if (cmd == "fix" && n == 4) {
        Channel* c = pick(tok[1]);
        const float s = tok[2].toFloat(), d = tok[3].toFloat();
        if (c && s > 0 && d > 0 && d < s / 2) {
            c->cal.spanMm = s;
            c->cal.deflMm = d;
            ok(true);
        } else {
            ok(false);
        }
    } else if (cmd == "show") {
        printCal(chA);
        printCal(chB);
    } else if (cmd == "save") {
        ok(storage::save(chA.cal, chB.cal));
    } else if (cmd == "load") {
        ok(storage::load(chA.cal, chB.cal));
    } else if (cmd == "stream") {
        streaming = !streaming;
        if (streaming) Serial.println("ms,rawA,rawB,TA,TB");
    } else {
        Serial.println("? (try 'help')");
    }
}

static void pollSerial() {
    static String buf;
    while (Serial.available()) {
        char c = static_cast<char>(Serial.read());
        if (c == '\n' || c == '\r') {
            if (buf.length()) handleCommand(buf);
            buf = "";
        } else if (buf.length() < 64) {
            buf += c;
        }
    }
}

// Short press: tare both. Long press: toggle display hold.
static void pollButton() {
    static bool     wasDown = false;
    static uint32_t downAt = 0;
    static bool     longFired = false;
    const bool down = digitalRead(PIN_BUTTON) == LOW;
    const uint32_t now = millis();

    if (down && !wasDown) {
        downAt = now;
        longFired = false;
    } else if (down && !longFired && now - downAt > LONG_PRESS_MS) {
        hold = !hold;
        longFired = true;
    } else if (!down && wasDown && !longFired && now - downAt > 30) {
        display::message("tare...");
        chA.tare(&chB);
        chB.tare(&chA);
    }
    wasDown = down;
}

// ---- main -----------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BUTTON, INPUT_PULLUP);

#if defined(ARDUINO_RASPBERRY_PI_PICO)
    // Pico (not Pico W): GP23 high forces the onboard SMPS into PWM mode.
    // Lower ripple on 3V3 means less noise coupled into the HX711 excitation.
    pinMode(23, OUTPUT);
    digitalWrite(23, HIGH);
#endif

    display::begin();
    display::message("HX2", "booting");

    chA.begin(PIN_A_DOUT, PIN_A_SCK);
    chB.begin(PIN_B_DOUT, PIN_B_SCK);

    storage::begin();
    const bool loaded = storage::load(chA.cal, chB.cal);
    delay(300);
    Serial.println(loaded ? "HX2: calibration loaded" : "HX2: no calibration, using defaults");
    help();
}

void loop() {
    const bool newA = chA.poll();
    const bool newB = chB.poll();

    if (streaming && (newA || newB)) {
        Serial.printf("%lu,%ld,%ld,%.3f,%.3f\n", (unsigned long)millis(), (long)chA.lastRaw(),
                      (long)chB.lastRaw(), chA.tensionN(), chB.tensionN());
    }

    pollSerial();
    pollButton();

    // Hold freezes the screen on the values at the moment of the long press, so
    // you can read it after pulling the probe out of a cramped printer.
    static bool heldFrameDrawn = false;
    if (hold) {
        if (!heldFrameDrawn) {
            display::draw(chA, chB, true);
            heldFrameDrawn = true;
        }
    } else {
        heldFrameDrawn = false;
        if (millis() - lastDraw >= DRAW_INTERVAL_MS) {
            lastDraw = millis();
            display::draw(chA, chB, false);
        }
    }
}
