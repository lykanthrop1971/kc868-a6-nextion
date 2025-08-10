#include "esphome.h"
#include "DS2482.h"

class DS2482Component : public PollingComponent {
 public:
  DS2482Component() : PollingComponent(15000) {} // every 15s

  void setup() override {
    ESP_LOGI("ds2482", "Initializing DS2482...");
    Wire.begin();
    ds = new DS2482(0x18); // adjust address if needed
    if (!ds->begin()) {
      ESP_LOGE("ds2482", "DS2482 not found on I2C");
    } else {
      ESP_LOGI("ds2482", "DS2482 ready");
    }
  }

  void update() override {
    // Example: select channel 0 and try a 1-Wire reset
    for (uint8_t ch = 0; ch < 8; ch++) {
      if (ds->selectChannel(ch)) {
        bool present = ds->wireReset();
        ESP_LOGI("ds2482", "Channel %u presence: %s", ch, present ? "yes" : "no");
      }
    }
  }

 private:
  DS2482 *ds;
};
