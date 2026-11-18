#include "display.h"
#include <U8g2lib.h>
#include <Wire.h>
#include <math.h>

// 0.96" SSD1306 128x64. For a 1.3" SH1106 use U8G2_SH1106_128X64_NONAME_F_HW_I2C.
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

namespace {
// Monospace "terminal" font, 9 px wide => 14 columns.
const uint8_t* FONT = u8g2_font_profont17_mr;
constexpr int COL = 9;

void drawDelta(int x, int baseline) {   // outline triangle, the font has no glyph for it
    oled.drawLine(x, baseline, x + 4, baseline - 10);
    oled.drawLine(x + 4, baseline - 10, x + 8, baseline);
    oled.drawLine(x, baseline, x + 8, baseline);
}

// Marker column: '=' stable, '~' settling, 'H' display held.
void row(char label, int baseline, const Channel& ch, bool hold) {
    char buf[16];
    oled.setCursor(0, baseline);
    oled.print(label);
    if (!ch.online()) {
        oled.setCursor(2 * COL, baseline);
        oled.print("-- no hx --");
        return;
    }
    snprintf(buf, sizeof buf, "%7.2f N", ch.tensionN());
    oled.setCursor(2 * COL, baseline);
    oled.print(buf);
    oled.setCursor(13 * COL, baseline);
    oled.print(hold ? 'H' : (ch.stable() ? '=' : '~'));
}
}  // namespace

namespace display {

void begin() {
    Wire.setSDA(PIN_SDA);
    Wire.setSCL(PIN_SCL);
    oled.begin();
    oled.setFont(FONT);
}

void message(const char* line1, const char* line2) {
    oled.clearBuffer();
    oled.setCursor(0, 24);
    oled.print(line1);
    if (line2) {
        oled.setCursor(0, 48);
        oled.print(line2);
    }
    oled.sendBuffer();
}

void draw(const Channel& a, const Channel& b, bool hold) {
    oled.clearBuffer();
    row('A', 15, a, hold);
    row('B', 34, b, hold);
    oled.drawHLine(0, 40, 128);

    drawDelta(0, 60);
    if (a.online() && b.online()) {
        const float ta = a.tensionN();
        const float tb = b.tensionN();
        const float d = ta - tb;
        const float mean = 0.5f * (ta + tb);
        char buf[16];
        snprintf(buf, sizeof buf, "%7.2f", d);
        oled.setCursor(2 * COL, 60);
        oled.print(buf);
        if (fabsf(mean) > 0.5f) {
            snprintf(buf, sizeof buf, "%+4.0f%%", 100.0f * d / mean);
            oled.setCursor(128 - 5 * COL, 60);
            oled.print(buf);
        }
    } else {
        oled.setCursor(2 * COL, 60);
        oled.print("   --");
    }
    oled.sendBuffer();
}

}  // namespace display
