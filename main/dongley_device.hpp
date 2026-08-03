#pragma once

#include "espbase/esp_result.hpp"
#include "happy/transports/mqtt_device.hpp"

extern constinit HAPPY::Transports::MqttDevice dongley_device;

// Call once a network connection is established to initialize the device and its entities.
EspResult<> dongley_device_begin();