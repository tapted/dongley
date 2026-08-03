#include <cstdint>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <soc/gpio_num.h>

#include "alarm_clock.hpp"
#include "dongley_device.hpp"
#include "dongley_sensors.hpp"
#include "dongley_display.hpp"
#include "espbase/boot/check_crash_loop.hpp"
#include "espbase/boot/delayed_pm_enable.hpp"
#include "espbase/boot/favicon_route.hpp"
#include "espbase/boot/network_logger.hpp"
#include "espbase/boot/ota_rollback_watchdog.hpp"
#include "espbase/json.hpp"
#include "espbase/main_loop.hpp"
#include "espbase/nvs_store.hpp"
#include "halpp/buzzer/beeps.hpp"
#include "halpp/buzzer/passive.hpp"
#include "halpp/led_strip/led_strip.hpp"
#include "halpp/network/default_network.hpp"
#include "halpp/segmented/i2c_7seg.hpp"
#include "happy/entities/light.hpp"
#include "happy/entities/ota.hpp"

namespace {
static constexpr char TAG[] = "dongley";
static constexpr gpio_num_t LED_GPIO_PIN = GPIO_NUM_48;
static volatile bool ntp_is_ready = false;
}  // namespace

// We use a static atomic counter to track the number of startup checks that need to complete before
// marking the app as valid and canceling any rollback. This ensures that both the NTP sync and
// display initialization have completed successfully before proceeding.
static std::atomic<int> startup_checks = 3;

class Network : public DefaultNetwork {
  httpd_handle_t server_ = nullptr;

 public:
  void network_ready(const esp_netif_ip_info_t& ip_info) override;
};

static constinit AlarmClock<3> alarms;
static HAPPY::Entities::OtaController* ota_controller = nullptr;

namespace {

constinit Network network;

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

void Network::network_ready(const esp_netif_ip_info_t& /*ip_info*/) {
  if (server_) {
    ESP_LOGI(TAG, "Network::network_ready() called multiple times, ignoring.");
    return;
  }
  auto server = install_network_logger_routes(nullptr);
  if (server) {
    server_ = *server;
    install_favicon_route(server_);
    ESP_LOGI(TAG, "Network logger HTTP server started successfully.");
  }
  if (dongley_device_begin()) {
    ESP_LOGI(TAG, "MQTT client started successfully");
    if (--startup_checks == 0) mark_ota_valid();
  }
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

  uint32_t i = 0;
  uint32_t divisor = 1;
  uint32_t delay_ms = 10;
  uint32_t next_threshold = 10000;
  bool first_loop = true;
  bool network_start_called = false;

  while (true) {
    if (!first_loop && !network_start_called) {
      network.start();
      network_start_called = true;
    }
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
      // Ensure network is started even if display fails
      if (!network_start_called) network.start();

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
    first_loop = false;
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

extern "C" void app_main(void) {
  check_crash_loop(on_crash_loop_threshold);
  initialize_network_logger();
  delayed_pm_enable();

  // Start the 2-minute countdown (Does nothing if this isn't a fresh OTA)
  start_ota_rollback_watchdog(120000);

  NvsStore::init_flash().log_error(TAG, "Failed to init NVS flash");
  init_json_to_use_psram();

  init_dongley_display([] {
    if (--startup_checks == 0) mark_ota_valid();
  });

  HAL::LedStrip::init_default({.gpio_num = LED_GPIO_PIN})
      .log_error(TAG, "Failed to init default LED");

  alarms.init(dongley_device);

  ota_controller = new HAPPY::Entities::OtaController(dongley_device, "1.0.0");

  install_dongley_sensors();

  dongley_device.load();  // Load all entities from NVS before starting the network

  network.time_sync_callback = [](struct timeval* /*tv*/) {
    ntp_is_ready = true;
    AlarmClockBase::on_time_synced();
    if (--startup_checks == 0) mark_ota_valid();
    publish_sensors_on_time_sync();

    network.time_sync_callback = [](struct timeval* /*tv*/) {
      publish_sensors_on_time_sync();  // Re-publish sensors/diagnostics after each NTP sync.
    };
  };

  // Entities must be registered before the network is started so discovery messages are not missed.
  // We delay network.start() until the loop has iterated once so that the code is cached, and not
  // stalled while the wifi stack pulls calibration data from flash.
  init_and_run_display();

  ESP_LOGI(TAG, "Starting main loop...");
  main_loop.run_forever();
}