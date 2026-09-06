#include "dongley_device.hpp"

#include <mqtt_client.h>

#include "halpp/display/default_display.hpp"
#include "happy/entities/button.hpp"
#include "happy/entities/light.hpp"
#include "happy/entities/select.hpp"

constinit HAPPY::Transports::MqttDevice dongley_device({
    .identifiers = "dongley",
    .name = "Dongley",
    .manufacturer = "Custom",
    .model = "ESP32-S3 WROOM-1 DevKit",
    .append_mac_chars = 4,  // Append last 4 chars of MAC to identifiers and name
});

static const char* SCROLL_SPEED_OPTIONS[] = {"Off", "1", "2", "3", "4", "5", "6", "7", "8"};

static void on_backlight_update(const HAPPY::Entities::Light& light) {
  uint8_t brightness = static_cast<uint32_t>(light.brightness()) * 100 / 255;
  halpp::default_display().set_backlight(
      light.is_on() ? halpp::BacklightState::On : halpp::BacklightState::Off, brightness);
}

static void on_torch_update(const HAPPY::Entities::Light& light) {
  halpp::default_display().whole_display_on(light.is_on());
}

static void on_scroll_speed_update(void*, const HAPPY::Entities::Select& select) {
  auto it = std::ranges::find(SCROLL_SPEED_OPTIONS, select.get_selected());
  if (it != std::end(SCROLL_SPEED_OPTIONS)) {
    halpp::default_display().horizontal_scroll(it - std::begin(SCROLL_SPEED_OPTIONS));
  }
}

static HAPPY::Entities::Light screen_backlight(dongley_device, "oled_brightness", "OLED Brightness",
                                               {
                                                   .icon = "mdi:brightness-5",
                                                   .supports_rgb = false,
                                                   .on_update = on_backlight_update,
                                               });

static HAPPY::Entities::Select scroll_speed(dongley_device, "scroll_speed", "Scroll Speed",
                                            {
                                                .icon = "mdi:swap-horizontal",
                                                .options = SCROLL_SPEED_OPTIONS,
                                                .on_update = on_scroll_speed_update,
                                            });

static HAPPY::Entities::Light torch(dongley_device, "torch", "Torch",
                                    {
                                        .icon = "mdi:flashlight",
                                        .supports_rgb = false,
                                        .supports_brightness = false,
                                        .on_update = on_torch_update,
                                    });

static HAPPY::Entities::Button reboot(dongley_device, "reboot", "Reboot",
                                      {.icon = "mdi:restart",
                                       .on_press = [](void*, const auto&) { esp_restart(); }});

EspResult<> dongley_device_begin() {
  esp_mqtt_client_config_t mqtt_cfg = {};
  mqtt_cfg.broker.address.uri = "mqtt://10.1.0.201";
  // Cap the outbox to 16KB. If it fills up, enqueue will fail safely instead of OOMing.
  mqtt_cfg.outbox.limit = 16384;
  mqtt_cfg.credentials.username = "puck1e80";
  mqtt_cfg.credentials.authentication.password = "A9CeSm4MX7tcSMT";
  return dongley_device.begin(mqtt_cfg);
}