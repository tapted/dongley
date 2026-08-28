#pragma once

#include <string_view>

void init_dongley_display();
void set_display_temperature(std::string_view temperature_str);
void set_display_humidity(std::string_view humidity_str);
void set_display_footer(std::string_view footer_str);