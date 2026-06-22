#include <cstdint>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <soc/gpio_num.h>

#include "espbase/boot/check_crash_loop.hpp"
#include "espbase/boot/delayed_pm_enable.hpp"
#include "halpp/buzzer/beeps.hpp"
#include "halpp/buzzer/melodies.hpp"
#include "halpp/buzzer/passive.hpp"
#include "halpp/led_strip/led_strip.hpp"
#include "halpp/segmented/i2c_7seg.hpp"

#include "hal/board.hpp"

namespace {
static constexpr char TAG[] = "dongley";
static constexpr gpio_num_t LED_GPIO_PIN = GPIO_NUM_48;
}

EspResult<void> init_and_run_display() {
  if (EspError err = HAL::Passive::init_default({.gpio_num = GPIO_NUM_13})) {
    return err.log(TAG, "Failed to initialize passive buzzer");
  }
  HAL::Passive& buzzer = HAL::Passive::default_instance();

  buzzer.play(HAL::beeps::startup);

  if (EspError err = HAL::I2C7Seg::init_default(HAL::I2CConfig::ADDR_7SEG)) {
    return err.log(TAG, "Failed to initialize 7-segment display");
  }
  HAL::I2C7Seg& display = HAL::I2C7Seg::default_instance();

  // 5. Write to the local buffer using the modern C++ formatters
  // display.print_float(42.69, 2);

  vTaskDelay(pdMS_TO_TICKS(500));
  // buzzer.play(HAL::melodies::mo_li_hua);

  uint32_t i = 0;
  uint32_t divisor = 1;
  uint32_t delay_ms = 10;
  uint32_t next_threshold = 10000;

  while (true) {
    // When we cross the threshold, scale our units by 10
    if (i >= next_threshold) {
      divisor *= 10;
      delay_ms *= 10;
      next_threshold *= 10;

      buzzer.play(HAL::beeps::success);
      ESP_LOGI(TAG, "Scale shifted! Divisor: %lu, Delay: %lu ms", divisor, delay_ms);
    }

    // Print the scaled value (drops the least significant digits)
    display.print_number(i / divisor);

    if (EspError err = display.write_display()) {
      return err.log(TAG, "Failed to write 7-segment display");
    }

    // Increment `i` by the divisor.
    // This ensures `i` always represents the total elapsed time in 10ms ticks,
    // and the display visibly updates on every single loop iteration.
    i += divisor;

    vTaskDelay(pdMS_TO_TICKS(delay_ms));
  }

  return ESP_OK;
}

static void on_crash_loop_threshold() {
  HAL::LedStrip::init_default({.gpio_num = LED_GPIO_PIN})
      .log_error(TAG, "Failed to init crash loop LED");
  HAL::LedStrip& led = HAL::LedStrip::default_instance();

  led.set_pixel_hsv(0, 0, 255, 20);  // Red color at low brightness
  led.refresh();
}

extern "C" void app_main(void) {
  check_crash_loop(on_crash_loop_threshold);
  delayed_pm_enable();

  init_and_run_display();

  ESP_LOGI(TAG, "Starting Rainbow LED cycle...");

  HAL::LedStrip::init_default({.gpio_num = LED_GPIO_PIN})
      .log_error(TAG, "Failed to init default LED");
  HAL::LedStrip& led = HAL::LedStrip::default_instance();

  uint16_t hue = 0;

  while (true) {
    // hue: 0-359, sat: 0-255, val: 0-255
    // Value (brightness) is kept low at 20 to prevent blinding glare and high current draw
    led.set_pixel_hsv(0, hue, 255, 20).log_error(TAG, "Failed to set LED color");
    led.refresh().log_error(TAG, "Failed to refresh LED");

    hue = (hue + 1) % 360;

    // 10ms delay yields approximately a 3.6-second rainbow cycle
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}