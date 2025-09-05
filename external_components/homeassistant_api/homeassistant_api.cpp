#include "esphome/core/log.h"
#include "homeassistant_api.h"

namespace esphome {
namespace homeassistant_api {

static const char *TAG = "homeassistant_api";

void HomeAssistantApi::setup() {
}

void HomeAssistantApi::loop() {
}

void HomeAssistantApi::dump_config() {
    ESP_LOGCONFIG(TAG, "Home Assistant API");
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

  // HttpRequestIDF doesn't support chunked transfer so we have to fake it.
  std::vector<uint8_t, RAMAllocator<uint8_t>> buf;

  size_t read_index = 0;
  int read;
  do {
    if (buf.size() - read_index < 512) {
      buf.resize(buf.size() + 1024);
    }

    container->content_length = buf.size();
    read = container->read(buf.data() + read_index, buf.size() - read_index);
    read_index += read;
  } while (read > 0);

  std::string response_body;
  response_body.reserve(read_index);
  response_body.assign((char *) buf.data(), read_index);
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
