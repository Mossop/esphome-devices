#include "esphome/core/log.h"
#include "calendar_events.h"

namespace esphome {
namespace calendar_events {

static const char *TAG = "calendar_events";

void CalendarEvents::setup() {
}

void CalendarEvents::loop() {
}

void CalendarEvents::update() {
  std::string data = json::build_json([this](JsonObject root) {
    JsonArray data = root["entity_id"].to<JsonArray>();
    for (const auto &calendar : this->calendars) {
      data.add(calendar);
    }
    root["duration"] = this->duration;
  });

  std::vector<Event> found_events;

  bool success = this->homeassistant_api->query_service("calendar", "get_events", data, [&](JsonObject obj) {
    for (JsonPair kv : obj) {
      std::string calendar = kv.key().c_str();

      if (kv.value().is<JsonObject>() && kv.value()["events"].is<JsonArray>()) {
        JsonArray events = kv.value()["events"];
        for (JsonVariant event_var : events) {
          if (event_var.is<JsonObject>()) {
            JsonObject json_event = event_var;
            Event event;

            event.calendar = calendar;
            event.summary = json_event["summary"] | "";
            if (!homeassistant_api::parse_iso8601(json_event["start"], event.start)) {
              continue;
            }
            if (!homeassistant_api::parse_iso8601(json_event["end"], event.end)) {
              continue;
            }

            found_events.push_back(std::move(event));
          }
        }
      }
    }

    return true;
  });

  if (!success) {
    ESP_LOGW(TAG, "Failed to fetch calendar events");
    return;
  }

  ESP_LOGD(TAG, "Found %u calendar entries", found_events.size());

  std::sort(found_events.begin(), found_events.end());
  if (this->events != found_events) {
    this->events = std::move(found_events);
    this->state_callback.call();
  }
}

void CalendarEvents::dump_config() {
    ESP_LOGCONFIG(TAG, "Calendar Events");
}

} //namespace calendar_events
} //namespace esphome
