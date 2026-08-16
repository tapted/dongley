#pragma once

#include "halpp/config_defaults.hpp"

namespace halpp::board {

struct config : detail::Defaults {
  struct System : detail::Defaults::System {
    static constexpr gpio_num_t PIN_VSPI = GPIO_NUM_45;  // VSPI strapping pin
    static constexpr gpio_num_t PIN_LOG = GPIO_NUM_46;   // Log strapping pin
  };
  struct I2CConfig : detail::Defaults::I2CConfig {
    static constexpr gpio_num_t PIN_SDA = GPIO_NUM_8;  // Left 12 (label 8)
    static constexpr gpio_num_t PIN_SCL = GPIO_NUM_9;  // Left 15 (label 9)
  };
  struct Qspi : detail::Defaults::Qspi {
    static constexpr gpio_num_t PIN_CHIP_SELECT = GPIO_NUM_10;   // FSPICS0
    static constexpr gpio_num_t PIN_SERIAL_CLOCK = GPIO_NUM_12;  // FSPICLK
    static constexpr gpio_num_t PIN_QSPI_SDA_0 = GPIO_NUM_11;    // FSPID
    static constexpr gpio_num_t PIN_QSPI_SDA_1 = GPIO_NUM_13;    // FSPIQ
    static constexpr gpio_num_t PIN_QSPI_SDA_2 = GPIO_NUM_14;    // FSPIWP
    static constexpr gpio_num_t PIN_QSPI_SDA_3 = GPIO_NUM_9;     // FSPIHD
  };
  struct Display7Seg : detail::Defaults::Display7Seg {
    static constexpr uint8_t I2C_ADDRESS = 0x70;
  };
  struct TempDht11 {
    static constexpr gpio_num_t PIN_DATA = GPIO_NUM_16;
  };
  struct TempDht22 {
    static constexpr gpio_num_t PIN_DATA = GPIO_NUM_4;
  };
  struct IndicatorLed {
    static constexpr gpio_num_t PIN_PWM = GPIO_NUM_48;
  };
  struct AmbientLightSensor {
    static constexpr gpio_num_t PIN_DATA = GPIO_NUM_5;
  };
};  // struct config

static_assert(GPIO_NUM_0 == config::System::PIN_BOOT);
// static_assert(GPIO_NUM_1 == HAL::SystemConfig::PIN_EXT1);
// static_assert(GPIO_NUM_2 == config::Audio::PIN_MCLK);  // I2S Master Clock (MCLK)
// static_assert(GPIO_NUM_3 == HAL::SystemConfig::PIN_EXT3);
static_assert(GPIO_NUM_4 == config::TempDht22::PIN_DATA);           // DHT22 Data Pin
static_assert(GPIO_NUM_5 == config::AmbientLightSensor::PIN_DATA);  // Ambient Light Sensor Data Pin
// static_assert(GPIO_NUM_6 == HAL::SystemConfig::PIN_EXT6);
// static_assert(GPIO_NUM_7 == HAL::SystemConfig::PIN_EXT7);
static_assert(GPIO_NUM_8 == config::I2CConfig::PIN_SDA);
static_assert(GPIO_NUM_9 == config::I2CConfig::PIN_SCL);
// static_assert(GPIO_NUM_10 == config::I2CConfig::PIN_SCL);
// static_assert(GPIO_NUM_11 == config::I2CConfig::PIN_SDA);
// static_assert(GPIO_NUM_12 == HAL::SystemConfig::PIN_EXT12);
static_assert(GPIO_NUM_13 == config::Buzzer::PIN_PWM);  // Passive Buzzer PWM output
// static_assert(GPIO_NUM_14 == HAL::SDCardConfig::PIN_CLK);
// static_assert(GPIO_NUM_15 == config::Audio::PIN_AMP_ENABLE);
static_assert(GPIO_NUM_16 == config::TempDht11::PIN_DATA);  // DHT11 Data Pin
// static_assert(GPIO_NUM_17 == HAL::SDCardConfig::PIN_CMD);
// static_assert(GPIO_NUM_18 == HAL::LCDConfig::PIN_TE);
static_assert(GPIO_NUM_19 == config::Usb::PIN_USB_DM);
static_assert(GPIO_NUM_20 == config::Usb::PIN_USB_DP);
// static_assert(GPIO_NUM_21 == HAL::LCDConfig::PIN_CS);
// static_assert(GPIO_NUM_33 == HAL::SystemConfig::PIN_HDR_33);
// static_assert(GPIO_NUM_34 == HAL::SystemConfig::PIN_HDR_34);
// static_assert(GPIO_NUM_35 == HAL::SystemConfig::PIN_HDR_35);
// static_assert(GPIO_NUM_36 == HAL::SystemConfig::PIN_HDR_36);
// static_assert(GPIO_NUM_37 == HAL::SystemConfig::PIN_HDR_37);
// static_assert(GPIO_NUM_38 == config::Audio::PIN_WS);
// static_assert(GPIO_NUM_39 == config::Audio::PIN_DATA_IN);
// static_assert(GPIO_NUM_40 == HAL::LCDConfig::PIN_SCK);
// static_assert(GPIO_NUM_41 == HAL::LCDConfig::PIN_SDA0);
// static_assert(GPIO_NUM_42 == HAL::LCDConfig::PIN_SDA2);
static_assert(GPIO_NUM_43 == config::Usb::PIN_UART_TX);
static_assert(GPIO_NUM_44 == config::Usb::PIN_UART_RX);
static_assert(GPIO_NUM_45 == config::System::PIN_VSPI);  // VSPI
// static_assert(GPIO_NUM_46 == HAL::LCDConfig::PIN_SDA3);
// static_assert(GPIO_NUM_47 == config::Audio::PIN_DATA_OUT);  // Data to Speaker (ES8311)
static_assert(GPIO_NUM_48 == config::IndicatorLed::PIN_PWM);  // Onboard Indicator LED

}  // namespace halpp::board