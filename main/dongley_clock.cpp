#include "dongley_clock.hpp"

#include <cstdint>

#include "halpp/buzzer/beeps.hpp"
#include "halpp/buzzer/passive.hpp"
#include "halpp/segmented/i2c_7seg.hpp"

constexpr char const TAG[] = "dongley_clock";

EspResult<> init_and_run_clock(volatile bool* exit_stopwatch) {
  if (EspError err = HAL::Passive::init_default({.gpio_num = GPIO_NUM_13})) {
    return err.log(TAG, "Failed to initialize passive buzzer");
  }
  HAL::Passive& buzzer = HAL::Passive::default_instance();

  // disable during development - it's too annoying :P.
  // buzzer.play(HAL::beeps::startup);

  if (EspError err = HAL::I2C7Seg::init_default()) {
    return err.log(TAG, "Failed to initialize 7-segment display");
  }
  HAL::I2C7Seg& display = HAL::I2C7Seg::default_instance();

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
    if (*exit_stopwatch) {
      break;
    }
  }
  return ESP_OK;
}