#include "dongley_display.hpp"

#include <lvgl.h>

#include "dongley_device.hpp"
#include "espbase/boot/ota_rollback_watchdog.hpp"
#include "espbase/esp_task.hpp"
#include "halpp/display/display.hpp"
#include "halpp/display/ssd1306.hpp"
#include "happy/entities/text.hpp"
#include "widgets/label/lv_label.h"

static constexpr char TAG[] = "DongleyDisplay";
static constexpr int DISPLAY_WIDTH = 128;
static constexpr int DISPLAY_HEIGHT = 64;

static lv_obj_t* motd_label = nullptr;
static lv_obj_t* temperature_label = nullptr;
static lv_obj_t* humidity_label = nullptr;
static lv_obj_t* footer_label = nullptr;

void update_motd(const HAPPY::Entities::Text& entity) {
  ESP_LOGI(TAG, "Updating MOTD to: %.*s", static_cast<int>(entity.get_value().length()),
           entity.get_value().data());
  HAL::Display::Guard lock;
  if (motd_label) {
    lv_label_set_text_static(motd_label, entity.get_value().data());
  }
}

HAPPY::Entities::Text motd(dongley_device, "motd", "Message of the Day",
                           {
                               .icon = "mdi:message-text",
                               .on_update = update_motd,
                           });

void show_dongley_test_label() {
  HAL::Display::Guard lock;

  // 1. Main full-screen vertical container (Column)
  lv_obj_t* main_cont = lv_obj_create(lv_screen_active());
  lv_obj_set_size(main_cont, LV_PCT(100), LV_PCT(100));

  // Strip default borders and padding
  lv_obj_set_style_border_width(main_cont, 0, 0);
  lv_obj_set_style_pad_all(main_cont, 0, 0);
  lv_obj_set_style_bg_opa(main_cont, LV_OPA_TRANSP, 0);

  // Configure vertical flow and center everything
  lv_obj_set_layout(main_cont, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // 2. MOTD label (Line 1)
  motd_label = lv_label_create(main_cont);
  const char* message = "Dongley - KPop Demon Hunters Edition!    ";
  if (motd.get_value().length() > 0) {
    message = motd.get_value().data();
  }
  lv_label_set_text_static(motd_label, message);
  lv_obj_set_width(motd_label, DISPLAY_WIDTH);
  lv_label_set_long_mode(motd_label, LV_LABEL_LONG_SCROLL_CIRCULAR);

  // 3. Middle horizontal container (Line 2)
  lv_obj_t* row_cont = lv_obj_create(main_cont);
  lv_obj_set_size(row_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Only as big as the text inside it
  lv_obj_set_style_border_width(row_cont, 0, 0);
  lv_obj_set_style_pad_all(row_cont, 0, 0);
  lv_obj_set_style_bg_opa(row_cont, LV_OPA_TRANSP, 0);

  // Add a vertical gap below the MOTD, and a horizontal gap between Temp & Humidity
  lv_obj_set_style_pad_top(row_cont, 10, 0);
  lv_obj_set_style_pad_column(row_cont, 20, 0); 

  // Configure horizontal flow
  lv_obj_set_layout(row_cont, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(row_cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // 4. Temperature label (Left)
  temperature_label = lv_label_create(row_cont);
  lv_label_set_text(temperature_label, "--°C");

  // 5. Humidity label (Right)
  humidity_label = lv_label_create(row_cont);
  lv_label_set_text(humidity_label, "--%");

  // 6. Footer label (Line 3)
  footer_label = lv_label_create(main_cont);
  lv_label_set_text_static(footer_label, "Footer text here");
  lv_obj_set_width(footer_label, DISPLAY_WIDTH);
  lv_label_set_long_mode(footer_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
}

void init_dongley_display() {
  EspTask<int> display_init_task;
  display_init_task.start({.core_id = 1}, 0, [](auto&) {
    // Initialize the display in parallel.
    HAL::Ssd1306::init_default_i2c().log_error(TAG, "Failed to init SSD1306 display");
    HAL::Ssd1306::default_instance().init_lvgl().log_error(TAG, "Failed to init LVGL display");
    show_dongley_test_label();
    startup_gate_passed("Ssd1306 Display Initialized");
  });
}

void set_display_temperature(const std::string& temp_str) {
  HAL::Display::Guard lock;
  if (temperature_label) {
    lv_label_set_text_fmt(temperature_label, "%s°C", temp_str.c_str());
  }
}

void set_display_humidity(const std::string& hum_str) {
  HAL::Display::Guard lock;
  if (humidity_label) {
    lv_label_set_text_fmt(humidity_label, "%s%%", hum_str.c_str());
  }
}

void set_display_footer(const std::string& footer_str) {
  HAL::Display::Guard lock;
  if (footer_label) {
    lv_label_set_text(footer_label, footer_str.c_str());
  }
}