#pragma once
#include "channel.h"

// Calibration persisted in flash (Arduino-Pico EEPROM emulation: last flash sector).
namespace storage {
void begin();
bool load(ChannelCal& a, ChannelCal& b);   // false => defaults kept
bool save(const ChannelCal& a, const ChannelCal& b);
}
