#pragma once
#include <Arduino.h>

// Minimal HX711 driver: channel A, gain 128. Non-blocking: poll ready(), then read().
class Hx711 {
public:
    void begin(uint8_t dout, uint8_t sck);
    bool ready() const { return digitalRead(dout_) == LOW; }
    int32_t read();       // call only when ready(); ~50 us with interrupts masked
    void powerDown();
    void powerUp();

private:
    uint8_t dout_ = 0xFF;
    uint8_t sck_  = 0xFF;
};
