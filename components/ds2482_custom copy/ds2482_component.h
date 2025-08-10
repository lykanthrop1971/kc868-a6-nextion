#pragma once
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "DS2482.h"
#include <vector>

namespace esphome {
namespace ds2482_custom {

class DS2482Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_address(uint8_t address) { this->address_ = address; }

  void setup() override {
    ESP_LOGI("ds2482", "Initializing DS2482 at 0x%02X...", this->address_);
    this->ds_ = new DS2482(this->address_);
    if (!this->ds_->begin()) {
      ESP_LOGE("ds2482", "DS2482 not found at I2C address 0x%02X", this->address_);
      return;
    }
    ESP_LOGI("ds2482", "DS2482 ready — scanning for DS18B20 sensors...");
    this->scan_all_channels();
  }

  void update() override {
    for (auto &entry : this->sensors_) {
      uint8_t ch = entry.channel;
      if (!this->ds_->selectChannel(ch))
        continue;

      // Reset and start temperature conversion
      if (!this->ds_->wireReset())
        continue;

      this->ds_->wireWriteByte(0xCC); // Skip ROM
      this->ds_->wireWriteByte(0x44); // Convert T
      delay(750); // Wait for conversion (blocking for now)

      // Read temperature
      this->ds_->wireReset();
      this->ds_->wireWriteByte(0x55); // Match ROM
      for (int i = 0; i < 8; i++)
        this->ds_->wireWriteByte(entry.rom[i]);
      this->ds_->wireWriteByte(0xBE); // Read scratchpad

      uint8_t data[9];
      for (int i = 0; i < 9; i++)
        data[i] = this->ds_->wireReadByte();

      int16_t raw = (data[1] << 8) | data[0];
      float temp_c = raw / 16.0f;
      entry.sensor->publish_state(temp_c);
    }
  }

 protected:
  struct SensorEntry {
    uint8_t channel;
    uint8_t rom[8];
    sensor::Sensor *sensor;
  };

  DS2482 *ds_{nullptr};
  uint8_t address_{0x18};
  std::vector<SensorEntry> sensors_;

  void scan_all_channels() {
    for (uint8_t ch = 0; ch < 8; ch++) {
      if (!this->ds_->selectChannel(ch))
        continue;

      if (!this->ds_->wireReset())
        continue;

      // Dallas 1-Wire Search algorithm
      uint8_t rom[8];
      uint8_t last_discrepancy = 0;
      bool last_device_flag = false;

      while (!last_device_flag) {
        uint8_t id_bit_number = 1;
        uint8_t last_zero = 0;
        uint8_t rom_byte_number = 0;
        uint8_t rom_byte_mask = 1;
        uint8_t search_result = 0;
        memset(rom, 0, 8);

        if (!this->ds_->wireReset())
          break;

        this->ds_->wireWriteByte(0xF0); // Search ROM

        do {
          uint8_t id_bit = this->ds_->wireReadBit();
          uint8_t cmp_id_bit = this->ds_->wireReadBit();

          if ((id_bit == 1) && (cmp_id_bit == 1)) {
            break; // No devices
          }

          uint8_t search_direction;
          if (id_bit != cmp_id_bit) {
            search_direction = id_bit;
          } else {
            if (id_bit_number < last_discrepancy) {
              search_direction = ((rom[rom_byte_number] & rom_byte_mask) > 0);
            } else {
              search_direction = (id_bit_number == last_discrepancy);
            }
            if (search_direction == 0) {
              last_zero = id_bit_number;
            }
          }

          if (search_direction == 1) {
            rom[rom_byte_number] |= rom_byte_mask;
          }

          this->ds_->wireWriteBit(search_direction);
          id_bit_number++;
          rom_byte_mask <<= 1;

          if (rom_byte_mask == 0) {
            rom_byte_number++;
            rom_byte_mask = 1;
          }
        } while (rom_byte_number < 8);

        last_discrepancy = last_zero;
        if (last_discrepancy == 0) {
          last_device_flag = true;
        }

        if (rom[0] != 0) {
          char name[64];
          sprintf(name, "DS18B20 CH%u %02X%02X%02X%02X%02X%02X%02X%02X",
                  ch,
                  rom[0], rom[1], rom[2], rom[3],
                  rom[4], rom[5], rom[6], rom[7]);
          auto *sens = new sensor::Sensor();
          sens->set_name(name);
          App.register_sensor(sens);

          SensorEntry entry;
          entry.channel = ch;
          memcpy(entry.rom, rom, 8);
          entry.sensor = sens;
          this->sensors_.push_back(entry);

          ESP_LOGI("ds2482", "Found %s", name);
          search_result = 1;
        }

        if (!search_result)
          break;

      } // while search
    } // channel loop
  }
};

}  // namespace ds2482_custom
}  // namespace esphome
