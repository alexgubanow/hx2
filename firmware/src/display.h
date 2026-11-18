#pragma once
#include "channel.h"

namespace display {
void begin();
void message(const char* line1, const char* line2 = nullptr);
void draw(const Channel& a, const Channel& b, bool hold);
}
