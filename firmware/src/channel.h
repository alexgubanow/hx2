#pragma once
#include <Arduino.h>
#include "config.h"
#include "hx711.h"

// Per-channel calibration, persisted in storage.
struct ChannelCal {
    int32_t offset;          // raw counts at zero load (tare)
    float   countsPerN;      // raw counts per newton of NOSE force
    float   spanMm;          // fixture anvil spacing s
    float   deflMm;          // fixture hard-stop deflection d
    float   k;               // end-to-end correction from tension rig (1.0 = none)
};

ChannelCal defaultCal();

// Belt tension from nose force for a 3-point fixture: T = F / (2 sin(theta)).
float tensionFromForce(float forceN, const ChannelCal& c);

// One probe: HX711 + moving window + calibration.
class Channel {
public:
    explicit Channel(char name) : name_(name) {}

    void begin(uint8_t dout, uint8_t sck);
    bool poll();                       // non-blocking; true if a new sample arrived
    bool online() const;

    int32_t rawMean() const;
    float   forceN() const;            // filtered nose force
    float   tensionN() const;          // filtered belt tension
    float   spreadN() const;           // tension max-min in window
    bool    stable() const;

    // Blocking averaged capture (keeps polling `other` so it doesn't go stale).
    bool captureMean(int32_t& out, Channel* other = nullptr);
    bool tare(Channel* other = nullptr);
    bool calibrate(float knownN, Channel* other = nullptr);   // known force on the nose
    bool calibrateTension(float knownTensionN);                // sets k from rig

    char name() const { return name_; }
    int32_t lastRaw() const { return lastRaw_; }

    ChannelCal cal = defaultCal();

private:
    float countsToTension(float counts) const;

    char     name_;
    Hx711    hx_;
    int32_t  buf_[FILTER_LEN] = {};
    uint8_t  count_ = 0;
    uint8_t  head_  = 0;
    int32_t  lastRaw_ = 0;
    uint32_t lastSampleMs_ = 0;
};
