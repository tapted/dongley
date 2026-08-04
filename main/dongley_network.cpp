#include "dongley_network.hpp"

#include "dongley_device.hpp"
#include "espbase/boot/favicon_route.hpp"
#include "espbase/boot/network_logger.hpp"
#include "espbase/boot/ota_rollback_watchdog.hpp"

static constexpr char TAG[] = "DongleyNetwork";

constinit Network network;

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
    startup_gate_passed("MQTT client started successfully");
  }
}