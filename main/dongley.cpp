#include <cstdint>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lvgl.h>
#include <soc/gpio_num.h>

#include "espbase/boot/check_crash_loop.hpp"
#include "espbase/boot/delayed_pm_enable.hpp"
#include "espbase/boot/ota_rollback_watchdog.hpp"
#include "espbase/esp_task.hpp"
#include "espbase/nvs_store.hpp"
#include "halpp/buzzer/beeps.hpp"
#include "halpp/buzzer/melodies.hpp"
#include "halpp/buzzer/passive.hpp"
#include "halpp/config.hpp"
#include "halpp/display/display.hpp"
#include "halpp/display/ssd1306.hpp"
#include "halpp/led_strip/led_strip.hpp"
#include "halpp/network/default_network.hpp"
#include "halpp/segmented/clock_task.hpp"
#include "halpp/segmented/i2c_7seg.hpp"
#include "happy/entities/alarm.hpp"
#include "happy/entities/light.hpp"
#include "happy/entities/ota.hpp"
#include "happy/entities/system_diagnostics.hpp"
#include "happy/transports/mqtt_device.hpp"

namespace {
static constexpr char TAG[] = "dongley";
static constexpr gpio_num_t LED_GPIO_PIN = GPIO_NUM_48;
static volatile bool ntp_is_ready = false;
static volatile bool got_mqtt_command = false;
}  // namespace

class Network : public DefaultNetwork {
 public:
  void network_ready(const esp_netif_ip_info_t& ip_info) override;
};

static constexpr const char* const ALARM_TONES[] = {
    "acknowledge", "success", "error", "startup", "Jasmine Flower", "Radioactive",
};

static void trigger_alarm(const HAPPY::Entities::AlarmController& alarm) {
  const auto tone = alarm.selected_tone();
  ESP_LOGI(TAG, "Alarm %d triggered!", alarm.id);
  if (tone == "acknowledge") {
    HAL::Passive::default_instance().play(HAL::beeps::acknowledge);
  } else if (tone == "success") {
    HAL::Passive::default_instance().play(HAL::beeps::success);
  } else if (tone == "error") {
    HAL::Passive::default_instance().play(HAL::beeps::error);
  } else if (tone == "startup") {
    HAL::Passive::default_instance().play(HAL::beeps::startup);
  } else if (tone == "Jasmine Flower") {
    HAL::Passive::default_instance().play(HAL::melodies::mo_li_hua);
  } else if (tone == "Radioactive") {
    HAL::Passive::default_instance().play(HAL::melodies::radioactive_riff);
  }
}

static HAPPY::Entities::AlarmController* alarm1 = nullptr;
static HAPPY::Entities::SystemDiagnostics* diagnostics = nullptr;
static HAPPY::Entities::OtaController* ota_controller = nullptr;

static void on_alarm(size_t index) {
  if (alarm1 && index == 1) {
    trigger_alarm(*alarm1);
  }
}

namespace {
constinit Network network;
constinit ClockTask clock_task(on_alarm);

constinit HAPPY::Transports::MqttDevice dongley_device({
    .identifiers = "dongley_v1_001",
    .name = "Dongley",
    .manufacturer = "Custom",
    .model = "ESP32-S3 WROOM-1 DevKit",
    .sw_version = "0.1",  // esp_app_get_description()->version
});

void on_light_update(const HAPPY::Entities::Light& light) {
  got_mqtt_command = true;
  auto& strip = HAL::LedStrip::default_instance();
  auto [r, g, b] = light.scaled_rgb();

  strip.set_pixel(0, r, g, b);
  strip.refresh();
}

HAPPY::Entities::Light onboard_led(dongley_device, "status_led", "Onboard LED",
                                   {
                                       .supports_rgb = true,
                                       .on_update = on_light_update,
                                   });

}  // namespace

void Network::network_ready(const esp_netif_ip_info_t& /*ip_info*/) {
  esp_mqtt_client_config_t mqtt_cfg = {};
  mqtt_cfg.broker.address.uri = "mqtt://10.1.0.201";
  mqtt_cfg.credentials.username = "puck1e80";
  mqtt_cfg.credentials.authentication.password = "A9CeSm4MX7tcSMT";
  dongley_device.begin(mqtt_cfg);
}

EspResult<void> init_and_run_display() {
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
    if (ntp_is_ready) {
      break;
    }
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

void show_dongley_test_label() {
  HAL::Display::Guard lock;
  lv_obj_t* label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "Dongley - KPop Demon Hunters Edition!   ");
  lv_obj_set_width(label, 128);
  lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_center(label);
}

// We use a static atomic counter to track the number of startup checks that need to complete before
// marking the app as valid and canceling any rollback. This ensures that both the NTP sync and
// display initialization have completed successfully before proceeding.
static std::atomic<int> startup_checks = 2;

extern "C" void app_main(void) {
  check_crash_loop(on_crash_loop_threshold);
  delayed_pm_enable();

  // Start the 2-minute countdown (Does nothing if this isn't a fresh OTA)
  start_ota_rollback_watchdog(120000);

  NvsStore::init_flash().log_error(TAG, "Failed to init NVS flash");

  EspTask<int> display_init_task;
  display_init_task.start({.core_id = 1}, 0, [](auto&) {
    // Initialize the display in parallel.
    HAL::Ssd1306::init_default_i2c().log_error(TAG, "Failed to init SSD1306 display");
    HAL::Ssd1306::default_instance().init_lvgl().log_error(TAG, "Failed to init LVGL display");
    show_dongley_test_label();
    if (--startup_checks == 0) mark_ota_valid();
  });

  HAL::LedStrip::init_default({.gpio_num = LED_GPIO_PIN})
      .log_error(TAG, "Failed to init default LED");

  alarm1 = new HAPPY::Entities::AlarmController(
      dongley_device, 1, ALARM_TONES,
      [](const HAPPY::Entities::AlarmController& alarm) {
        clock_task.set_alarm(alarm.id, alarm.time().hour(), alarm.time().minute(),
                             alarm.time().second());
        ESP_LOGI(TAG, "Alarm %d updated: time=%02d:%02d:%02d, tone=%s", alarm.id,
                 alarm.time().hour(), alarm.time().minute(), alarm.time().second(),
                 alarm.selected_tone().data());
      },
      trigger_alarm);

  diagnostics = new HAPPY::Entities::SystemDiagnostics(dongley_device);
  ota_controller = new HAPPY::Entities::OtaController(dongley_device, "1.0.0");

  // Entities must be registered before the network is started so discovery messages are not missed.
  network.start();
  network.time_sync_callback = [](struct timeval* /*tv*/) {
    ntp_is_ready = true;
    clock_task.on_time_synced();
    diagnostics->publish_all();  // Re-publish diagnostics after NTP sync.
    if (--startup_checks == 0) mark_ota_valid();
  };

  init_and_run_display();

  ESP_LOGI(TAG, "Starting Rainbow LED cycle. got_mqtt_command=%d", got_mqtt_command);

  HAL::LedStrip& led = HAL::LedStrip::default_instance();
  uint16_t hue = 0;
  while (true) {
    if (!got_mqtt_command) {
      // hue: 0-359, sat: 0-255, val: 0-255
      // Value (brightness) is kept low at 20 to prevent blinding glare and high current draw
      led.set_pixel_hsv(0, hue, 255, 20).log_error(TAG, "Failed to set LED color");
      led.refresh().log_error(TAG, "Failed to refresh LED");

      hue = (hue + 1) % 360;
    }

    // 10ms delay yields approximately a 3.6-second rainbow cycle
    // vTaskDelay(pdMS_TO_TICKS(10));
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}