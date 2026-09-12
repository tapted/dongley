#include "hal/board_display.hpp"

#include "halpp/config.hpp"

const constinit esp_lcd_panel_ssd1306_config_t ssd1306_vendor_config = {
  .height = halpp::config::Display::HEIGHT,
  .contrast = halpp::config::Display::BACKLIGHT_DEFAULT * 255 / 100
};
