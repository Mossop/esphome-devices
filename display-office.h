#define DASHBOARD_MARGIN 5
#define DASHBOARD_SPACING 10

#define ICON_WIFI_CONNECTED "\U0000e63e"
#define ICON_WIFI_DISCONNECTED "\U0000f063"
#define ICON_FLIGHT "\U0000e6ca"
#define ICON_EVENT_CURRENT "\U0000e878"
#define ICON_EVENT_NEXT "\U0000f238"

#define KNOTS_TO_MPH 1.15078

namespace dashboard {

using namespace shared::display;

#define ICON_UNKNOWN "\U0000eb8b"

std::string format_time(ESPTime time) {
  return time.strftime("%H:%M");
}

bool same_day(const ESPTime &a, const ESPTime &b) {
  return a.year == b.year && a.day_of_year == b.day_of_year;
}

std::string format_day(ESPTime time) {
  if (same_day(time, id(rtc).now())) {
    return "";
  }

  ESPTime tomorrow = id(rtc).now();
  tomorrow.increment_day();
  if (same_day(time, tomorrow)) {
    return "Tomorrow";
  }

  return time.strftime("%A");
}

static const std::unordered_map<std::string, std::string> WEATHER_ICONS = {
  {"sunny", "\U0000e81a"},
  {"clear-night", "\U0000f036"},
  {"cloudy", "\U0000e2bd"},
  {"partlycloudy", "\U0000f172"},
  {"rainy", "\U0000f176"},
  {"pouring", "\U0000f61f"},
  {"snowy", "\U0000e2cd"},
  {"snowy-rainy", "\U0000f60b"},
  {"lightning", "\U0000ebdb"},
  {"lightning-rainy", "\U0000ebdb"},
  {"windy", "\U0000efd8"},
  {"windy-variant", "\U0000efd8"},
  {"fog", "\U0000e818"},
  {"hail", "\U0000f67f"}
};

static std::string condition_icon(const std::string &condition) {
  auto it = WEATHER_ICONS.find(condition);
  if (it != WEATHER_ICONS.end()) {
    return it->second;
  }
  return ICON_UNKNOWN;
}

void render_disconnected(esphome::display::Display & it) {
  if (id(rtc).now().is_valid()) {
    it.strftime(it.get_width() / 2, DASHBOARD_SPACING * 4 + DASHBOARD_MARGIN, &id(text64), TextAlign::TOP_CENTER, "%H:%M", id(rtc).now());
  }

  if (id(espnet).is_connected()) {
    it.printf(it.get_width() / 2, it.get_height() - (DASHBOARD_SPACING * 4 + DASHBOARD_MARGIN), &id(icons256), TextAlign::BOTTOM_CENTER, ICON_WIFI_CONNECTED);
  } else {
    it.printf(it.get_width() / 2, it.get_height() - (DASHBOARD_SPACING * 4 + DASHBOARD_MARGIN), &id(icons256), TextAlign::BOTTOM_CENTER, ICON_WIFI_DISCONNECTED);
  }
}

void render_hourly(esphome::display::Display & it, const weather_forecast::Forecast& forecast, int left, int right, int& y) {
  it.line(left + DASHBOARD_MARGIN, y, right - DASHBOARD_MARGIN, y);

  Icon icon(&id(icons64), condition_icon(forecast.condition));
  Bounds bounds = icon.render(it, left + DASHBOARD_SPACING, y + DASHBOARD_SPACING / 2, Anchor::Top_Left);

  Text time(
    &id(text24),
    "%s - %s",
    format_time(forecast.datetime).c_str(),
    format_time(ESPTime::from_epoch_local(forecast.datetime.timestamp + 3600)).c_str()
  );
  time.render(it, right - DASHBOARD_SPACING, bounds.top, Anchor::Top_Right);

  Text temp(&id(text24), "%.0f°", forecast.temperature);
  temp.render(it, right - DASHBOARD_SPACING, bounds.bottom, Anchor::Bottom_Right);

  y = bounds.bottom + DASHBOARD_MARGIN;
}

void render_daily(esphome::display::Display & it, const weather_forecast::Forecast& forecast, int left, int right, int& y) {
  it.line(left + DASHBOARD_MARGIN, y, right - DASHBOARD_MARGIN, y);

  Icon icon(&id(icons64), condition_icon(forecast.condition));
  Bounds bounds = icon.render(it, left + DASHBOARD_SPACING, y + DASHBOARD_SPACING / 2, Anchor::Top_Left);

  Text time(
    &id(text24),
    format_day(forecast.datetime).c_str()
  );
  time.render(it, right - DASHBOARD_SPACING, bounds.top, Anchor::Top_Right);

  Text temp(&id(text24), "%.0f°", forecast.temperature);
  temp.render(it, right - DASHBOARD_SPACING, bounds.bottom, Anchor::Bottom_Right);

  y = bounds.bottom + DASHBOARD_MARGIN;
}

void render_left(esphome::display::Display & it, int left, int right) {
  it.start_clipping(left + DASHBOARD_MARGIN, DASHBOARD_MARGIN, right - DASHBOARD_MARGIN, it.get_height() - DASHBOARD_MARGIN);

  int mid = (left + right) / 2;

  // Current time
  Text time(&id(text56), id(rtc).now().strftime("%H:%M"));
  Bounds bounds = time.render(it, mid, DASHBOARD_SPACING * 2, Anchor::Top_Center);
  int y = bounds.bottom + DASHBOARD_SPACING * 2;

  const std::vector<weather_forecast::Forecast> & hourly = id(weather_hourly).get_forecasts();

  if (hourly.size() >= 3) {
    render_hourly(it, hourly[0], left, right, y);
    render_hourly(it, hourly[1], left, right, y);
    render_hourly(it, hourly[2], left, right, y);
  }

  const std::vector<weather_forecast::Forecast> & daily = id(weather_daily).get_forecasts();
  if (daily.size() > 1) {
    render_daily(it, daily[1], left, right, y);
  }

  it.end_clipping();
}

void render_right(esphome::display::Display & it, int left, int right) {
  it.start_clipping(left + DASHBOARD_MARGIN, DASHBOARD_MARGIN, right - DASHBOARD_MARGIN, it.get_height() - DASHBOARD_MARGIN);

  int y = 0;
  int mid = (left + right) / 2;

  const std::vector<calendar_events::Event> & upcoming_events = id(events).get_events();
  ESPTime now = id(rtc).now();

  auto rit = std::find_if(upcoming_events.rbegin(), upcoming_events.rend(), [now](const calendar_events::Event &e){
    return e.start <= now && e.end > now;
  });

  std::vector<calendar_events::Event> tail(rit.base(), upcoming_events.end());

  if (rit != upcoming_events.rend()) {
    auto it = rit.base();
    tail.assign(it, upcoming_events.end());
  }

  if (rit != upcoming_events.rend()) {
    const calendar_events::Event &current_event = *rit;

    std::string start = format_time(current_event.start);
    std::string end = format_time(current_event.end);

    y += DASHBOARD_SPACING;

    Text summary(&id(text48), current_event.summary);
    Bounds bounds = summary.render(it, mid, y, Anchor::Top_Center);

    y = bounds.bottom + DASHBOARD_SPACING;

    Text time(&id(text24), "%s - %s", start.c_str(), end.c_str());
    bounds = time.render(it, mid, y, Anchor::Top_Center);

    y = bounds.bottom + DASHBOARD_SPACING;
    it.line(left + DASHBOARD_MARGIN, y, right - DASHBOARD_SPACING, y);
  }

  for (auto &next_event : tail) {
    if (y >= it.get_height()) {
      break;
    }

    if (next_event.end <= now) {
      continue;
    }

    std::string day = format_day(next_event.start);
    std::string start = format_time(next_event.start);
    std::string end = format_time(next_event.end);

    y += DASHBOARD_SPACING;

    Icon icon(&id(icons24), ICON_EVENT_NEXT);
    Text summary(&id(text24), next_event.summary);
    Row row(Align::Center, &id(text24));
    row.add(&icon);
    row.add(&summary);
    Bounds bounds = row.render(it, left + DASHBOARD_SPACING, y, Anchor::Top_Left);

    y = bounds.bottom + DASHBOARD_SPACING;

    Text time(&id(text16), "%s %s - %s", day.c_str(), start.c_str(), end.c_str());
    bounds = time.render(it, right - DASHBOARD_SPACING, y, Anchor::Top_Right);

    y = bounds.bottom + DASHBOARD_SPACING;
    it.line(left + DASHBOARD_MARGIN, y, right - DASHBOARD_MARGIN, y);
  }

  it.end_clipping();
}

void render_flight(esphome::display::Display & it, const flights::Flight& flight) {
  int left = DASHBOARD_SPACING * 10;
  int right = it.get_width() - DASHBOARD_SPACING * 10;
  int top = DASHBOARD_SPACING * 5;
  int bottom = it.get_height() - DASHBOARD_SPACING * 5;

  int mid = (left + right) / 2;
  int y = top;

  it.filled_rectangle(left, y, right - left, bottom - y, COLOR_OFF);
  it.rectangle(left, y, right - left, bottom - y);

  it.start_clipping(left + DASHBOARD_MARGIN, y + DASHBOARD_MARGIN, right - DASHBOARD_MARGIN, bottom - DASHBOARD_MARGIN);

  y += DASHBOARD_SPACING * 3;

  Icon icon(&id(icons64), ICON_FLIGHT);

  if (!flight.flight_number.empty()) {
    Text flight_number(&id(text64), flight.flight_number);
    Row row(Align::Baseline, &id(text64));
    row.add(&icon);
    row.add(&flight_number);

    row.render(it, mid, y, Anchor::Top_Center);

    y = y + row.height + DASHBOARD_SPACING * 3;
  } else {
    icon.render(it, mid, y, Anchor::Top_Center);
    y += icon.height + DASHBOARD_SPACING * 3;
  }

  Text stats(&id(text24), "%.0f ft, %.0f mph, %.1f km away", flight.altitude, flight.ground_speed * KNOTS_TO_MPH, flight.distance);
  Bounds bounds = stats.render(it, mid, y, Anchor::Top_Center);
  y = bounds.bottom + DASHBOARD_SPACING * 3;

  Text details(&id(text24), "%s (%s)", flight.aircraft_model.c_str(), flight.aircraft_registration.c_str());
  bounds = details.render(it, mid, y, Anchor::Top_Center);

  if (
    !flight.airport_origin_city.empty() &&
    !flight.airport_origin_country_name.empty() &&
    !flight.airport_destination_city.empty() &&
    !flight.airport_destination_country_name.empty()
  ) {
    Text origin_city(&id(text24), flight.airport_origin_city);
    Text origin_country(&id(text24), flight.airport_origin_country_name);
    Column origin(Align::Center, DASHBOARD_SPACING);
    origin.add(&origin_city);
    origin.add(&origin_country);
    Text destination_city(&id(text24), flight.airport_destination_city);
    Text destination_country(&id(text24), flight.airport_destination_country_name);
    Column destination(Align::Center, DASHBOARD_SPACING);
    destination.add(&destination_city);
    destination.add(&destination_country);

    y = bottom - DASHBOARD_SPACING * 3;

    it.start_clipping(left + DASHBOARD_MARGIN, top + DASHBOARD_MARGIN, mid - DASHBOARD_MARGIN, bottom - DASHBOARD_MARGIN);
    origin.render(it, (left + mid) / 2, y, Anchor::Bottom_Center);
    it.end_clipping();

    it.start_clipping(mid + DASHBOARD_MARGIN, top + DASHBOARD_MARGIN, right + DASHBOARD_MARGIN, bottom - DASHBOARD_MARGIN);
    destination.render(it, (mid + right) / 2, y, Anchor::Bottom_Center);
    it.end_clipping();
  }

  it.end_clipping();
}

void render_dashboard(esphome::display::Display & it) {
  if (!id(rtc).now().is_valid() || !id(espnet).is_connected()) {
    render_disconnected(it);
  } else {
    int split = (it.get_width() * 3) / 8;

    it.line(split, 0, split, it.get_height());

    render_left(it, 0, split);
    render_right(it, split, it.get_width());

    const std::vector<flights::Flight> & flights = id(local_flights).get_flights();

    auto flight = std::find_if(flights.begin(), flights.end(), [](const flights::Flight &f){
      return f.on_ground == 0 && f.distance < 3.0;
    });

    if (flight != flights.end()) {
      render_flight(it, *flight);
    }
  }
}

}
