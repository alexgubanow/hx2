#include "hx711.h"

// SCK high for >60 us powers the HX711 down, so the ~50 us bit-bang must not be
// interrupted (USB/timer ISRs). A PIO state machine can replace this later.
#define HX_CRITICAL_BEGIN() noInterrupts()
#define HX_CRITICAL_END()   interrupts()

void Hx711::begin(uint8_t dout, uint8_t sck) {
    dout_ = dout;
    sck_  = sck;
    // Pull-up: a missing/unplugged HX711 then reads "not ready" instead of a floating
    // pin that looks ready and floods the loop with garbage. DOUT is push-pull, so harmless.
    pinMode(dout_, INPUT_PULLUP);
    pinMode(sck_, OUTPUT);
    digitalWrite(sck_, LOW);
}

int32_t Hx711::read() {
    uint32_t v = 0;
    HX_CRITICAL_BEGIN();
    for (uint8_t i = 0; i < 24; ++i) {
        digitalWrite(sck_, HIGH);
        delayMicroseconds(1);
        v = (v << 1) | (digitalRead(dout_) ? 1u : 0u);
        digitalWrite(sck_, LOW);
        delayMicroseconds(1);
    }
    // 25th pulse: next conversion = channel A, gain 128
    digitalWrite(sck_, HIGH);
    delayMicroseconds(1);
    digitalWrite(sck_, LOW);
    HX_CRITICAL_END();

    if (v & 0x800000u) v |= 0xFF000000u;   // sign-extend 24 -> 32 bit
    return static_cast<int32_t>(v);
}

void Hx711::powerDown() {
    digitalWrite(sck_, LOW);
    digitalWrite(sck_, HIGH);
    delayMicroseconds(70);
}

void Hx711::powerUp() {
    digitalWrite(sck_, LOW);   // chip resets to channel A / gain 128
}
