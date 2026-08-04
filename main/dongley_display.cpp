#include "dongley_display.hpp"

#include <lvgl.h>

#include "dongley_device.hpp"
#include "espbase/boot/ota_rollback_watchdog.hpp"
#include "espbase/esp_task.hpp"
#include "halpp/display/display.hpp"
#include "halpp/display/ssd1306.hpp"
#include "happy/entities/text.hpp"

static constexpr char TAG[] = "DongleyDisplay";

static lv_obj_t* motd_label = nullptr;

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
  motd_label = lv_label_create(lv_screen_active());
  const char* message = "Dongley - KPop Demon Hunters Edition!    ";
  if (motd.get_value().length() > 0) {
    message = motd.get_value().data();
  }
  lv_label_set_text_static(motd_label, message);
  lv_obj_set_width(motd_label, 128);
  lv_label_set_long_mode(motd_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_center(motd_label);
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