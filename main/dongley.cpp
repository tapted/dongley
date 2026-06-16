#include <cstdint>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"

namespace {
constexpr char TAG[] = "dongley";
constexpr int LED_GPIO_PIN = 48;
constexpr int LED_COUNT = 1;

class Ws2812 {
 public:
  Ws2812(int gpio, int num_leds) {
    led_strip_config_t strip_config = {};
    strip_config.strip_gpio_num = gpio;
    strip_config.max_leds = num_leds;
    strip_config.led_model = LED_MODEL_WS2812;
    strip_config.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB;
    strip_config.flags.invert_out = false;

    led_strip_rmt_config_t rmt_config = {};
    rmt_config.clk_src = RMT_CLK_SRC_DEFAULT;
    rmt_config.resolution_hz = 10 * 1000 * 1000;  // 10MHz resolution
    rmt_config.mem_block_symbols = 0;
    rmt_config.flags.with_dma = false;

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &handle_));
    ESP_ERROR_CHECK(led_strip_clear(handle_));
  }

  ~Ws2812() {
    if (handle_) {
      led_strip_del(handle_);
    }
  }

  void set_hsv(uint32_t index, uint16_t hue, uint8_t sat, uint8_t val) {
    ESP_ERROR_CHECK(led_strip_set_pixel_hsv(handle_, index, hue, sat, val));
  }

  void refresh() { ESP_ERROR_CHECK(led_strip_refresh(handle_)); }

 private:
  led_strip_handle_t handle_ = nullptr;
};
}  // namespace

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "Starting Rainbow LED cycle...");

  Ws2812 led(LED_GPIO_PIN, LED_COUNT);

  uint16_t hue = 0;

  while (true) {
    // hue: 0-359, sat: 0-255, val: 0-255
    // Value (brightness) is kept low at 20 to prevent blinding glare and high current draw
    led.set_hsv(0, hue, 255, 20);
    led.refresh();

    hue = (hue + 1) % 360;

    // 10ms delay yields approximately a 3.6-second rainbow cycle
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}