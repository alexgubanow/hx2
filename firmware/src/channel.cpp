#include "channel.h"
#include <math.h>
#include <algorithm>

ChannelCal defaultCal() {
    // countsPerN is a placeholder: a 1 kg / 1 mV/V cell on HX711 @ gain 128
    // gives very roughly 400-450 counts per gram. Calibrate before trusting.
    return ChannelCal{0, 430.0f * 1000.0f / GRAVITY, DEFAULT_SPAN_MM, DEFAULT_DEFL_MM, 1.0f};
}

float tensionFromForce(float forceN, const ChannelCal& c) {
    const float half = c.spanMm * 0.5f;
    const float sinT = c.deflMm / sqrtf(c.deflMm * c.deflMm + half * half);
    return c.k * forceN / (2.0f * sinT);
}

void Channel::begin(uint8_t dout, uint8_t sck) { hx_.begin(dout, sck); }

bool Channel::poll() {
    if (!hx_.ready()) return false;
    lastRaw_ = hx_.read();
    buf_[head_] = lastRaw_;
    head_ = (head_ + 1) % FILTER_LEN;
    if (count_ < FILTER_LEN) ++count_;
    lastSampleMs_ = millis();
    return true;
}

bool Channel::online() const {
    return count_ > 0 && (millis() - lastSampleMs_) < HX_TIMEOUT_MS;
}

int32_t Channel::rawMean() const {
    if (count_ == 0) return 0;
    int64_t sum = 0;
    for (uint8_t i = 0; i < count_; ++i) sum += buf_[i];
    return static_cast<int32_t>(sum / count_);
}

float Channel::countsToTension(float counts) const {
    return tensionFromForce((counts - cal.offset) / cal.countsPerN, cal);
}

float Channel::forceN() const { return (rawMean() - cal.offset) / cal.countsPerN; }

float Channel::tensionN() const { return countsToTension(static_cast<float>(rawMean())); }

float Channel::spreadN() const {
    if (count_ == 0) return INFINITY;
    int32_t lo = buf_[0], hi = buf_[0];
    for (uint8_t i = 1; i < count_; ++i) {
        lo = std::min(lo, buf_[i]);
        hi = std::max(hi, buf_[i]);
    }
    return fabsf(countsToTension(static_cast<float>(hi)) - countsToTension(static_cast<float>(lo)));
}

bool Channel::stable() const {
    return online() && count_ == FILTER_LEN && spreadN() < STABLE_SPREAD;
}

bool Channel::captureMean(int32_t& out, Channel* other) {
    int64_t sum = 0;
    uint8_t n = 0;
    uint32_t last = millis();
    while (n < CAL_SAMPLES) {
        if (other) other->poll();
        if (poll()) {
            sum += lastRaw_;
            ++n;
            last = millis();
        } else if (millis() - last > HX_TIMEOUT_MS) {
            return false;
        }
        yield();
    }
    out = static_cast<int32_t>(sum / n);
    return true;
}

bool Channel::tare(Channel* other) {
    int32_t m;
    if (!captureMean(m, other)) return false;
    cal.offset = m;
    return true;
}

bool Channel::calibrate(float knownN, Channel* other) {
    if (knownN <= 0.0f) return false;
    int32_t m;
    if (!captureMean(m, other)) return false;
    const float delta = static_cast<float>(m - cal.offset);
    if (fabsf(delta) < 500.0f) return false;   // nothing on the probe / not tared
    cal.countsPerN = delta / knownN;           // sign handles swapped A+/A-
    return true;
}

bool Channel::calibrateTension(float knownTensionN) {
    if (knownTensionN <= 0.0f) return false;
    const float k0 = cal.k;
    cal.k = 1.0f;
    const float t = tensionN();
    if (t <= 0.1f) { cal.k = k0; return false; }
    cal.k = knownTensionN / t;
    return true;
}
