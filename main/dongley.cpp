#include "esp_log.h"

namespace {
  const char* TAG = "dongley";
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "Hello, Dongley!");
}
