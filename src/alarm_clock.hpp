#pragma once

#include <span>

#include "halpp/segmented/clock_task.hpp"

namespace HAPPY {
class Device;
namespace Entities {
class AlarmController;
}  // namespace Entities
}  // namespace HAPPY

class AlarmClockBase {
 public:
  using AlarmController = HAPPY::Entities::AlarmController;
  constexpr AlarmClockBase(AlarmController** alarms, size_t max_alarms)
      : alarms_(alarms), max_alarms_(max_alarms), clock_task_(on_alarm) {}

  void init(HAPPY::Device& device);
  std::span<AlarmController*> alarms() { return {alarms_, max_alarms_}; }

  static void on_time_synced();

  private:
  AlarmController** const alarms_;
  const size_t max_alarms_;
  ClockTask clock_task_;

  static void on_alarm(size_t alarm_index);
  static void alarm_changed(const HAPPY::Entities::AlarmController& alarm);
};

template <size_t MAX_ALARMS = 3>
class AlarmClock : public AlarmClockBase {
 public:
  constexpr AlarmClock() : AlarmClockBase(controllers_, MAX_ALARMS) {}

 private:
  HAPPY::Entities::AlarmController* controllers_[MAX_ALARMS]{};
};