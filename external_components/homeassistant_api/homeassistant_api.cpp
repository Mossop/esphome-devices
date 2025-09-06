#include "esphome/core/log.h"
#include "homeassistant_api.h"

namespace esphome {
namespace homeassistant_api {

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

bool parse_iso8601(const char* s, ESPTime& out) {
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

static const char *TAG = "homeassistant_api";

static bool read_body(http_request::HttpContainer *container, std::string &body) {
  // HttpRequestIDF doesn't support chunked transfer so we have to fake it.

  ESP_LOGD(TAG, "Reading body");
  size_t read_index = 0;
  int read;
  do {
    yield();

    if (body.size() - read_index < 512) {
      ESP_LOGD(TAG, "Growing body buffer from %d to %d", body.size(), body.size() + 10240);
      body.resize(body.size() + 10240);

      yield();
    }

    container->content_length = body.size();
    read = container->read(reinterpret_cast<uint8_t*>(body.data()) + read_index, body.size() - read_index);
    read_index += read;
  } while (read > 0);

  ESP_LOGD(TAG, "Read %d bytes of body", read_index);
  body.data()[read_index] = 0;
  body.resize(read_index + 1);

  yield();
  ESP_LOGD(TAG, "Finished body read", read_index);

  return true;
}

void HomeAssistantApi::setup() {
}

void HomeAssistantApi::loop() {
}

void HomeAssistantApi::dump_config() {
    ESP_LOGCONFIG(TAG, "Home Assistant API");
}

bool HomeAssistantApi::state(const std::string &entity_id, const json::json_parse_t &cb) {
  std::list<http_request::Header> headers = {
    {"Content-Type", "application/json"},
    {"Authorization", "Bearer " + this->token}
  };

  std::string url = this->url + "api/states/" + entity_id;
  ESP_LOGD(TAG, "Querying: %s", url.c_str());
  auto container = this->http_request->get(url, headers);
  if (!container) {
    return false;
  }

  bool is_success = http_request::is_success(container->status_code);

  if (!is_success) {
    ESP_LOGW(TAG, "Home Assistant returned status %d", container->status_code);
    container->end();
    return false;
  }

  std::string response_body;
  if (!read_body(container.get(), response_body)) {
    return false;
  }

  container->end();

  if (!json::parse_json(response_body, cb)) {
    ESP_LOGE(TAG, "Failed to parse JSON response");
    return false;
  } else {
    return true;
  }
}

bool HomeAssistantApi::call_service(
  const std::string &domain,
  const std::string &service,
  const std::string &data
) {
  std::list<http_request::Header> headers = {
    {"Content-Type", "application/json"},
    {"Authorization", "Bearer " + this->token}
  };

  std::string url = this->url + "api/services/" + domain + "/" + service;
  ESP_LOGD(TAG, "Querying: %s, '%s'", url.c_str(), data.c_str());
  auto container = this->http_request->post(url, data, headers);
  if (!container) {
    return false;
  }

  if (!http_request::is_success(container->status_code)) {
    ESP_LOGW(TAG, "Home Assistant returned status %d", container->status_code);
    container->end();
    return false;
  }

  container->end();

  return true;
}

bool HomeAssistantApi::query_service(
  const std::string &domain,
  const std::string &service,
  const std::string &data,
  const json::json_parse_t &cb
) {
  std::list<http_request::Header> headers = {
    {"Content-Type", "application/json"},
    {"Authorization", "Bearer " + this->token}
  };

  std::string url = this->url + "api/services/" + domain + "/" + service + "?return_response";
  ESP_LOGD(TAG, "Querying: %s, '%s'", url.c_str(), data.c_str());
  auto container = this->http_request->post(url, data, headers);
  if (!container) {
    return false;
  }

  bool is_success = http_request::is_success(container->status_code);

  if (!is_success) {
    ESP_LOGW(TAG, "Home Assistant returned status %d", container->status_code);
    container->end();
    return false;
  }

  std::string response_body;
  if (!read_body(container.get(), response_body)) {
    return false;
  }

  container->end();

  if (!json::parse_json(response_body, [&](JsonObject root) {
    if (root["service_response"].is<JsonObject>()) {
      return cb(root["service_response"]);
    } else {
      ESP_LOGE(TAG, "Missing service_response in JSON response");
      return false;
    }
  })) {
    ESP_LOGE(TAG, "Failed to parse JSON response");
    return false;
  } else {
    return true;
  }
}


} //namespace homeassistant_api
} //namespace esphome
