#include "dongley_device.hpp"

constinit HAPPY::Transports::MqttDevice dongley_device({
    .identifiers = "dongley_v1_001",
    .name = "Dongley",
    .manufacturer = "Custom",
    .model = "ESP32-S3 WROOM-1 DevKit",
    .sw_version = "0.1",  // esp_app_get_description()->version
});

EspResult<> dongley_device_begin() {
  esp_mqtt_client_config_t mqtt_cfg = {};
  mqtt_cfg.broker.address.uri = "mqtt://10.1.0.201";
  // Cap the outbox to 16KB. If it fills up, enqueue will fail safely instead of OOMing.
  mqtt_cfg.outbox.limit = 16384;
  mqtt_cfg.credentials.username = "puck1e80";
  mqtt_cfg.credentials.authentication.password = "A9CeSm4MX7tcSMT";
  return dongley_device.begin(mqtt_cfg);
}