#pragma once
#include <Arduino.h>

// ---- pins (RP2040; same GPIO numbers exist on Pico and RP2040-Zero) -------
constexpr uint8_t PIN_A_DOUT = 2;
constexpr uint8_t PIN_A_SCK  = 3;
constexpr uint8_t PIN_SDA    = 4;   // I2C0 (Wire default pins)
constexpr uint8_t PIN_SCL    = 5;
constexpr uint8_t PIN_B_DOUT = 6;
constexpr uint8_t PIN_B_SCK  = 7;
constexpr uint8_t PIN_BUTTON = 14;  // tactile switch to GND

// ---- measurement ----------------------------------------------------------
constexpr float    GRAVITY        = 9.80665f;
constexpr uint8_t  FILTER_LEN     = 8;      // samples in moving window (~0.8 s @ 10 SPS)
constexpr float    STABLE_SPREAD  = 0.15f;  // N of tension, max-min in window => "stable"
constexpr uint32_t HX_TIMEOUT_MS  = 500;    // no sample for this long => channel offline
constexpr uint8_t  CAL_SAMPLES    = 20;     // samples averaged for tare / calibration

// Default fixture geometry (see docs/measurement.md)
constexpr float DEFAULT_SPAN_MM = 80.0f;
constexpr float DEFAULT_DEFL_MM = 2.5f;

// ---- UI -------------------------------------------------------------------
constexpr uint32_t DRAW_INTERVAL_MS = 100;
constexpr uint32_t LONG_PRESS_MS    = 1000;
