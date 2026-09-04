#include "hal/board_display.hpp"

#include "halpp/config.hpp"

esp_lcd_panel_ssd1306_config_t ssd1306_vendor_config = {.height = halpp::config::Display::HEIGHT};
