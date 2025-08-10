#pragma once
#include "esphome/core/component.h"
#include "i2c/i2c.h"
#include "DS2482.h"

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
    } else {
      ESP_LOGI("ds2482", "DS2482 ready");
    }
  }

  void update() override {
    for (uint8_t ch = 0; ch < 8; ch++) {
      if (this->ds_->selectChannel(ch)) {
        bool present = this->ds_->wireReset();
        ESP_LOGI("ds2482", "Channel %u presence: %s", ch, present ? "yes" : "no");
      }
    }
  }

 protected:
  DS2482 *ds_{nullptr};
  uint8_t address_{0x18};
};

}  // namespace ds2482_custom
}  // namespace esphome
