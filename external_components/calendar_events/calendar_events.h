#pragma once

#include "esphome/core/component.h"
#include "esphome/core/time.h"
#include "esphome/components/http_request/http_request_idf.h"
#include "esphome/components/homeassistant_api/homeassistant_api.h"

namespace esphome {
namespace calendar_events {

struct Event {
  std::string calendar;
  std::string summary;
  ESPTime start;
  ESPTime end;

  bool operator==(const Event &other) const {
    return calendar == other.calendar && summary == other.summary && start == other.start && end == other.end;
  }

  bool operator<(const Event &other) const {
    return start < other.start;
  }
};

class CalendarEvents : public PollingComponent {
  public:
    void setup() override;
    void loop() override;
    void update() override;
    void dump_config() override;

    float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

    const std::vector<Event> &get_events() const { return this->events; }

    void add_on_state_callback(std::function<void()> &&callback) {
      this->state_callback.add(std::move(callback));
    }

    void set_homeassistant_api(homeassistant_api::HomeAssistantApi *homeassistant_api) { this->homeassistant_api = homeassistant_api; }
    void set_duration(uint32_t duration) { this->duration = duration; }
    void set_start_offset(int32_t offset) { this->start_offset = offset; }
    void set_calendars(const std::vector<std::string> calendars) { this->calendars = std::move(calendars); }

  private:
    std::vector<Event> events;
    homeassistant_api::HomeAssistantApi *homeassistant_api;
    CallbackManager<void()> state_callback{};
    uint32_t duration;
    int32_t start_offset;
    std::vector<std::string> calendars;
};

class UpdateTrigger : public Trigger<> {
 public:
  explicit UpdateTrigger(CalendarEvents *parent) {
    parent->add_on_state_callback([this]() { this->trigger(); });
  }
};

} //namespace calendar_events
} //namespace esphome
