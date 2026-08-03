#pragma once

#include "halpp/network/default_network.hpp"

typedef void* httpd_handle_t;

class Network : public DefaultNetwork {
  httpd_handle_t server_ = nullptr;

 public:
  void network_ready(const esp_netif_ip_info_t& ip_info) override;
};

extern constinit Network network;
