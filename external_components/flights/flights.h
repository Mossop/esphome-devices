#pragma once

#include "esphome/core/component.h"
#include "esphome/core/time.h"
#include "esphome/components/http_request/http_request_idf.h"
#include "esphome/components/homeassistant_api/homeassistant_api.h"

namespace esphome {
namespace flights {

struct Flight {
  std::string id;
  float distance;
  float longitude;
  float latitude;
  float heading;
  float ground_speed;
  float altitude;
  int on_ground;
  std::string flight_number;
  std::string airline;
  std::string aircraft_model;
  std::string aircraft_registration;
  std::string airport_origin_city;
  std::string airport_origin_country_name;
  std::string airport_destination_city;
  std::string airport_destination_country_name;

  bool operator==(const Flight &other) const {
    return id == other.id;
  }

  bool operator<(const Flight &other) const {
    return distance < other.distance;
  }
};

class Flights : public PollingComponent {
  public:
    void setup() override;
    void loop() override;
    void update() override;
    void dump_config() override;

    float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

    const std::vector<Flight> &get_flights() const { return this->flights; }

    void add_on_state_callback(std::function<void()> &&callback) {
      this->state_callback.add(std::move(callback));
    }

    void set_homeassistant_api(homeassistant_api::HomeAssistantApi *homeassistant_api) { this->homeassistant_api = homeassistant_api; }
    void set_entity_id(const std::string &entity_id) { this->entity_id = entity_id; }

  private:
    std::vector<Flight> flights;
    homeassistant_api::HomeAssistantApi *homeassistant_api;
    CallbackManager<void()> state_callback{};
    std::string entity_id;
};

class UpdateTrigger : public Trigger<> {
 public:
  explicit UpdateTrigger(Flights *parent) {
    parent->add_on_state_callback([this]() { this->trigger(); });
  }
};

} //namespace flights
} //namespace esphome
