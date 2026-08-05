#include "dongley_sensors.hpp"

#include "dongley_device.hpp"
#include "dongley_display.hpp"
#include "espbase/main_loop_task.hpp"
#include "halpp/gpio/debounced_input.hpp"
#include "happy/entities//lazy_sensor.hpp"
#include "happy/entities/system_diagnostics.hpp"
#include "happy/sensors/dht_sensor.hpp"

using halpp::gpio::DebouncedInput;
using HAPPY::Entities::format_tenths;
using HAPPY::Entities::Sensor;
using HAPPY::Entities::StatefulSensor;
using HAPPY::Sensors::DhtSensorReader;

static constinit DhtSensorReader dht11_reader(GPIO_NUM_16, HAPPY::Sensors::DHTType::DHT11);
static constinit DhtSensorReader dht22_reader(GPIO_NUM_4, HAPPY::Sensors::DHTType::AM2301);

static void on_temperature_change(Sensor& sensor, const std::string& value) {
  ESP_LOGD("DHT22", "Temperature changed: %s %s", value.c_str(),
           sensor.config().unit_of_measurement);
  if (value != "unknown") {
    set_display_temperature(value);
  }
}
static void on_humidity_change(Sensor& sensor, const std::string& value) {
  ESP_LOGD("DHT22", "Humidity changed: %s %s", value.c_str(), sensor.config().unit_of_measurement);
  if (value != "unknown") {
    set_display_humidity(value);
  }
}

static StatefulSensor<int16_t> temp11_entity(
    dongley_device, "dht11_temp", "DHT11 Temperature",
    {.device_class = "temperature", .unit_of_measurement = "°C"}, dht11_reader,
    []() -> int16_t { return dht11_reader.get_temp(); }, format_tenths);

static StatefulSensor<int16_t> hum11_entity(
    dongley_device, "dht11_hum", "DHT11 Humidity",
    {.device_class = "humidity", .unit_of_measurement = "%"}, dht11_reader,
    []() -> int16_t { return dht11_reader.get_humidity(); }, format_tenths);

static StatefulSensor<int16_t> temp22_entity(
    dongley_device, "dht22_temp", "DHT22 Temperature",
    {
        .device_class = "temperature",
        .unit_of_measurement = "°C",
        .on_state_publish = on_temperature_change,
    },
    dht22_reader, []() -> int16_t { return dht22_reader.get_temp(); }, format_tenths);

static StatefulSensor<int16_t> hum22_entity(
    dongley_device, "dht22_hum", "DHT22 Humidity",
    {
        .device_class = "humidity",
        .unit_of_measurement = "%",
        .on_state_publish = on_humidity_change,
    },
    dht22_reader, []() -> int16_t { return dht22_reader.get_humidity(); }, format_tenths);

static constinit DebouncedInput light_hw_input(GPIO_NUM_5);
static constexpr Sensor::Config ambient_light_sensor_config = {
    .icon = "mdi:theme-light-dark",
    .get_value = [](void*) -> std::string { return light_hw_input.get_level() ? "dark" : "light"; },
};
static Sensor ambient_light_sensor(dongley_device, "ambient_light", "Ambient Light Level",
                                   ambient_light_sensor_config);

static HAPPY::Entities::SystemDiagnostics* diagnostics = nullptr;

static constinit MainLoopTask<void> publish_sensors_on_time_interval;

void install_dongley_sensors() {
  diagnostics = new HAPPY::Entities::SystemDiagnostics(dongley_device);

  light_hw_input.begin({
      .debounce_ms = 200,
      .on_changed = [](bool,
                       void*) { main_loop.push<&Sensor::request_publish>(&ambient_light_sensor); },
  });
  main_loop.push<&Sensor::request_publish>(&ambient_light_sensor);

  publish_sensors_on_time_interval.start({.name = "publish_sensors"}, nullptr,
                                         [](auto&) -> std::optional<uint32_t> {
                                           publish_dongley_sensors();
                                           return 60000;  // Re-run every 60 seconds
                                         });
}

void publish_dongley_sensors(bool time_sync) {
  static bool synced_once = false;

  diagnostics->publish_all_mutable(time_sync);

  if (synced_once) {
    // DHT11 takes longer to settle. Wait for second sync.
    temp11_entity.publish_if_changed();
    hum11_entity.publish_if_changed();
  }
  temp22_entity.publish_if_changed();
  hum22_entity.publish_if_changed();
  if (time_sync) synced_once = true;
}