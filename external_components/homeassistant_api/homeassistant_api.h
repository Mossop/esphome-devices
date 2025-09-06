#pragma once

#include "esphome/core/component.h"
#include "esphome/core/time.h"
#include "esphome/components/http_request/http_request_idf.h"

namespace esphome {
namespace homeassistant_api {

bool parse_iso8601(const char* s, ESPTime& out);

class HomeAssistantApi : public Component {
  public:
    void setup() override;
    void loop() override;
    void dump_config() override;

    float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

    void set_http_request(http_request::HttpRequestComponent *http_request) { this->http_request = http_request; }
    void set_url(const std::string &url) {
      if (url.back() != '/') {
        this->url = url + '/';
      } else {
        this->url = url;
      }
    }

    void set_token(const std::string &token) { this->token = token; }

    bool state(const std::string &entity_id, const json::json_parse_t &cb);
    bool call_service(const std::string &domain, const std::string &service, const std::string &data = "");
    bool query_service(const std::string &domain, const std::string &service, const std::string &data, const json::json_parse_t &cb);

  private:
    http_request::HttpRequestComponent *http_request;
    std::string url;
    std::string token;
};

} //namespace homeassistant_api
} //namespace esphome
