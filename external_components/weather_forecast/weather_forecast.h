#pragma once

#include "esphome/core/component.h"
#include "esphome/core/time.h"
#include "esphome/components/http_request/http_request_idf.h"
#include "../homeassistant_api/homeassistant_api.h"

namespace esphome {
namespace weather_forecast {

struct Forecast {
  ESPTime datetime;
  std::string condition;
  float temperature;
  float pressure;
  float cloud_coverage;
  float wind_speed;
  float wind_bearing;
  float uv_index;
  float precipitation_probability;
  float precipitation;
  float apparent_temperature;
  float dew_point;
  float wind_gust_speed;
  float humidity;

  bool operator==(const Forecast &other) const {
    return datetime == other.datetime;
  }

  bool operator<(const Forecast &other) const {
    return datetime < other.datetime;
  }
};

class WeatherForecast : public PollingComponent {
  public:
    void setup() override;
    void loop() override;
    void update() override;
    void dump_config() override;

    float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

    const std::vector<Forecast> &get_forecasts() const { return this->forecasts; }

    void add_on_state_callback(std::function<void()> &&callback) {
      this->state_callback.add(std::move(callback));
    }

    void set_homeassistant_api(homeassistant_api::HomeAssistantApi *homeassistant_api) { this->homeassistant_api = homeassistant_api; }
    void set_entity_id(const std::string &entity_id) { this->entity_id = entity_id; }
    void set_type(const std::string &type) { this->type = type; }

  private:
    std::vector<Forecast> forecasts;
    homeassistant_api::HomeAssistantApi *homeassistant_api;
    CallbackManager<void()> state_callback{};
    std::string entity_id;
    std::string type;
};

class UpdateTrigger : public Trigger<> {
 public:
  explicit UpdateTrigger(WeatherForecast *parent) {
    parent->add_on_state_callback([this]() { this->trigger(); });
  }
};

} //namespace weather_forecast
} //namespace esphome
