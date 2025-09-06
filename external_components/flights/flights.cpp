#include "esphome/core/log.h"
#include "flights.h"

namespace esphome {
namespace flights {

static const char *TAG = "flights";

void Flights::setup() {
}

void Flights::loop() {
}

void Flights::update() {
  std::vector<Flight> found;

  bool success = this->homeassistant_api->state(entity_id, [&](JsonObject obj) {
    if (!obj["attributes"].is<JsonObject>()) {
      return false;
    }

    if (obj["attributes"]["flights"].is<JsonObject>()) {
      // Home Assistant returns a dictionary for an empty list.
      return true;
    }

    if (!obj["attributes"]["flights"].is<JsonArray>()) {
      return false;
    }

    JsonArray list = obj["attributes"]["flights"];
    for (JsonVariant flight : list) {
      if (flight.is<JsonObject>()) {
        JsonObject json_flight = flight;
        Flight f;

        f.id = json_flight["id"] | "";
        f.distance = json_flight["distance"] | NAN;
        f.longitude = json_flight["longitude"] | NAN;
        f.latitude = json_flight["latitude"] | NAN;
        f.heading = json_flight["heading"] | NAN;
        f.ground_speed = json_flight["ground_speed"] | NAN;
        f.altitude = json_flight["altitude"] | NAN;
        f.flight_number = json_flight["flight_number"] | "";
        f.airline = json_flight["airline"] | "";
        f.aircraft_model = json_flight["aircraft_model"] | "";
        f.aircraft_registration = json_flight["aircraft_registration"] | "";
        f.airport_origin_city = json_flight["airport_origin_city"] | "";
        f.airport_origin_country_name = json_flight["airport_origin_country_name"] | "";
        f.airport_destination_city = json_flight["airport_destination_city"] | "";
        f.airport_destination_country_name = json_flight["airport_destination_country_name"] | "";
        f.on_ground = json_flight["on_ground"] | 0;

        found.push_back(std::move(f));
      }
    }

    return true;
  });

  if (!success) {
    ESP_LOGW(TAG, "Failed to fetch flight information");
    return;
  }

  ESP_LOGD(TAG, "Found %u flight entries", found.size());

  std::sort(found.begin(), found.end());
  if (this->flights != found) {
    this->flights = std::move(found);
    this->state_callback.call();
  }
}

void Flights::dump_config() {
    ESP_LOGCONFIG(TAG, "Flights");
}

} //namespace flights
} //namespace esphome
