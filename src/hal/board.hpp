#pragma once

#include "halpp/config_defaults.hpp"

namespace HAL::board {

struct config : detail::Defaults {
  struct I2CConfig : detail::Defaults::I2CConfig {
    static constexpr gpio_num_t PIN_SDA = GPIO_NUM_8;  // Left 12 (label 8)
    static constexpr gpio_num_t PIN_SCL = GPIO_NUM_9;  // Left 15 (label 9)
  };
  struct Display7Seg : detail::Defaults::Display7Seg {
    static constexpr uint8_t I2C_ADDRESS = 0x70;
  };
};  // struct config

}  // namespace HAL::board