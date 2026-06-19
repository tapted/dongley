#pragma once

#include <cstdint>
#include <driver/gpio.h>
#include <driver/i2c_master.h>

namespace HAL {

namespace I2CConfig {
constexpr gpio_num_t PIN_SDA = GPIO_NUM_11;  //
constexpr gpio_num_t PIN_SCL = GPIO_NUM_10;  //

constexpr uint8_t BUS_NUM = 0;          // I2C_NUM_0
constexpr uint32_t CLK_SPEED = 400000;  // 400kHz standard
constexpr uint32_t TIMEOUT_MS = 1000;   // Transaction timeout
constexpr bool ENABLE_PULLUP = true;

// Bus timing (optional, can be left at defaults)
constexpr uint32_t SCL_WAIT_US = 0;  // 0 = use default

// Onboard Device Addresses
constexpr uint8_t ADDR_7SEG = 0x70;  // 7-Segment Display
}  // namespace I2CConfig

}  // namespace HAL