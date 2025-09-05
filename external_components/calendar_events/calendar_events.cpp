#include "esphome/core/log.h"
#include "calendar_events.h"

namespace esphome {
namespace calendar_events {

static const char *TAG = "calendar_events";

// timegm fallback for environments without it
static time_t timegm_utc(struct tm* tm) {
  const char* tz = getenv("TZ");
  bool had_tz = (tz != nullptr);
  std::string old_tz = had_tz ? tz : std::string();

  setenv("TZ", "UTC", 1);
  tzset();
  time_t t = mktime(tm);
  if (had_tz) {
    setenv("TZ", old_tz.c_str(), 1);
  } else {
    unsetenv("TZ");
  }

  tzset();

  return t;
}

static bool parse_iso8601(const char* s, ESPTime& out) {
  // Parse date/time part
  struct tm tm = {};
  const char* trailing = strptime(s, "%Y-%m-%dT%H:%M:%S", &tm);
  if (!trailing) {
    return false;
  }

  tm.tm_isdst = -1;
  time_t timestamp = timegm_utc(&tm);

  if (*trailing == '+' || *trailing == '-') {
    // Parse ±HH:MM
    int sign = (*trailing == '-') ? -1 : 1; ++trailing;

    int oh = 0, om = 0;
    if (sscanf(trailing, "%2d:%2d", &oh, &om) != 2) {
      return false;
    }

    timestamp -= sign * (oh * 3600 + om * 60);
  } else if (*trailing && *trailing != 'Z') {
    return false;
  }

  out = ESPTime::from_epoch_local(timestamp);

  return true;
}

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
            if (!parse_iso8601(json_event["start"], event.start)) {
              continue;
            }
            if (!parse_iso8601(json_event["end"], event.end)) {
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
    return;
  }

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
