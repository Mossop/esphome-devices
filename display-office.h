#define DASHBOARD_MARGIN 5
#define DASHBOARD_SPACING 10

#define WIFI_CONNECTED "\U0000e63e"
#define WIFI_DISCONNECTED "\U0000f063"
#define API_CONNECTED "\U0000e833"

namespace dashboard {

using namespace shared;

void render_disconnected(esphome::display::Display & it) {
  if (id(rtc).now().is_valid()) {
    it.strftime(it.get_width() / 2, DASHBOARD_SPACING * 4 + DASHBOARD_MARGIN, &id(text64), TextAlign::TOP_CENTER, "%H:%M", id(rtc).now());
  }

  if (id(espnet).is_connected()) {
    if (id(espapi).is_connected()) {
      it.printf(it.get_width() / 2, it.get_height() - (DASHBOARD_SPACING * 4 + DASHBOARD_MARGIN), &id(icons256), TextAlign::BOTTOM_CENTER, API_CONNECTED);
    } else {
      it.printf(it.get_width() / 2, it.get_height() - (DASHBOARD_SPACING * 4 + DASHBOARD_MARGIN), &id(icons256), TextAlign::BOTTOM_CENTER, WIFI_CONNECTED);
    }
  } else {
    it.printf(it.get_width() / 2, it.get_height() - (DASHBOARD_SPACING * 4 + DASHBOARD_MARGIN), &id(icons256), TextAlign::BOTTOM_CENTER, WIFI_DISCONNECTED);
  }
}

class EventSensor {
  public:
    EventSensor(const TextSensor & summary_sensor, const TextSensor & start_sensor, const TextSensor & end_sensor) {
      start = from_timestamp(start_sensor.state);
      end = from_timestamp(end_sensor.state);
      summary = summary_sensor.state;
      is_valid = summary_sensor.has_state() && start_sensor.has_state() && end_sensor.has_state();
    }

    bool is_valid;
    ESPTime start;
    ESPTime end;
    std::string summary;
};

void render_left(esphome::display::Display & it, int left, int right) {
  it.start_clipping(left + DASHBOARD_MARGIN, DASHBOARD_MARGIN, right - DASHBOARD_MARGIN, it.get_height() - DASHBOARD_MARGIN);

  int mid = (left + right) / 2;

  // Current time
  it.strftime(mid, DASHBOARD_SPACING * 2, text56, TextAlign::TOP_CENTER, "%H:%M", id(rtc).now());

  it.end_clipping();
}

void render_right(esphome::display::Display & it, int left, int right) {
  it.start_clipping(left + DASHBOARD_MARGIN, DASHBOARD_MARGIN, right - DASHBOARD_MARGIN, it.get_height() - DASHBOARD_MARGIN);

  int y = 0;
  int mid = (left + right) / 2;

  EventSensor current_event(id(current_event_summary), id(current_event_start), id(current_event_end));

  // Current event
  if (current_event.is_valid) {
    std::string start = current_event.start.strftime("%H:%M");
    std::string end = current_event.end.strftime("%H:%M");

    y += DASHBOARD_SPACING;

    Text summary(&id(text48), current_event.summary);
    Bounds bounds = summary.render(it, mid, y, TextAlign::TOP_CENTER);

    y = bounds.bottom + DASHBOARD_SPACING;

    Text time(&id(text24), "%s - %s", start.c_str(), end.c_str());
    bounds = time.render(it, mid, y, TextAlign::TOP_CENTER);

    y = bounds.bottom + DASHBOARD_SPACING;
    it.line(left + DASHBOARD_MARGIN, y, right - DASHBOARD_SPACING, y);
  }

  EventSensor next_event(id(next_event_summary), id(next_event_start), id(next_event_end));

  // Next event
  if (next_event.is_valid) {
    std::string start = next_event.start.strftime("%H:%M");
    std::string end = next_event.end.strftime("%H:%M");

    y += DASHBOARD_SPACING;

    Text summary(&id(text24), "Next: %s", next_event.summary.c_str());
    Bounds bounds = summary.render(it, left + DASHBOARD_SPACING, y, TextAlign::TOP_LEFT);

    y = bounds.bottom + DASHBOARD_SPACING;

    Text time(&id(text16), "%s - %s", start.c_str(), end.c_str());
    bounds = time.render(it, right - DASHBOARD_SPACING, y, TextAlign::TOP_RIGHT);

    y = bounds.bottom + DASHBOARD_SPACING;
    it.line(left + DASHBOARD_MARGIN, y, right - DASHBOARD_SPACING, y);
  }

  it.end_clipping();
}

void render_dashboard(esphome::display::Display & it) {
  if (!id(rtc).now().is_valid() || !id(espapi).is_connected()) {
    render_disconnected(it);
  } else {
    int split = (it.get_width() * 3) / 8;

    it.line(split, 0, split, it.get_height());

    render_left(it, 0, split);
    render_right(it, split, it.get_width());
  }
}

}
