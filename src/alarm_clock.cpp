#include "alarm_clock.hpp"

#include <esp_log.h>

#include "halpp/buzzer/beeps.hpp"
#include "halpp/buzzer/melodies.hpp"
#include "halpp/buzzer/passive.hpp"
#include "happy/entities/alarm.hpp"

static constexpr const char* const ALARM_TONES[] = {
    "Off",           "acknowledge",     "success",          "error",
    "startup",       "Jasmine Flower",  "Radioactive",      "Shanty",
    "Chord",         "Korobeiniki",     "Korobeiniki Riff", "Ambient",
    "Factory Drone", "Shanty Extended",
};

static constexpr std::span<const HAL::Note> ALARM_TONE_MELODIES[] = {
    {},
    HAL::beeps::acknowledge,
    HAL::beeps::success,
    HAL::beeps::error,
    HAL::beeps::startup,
    HAL::melodies::mo_li_hua,
    HAL::melodies::radioactive_riff,
    HAL::melodies::shanty_riff,
    HAL::melodies::limit_test_chord,
    HAL::melodies::korobeiniki,
    HAL::melodies::korobeiniki_riff,
    HAL::melodies::ambient_sequence,
    HAL::melodies::factory_drone,
    HAL::melodies::shanty_riff_extended,
};

static AlarmClockBase* instance_ = nullptr;

static void trigger_alarm(const HAPPY::Entities::AlarmController& alarm) {
  const auto tone = alarm.selected_tone();
  ESP_LOGI("AlarmClock", "Alarm %d triggered (%s)!", alarm.id, tone.data());
  for (size_t i = 0; i < sizeof(ALARM_TONES) / sizeof(ALARM_TONES[0]); ++i) {
    if (tone == ALARM_TONES[i]) {
      if (i == 0) {
        ESP_LOGI("AlarmClock", "Alarm %d is set to 'Off', no tone will be played.", alarm.id);
        return;
      }
      ESP_LOGI("AlarmClock", "Playing tone: %s (index: %zu)", ALARM_TONES[i], i);
      HAL::Passive::default_instance().play(ALARM_TONE_MELODIES[i]);
      return;
    }
  }
}

void AlarmClockBase::init(HAPPY::Device& device) {
  if (instance_) {
    ESP_LOGW("AlarmClock", "AlarmClock::init() called multiple times, ignoring.");
    return;
  }
  for (size_t i = 0; i < max_alarms_; ++i) {
    alarms_[i] = new AlarmController(device, i + 1, ALARM_TONES, alarm_changed, trigger_alarm);
  }
  instance_ = this;
}

void AlarmClockBase::on_time_synced() {
  if (!instance_) return;
  instance_->clock_task_.on_time_synced();
}

void AlarmClockBase::on_alarm(size_t index) {
  if (!instance_) return;

  ESP_LOGI("AlarmClock", "Alarm %zu triggered!", index);
  if (index < instance_->max_alarms_ && instance_->alarms_[index]) {
    trigger_alarm(*instance_->alarms_[index]);
  }
}

void AlarmClockBase::alarm_changed(const HAPPY::Entities::AlarmController& alarm) {
  if (!instance_) return;

  for (size_t i = 0; i < instance_->max_alarms_; ++i) {
    if (instance_->alarms_[i] != &alarm) continue;

    instance_->clock_task_.set_alarm(i, alarm.time().hour(), alarm.time().minute(),
                                     alarm.time().second());
    ESP_LOGI("AlarmClock", "Alarm %d updated: time=%02d:%02d:%02d, tone=%s", alarm.id,
             alarm.time().hour(), alarm.time().minute(), alarm.time().second(),
             alarm.selected_tone().data());
  }
}
