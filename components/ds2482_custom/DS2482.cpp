#include "DS2482.h"

DS2482::DS2482(uint8_t addr) {
  _addr = addr;
}

bool DS2482::begin() {
  Wire.beginTransmission(_addr);
  if (Wire.endTransmission() != 0) return false;
  return reset();
}

bool DS2482::reset() {
  return writeByte(DS2482_CMD_RESET);
}

bool DS2482::selectChannel(uint8_t ch) {
  if (ch > 7) return false;
  uint8_t ch_val = 0xF0 | (ch & 0x07);
  uint8_t ch_read = 0xB8 | ((~ch & 0x07) << 4) | (ch & 0x07);
  writeByte(DS2482_CMD_SELECT_CHANNEL, ch_val);
  Wire.requestFrom((int)_addr, 1);
  if (Wire.available()) {
    uint8_t r = Wire.read();
    return (r == ch_read);
  }
  return false;
}

bool DS2482::wireReset() {
  writeByte(DS2482_CMD_1WIRE_RESET);
  Wire.requestFrom((int)_addr, 1);
  if (Wire.available()) {
    uint8_t s = Wire.read();
    return (s & 0x02) == 0; // presence pulse detect
  }
  return false;
}

void DS2482::wireWriteByte(uint8_t b) {
  writeByte(DS2482_CMD_1WIRE_WRITE_BYTE, b);
}

uint8_t DS2482::wireReadByte() {
  writeByte(DS2482_CMD_1WIRE_READ_BYTE);
  Wire.requestFrom((int)_addr, 1);
  if (Wire.available()) return Wire.read();
  return 0xFF;
}

bool DS2482::writeByte(uint8_t b) {
  Wire.beginTransmission(_addr);
  Wire.write(b);
  return Wire.endTransmission() == 0;
}

bool DS2482::writeByte(uint8_t b1, uint8_t b2) {
  Wire.beginTransmission(_addr);
  Wire.write(b1);
  Wire.write(b2);
  return Wire.endTransmission() == 0;
}

uint8_t DS2482::readByte() {
  Wire.requestFrom((int)_addr, 1);
  if (Wire.available()) return Wire.read();
  return 0xFF;
}
bool DS2482::wireWriteBit(uint8_t bit) {
  writeByte(DS2482_CMD_1WIRE_SINGLE_BIT, bit ? 0x80 : 0x00);
  Wire.requestFrom((int)_addr, 1);
  if (Wire.available()) return true;
  return false;
}

uint8_t DS2482::wireReadBit() {
  wireWriteBit(1); // send read slot
  Wire.requestFrom((int)_addr, 1);
  if (Wire.available()) {
    uint8_t s = Wire.read();
    return (s & 0x01);
  }
  return 1;
}
