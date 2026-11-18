#include "storage.h"
#include <EEPROM.h>
#include <stddef.h>

namespace {
constexpr uint32_t MAGIC   = 0x48583243;  // "HX2C"
constexpr uint16_t VERSION = 1;           // bump when ChannelCal layout changes

struct Blob {
    uint32_t   magic;
    uint16_t   version;
    uint16_t   size;
    ChannelCal a;
    ChannelCal b;
    uint32_t   crc;
};

uint32_t crc32(const uint8_t* p, size_t n) {
    uint32_t c = 0xFFFFFFFFu;
    while (n--) {
        c ^= *p++;
        for (int i = 0; i < 8; ++i) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u)));
    }
    return ~c;
}

uint32_t blobCrc(const Blob& b) {
    return crc32(reinterpret_cast<const uint8_t*>(&b), offsetof(Blob, crc));
}
}  // namespace

namespace storage {

// Arduino-Pico's EEPROM emulation takes 256..4096 bytes (one flash sector backs it).
void begin() {
    static_assert(sizeof(Blob) <= 256, "calibration blob outgrew the EEPROM area");
    EEPROM.begin(256);
}

bool load(ChannelCal& a, ChannelCal& b) {
    Blob blob;
    EEPROM.get(0, blob);
    if (blob.magic != MAGIC || blob.version != VERSION || blob.size != sizeof(Blob)) return false;
    if (blob.crc != blobCrc(blob)) return false;
    a = blob.a;
    b = blob.b;
    return true;
}

bool save(const ChannelCal& a, const ChannelCal& b) {
    Blob blob{};
    blob.magic   = MAGIC;
    blob.version = VERSION;
    blob.size    = sizeof(Blob);
    blob.a       = a;
    blob.b       = b;
    blob.crc     = blobCrc(blob);
    EEPROM.put(0, blob);
    return EEPROM.commit();
}

}  // namespace storage
