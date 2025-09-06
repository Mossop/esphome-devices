#include "esphome/core/log.h"
#include "weather_forecast.h"

namespace esphome {
namespace weather_forecast {

static const char *TAG = "weather_forecast";

void WeatherForecast::setup() {
}

void WeatherForecast::loop() {
}

void WeatherForecast::update() {
  std::string data = json::build_json([this](JsonObject root) {
    root["entity_id"] = this->entity_id;
    root["type"] = this->type;
  });

  std::vector<Forecast> found;

  bool success = this->homeassistant_api->query_service("weather", "get_forecasts", data, [&](JsonObject obj) {
    for (JsonPair kv : obj) {
      if (kv.value().is<JsonObject>() && kv.value()["forecast"].is<JsonArray>()) {
        JsonArray list = kv.value()["forecast"];
        for (JsonVariant forecast : list) {
          if (forecast.is<JsonObject>()) {
            JsonObject json_forecast = forecast;
            Forecast f;

            if (!homeassistant_api::parse_iso8601(json_forecast["datetime"], f.datetime)) {
                continue;
            }

            f.condition = json_forecast["condition"] | "";
            f.temperature = json_forecast["temperature"] | NAN;
            f.pressure = json_forecast["pressure"] | NAN;
            f.cloud_coverage = json_forecast["cloud_coverage"] | NAN;
            f.wind_speed = json_forecast["wind_speed"] | NAN;
            f.wind_bearing = json_forecast["wind_bearing"] | NAN;
            f.uv_index = json_forecast["uv_index"] | NAN;
            f.precipitation_probability = json_forecast["precipitation_probability"] | NAN;
            f.precipitation = json_forecast["precipitation"] | NAN;
            f.apparent_temperature = json_forecast["apparent_temperature"] | NAN;
            f.dew_point = json_forecast["dew_point"] | NAN;
            f.wind_gust_speed = json_forecast["wind_gust_speed"] | NAN;
            f.humidity = json_forecast["humidity"] | NAN;

            found.push_back(std::move(f));
          }
        }
      }
    }

    return true;
  });

  if (!success) {
    ESP_LOGW(TAG, "Failed to fetch weather forecast");
    return;
  }

  ESP_LOGD(TAG, "Found %u forecast entries", found.size());

  std::sort(found.begin(), found.end());
  if (this->forecasts != found) {
    this->forecasts = std::move(found);
    this->state_callback.call();
  }
}

void WeatherForecast::dump_config() {
    ESP_LOGCONFIG(TAG, "Weather Forecast");
}

} //namespace weather_forecast
} //namespace esphome
