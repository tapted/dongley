#include <esp_log.h>
#include <soc/gpio_num.h>

#include "alarm_clock.hpp"
#include "dongley_clock.hpp"
#include "dongley_device.hpp"
#include "dongley_display.hpp"
#include "dongley_network.hpp"
#include "dongley_sensors.hpp"
#include "espbase/boot/check_crash_loop.hpp"
#include "espbase/boot/delayed_pm_enable.hpp"
#include "espbase/boot/network_logger.hpp"
#include "espbase/boot/ota_rollback_watchdog.hpp"
#include "espbase/json.hpp"
#include "espbase/main_loop.hpp"
#include "espbase/nvs_store.hpp"
#include "halpp/led_strip/led_strip.hpp"
#include "happy/entities/light.hpp"
#include "happy/entities/ota.hpp"

namespace {
static constexpr char TAG[] = "dongley";
static constexpr gpio_num_t LED_GPIO_PIN = GPIO_NUM_48;
static volatile bool ntp_is_ready = false;
}  // namespace

static constinit AlarmClock<3> alarms;
static HAPPY::Entities::OtaController* ota_controller = nullptr;

namespace {

void on_light_update(const HAPPY::Entities::Light& light) {
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

static void on_crash_loop_threshold() {
  HAL::LedStrip::init_default({.gpio_num = LED_GPIO_PIN})
      .log_error(TAG, "Failed to init crash loop LED");
  HAL::LedStrip& led = HAL::LedStrip::default_instance();

  led.set_pixel_hsv(0, 0, 255, 20);  // Red color at low brightness
  led.refresh();
}

extern size_t system_diagnostic_free_iram_at_boot;

extern "C" void app_main(void) {
  check_crash_loop(on_crash_loop_threshold);
  system_diagnostic_free_iram_at_boot = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  initialize_network_logger();
  delayed_pm_enable();

  // Start the 2-minute countdown (Does nothing if this isn't a fresh OTA)
  start_ota_rollback_watchdog(3, 120000);

  NvsStore::init_flash().log_error(TAG, "Failed to init NVS flash");
  init_json_to_use_psram();
  init_dongley_display();
  HAL::LedStrip::init_default({.gpio_num = LED_GPIO_PIN}).log_error(TAG, "Onboard LED init");

  alarms.init(dongley_device);
  ota_controller = new HAPPY::Entities::OtaController(dongley_device, "1.0.0");
  install_dongley_sensors();
  dongley_device.load();  // Load all entities from NVS before starting the network

  network.time_sync_callback = [](struct timeval* /*tv*/) {
    ntp_is_ready = true;
    AlarmClockBase::on_time_synced();
    startup_gate_passed("NTP Time Synced");
    publish_dongley_sensors(true);

    network.time_sync_callback = [](struct timeval* /*tv*/) {
      publish_dongley_sensors(true);  // Re-publish sensors/diagnostics after each NTP sync.
    };
  };

  // Entities must be registered before the network is started so discovery messages are not missed.
  network.start();

  init_and_run_clock(&ntp_is_ready);

  ESP_LOGI(TAG, "Starting main loop...");
  main_loop.run_forever();
}