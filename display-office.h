#define DASHBOARD_MARGIN 5
#define DASHBOARD_SPACING 10

#define WIFI_CONNECTED "\U0000e63e"
#define WIFI_DISCONNECTED "\U0000f063"

namespace dashboard {

using namespace shared;

void render_disconnected(esphome::display::Display & it) {
  if (id(rtc).now().is_valid()) {
    it.strftime(it.get_width() / 2, DASHBOARD_SPACING * 4 + DASHBOARD_MARGIN, &id(text64), TextAlign::TOP_CENTER, "%H:%M", id(rtc).now());
  }

  if (id(espnet).is_connected()) {
    it.printf(it.get_width() / 2, it.get_height() - (DASHBOARD_SPACING * 4 + DASHBOARD_MARGIN), &id(icons256), TextAlign::BOTTOM_CENTER, WIFI_CONNECTED);
  } else {
    it.printf(it.get_width() / 2, it.get_height() - (DASHBOARD_SPACING * 4 + DASHBOARD_MARGIN), &id(icons256), TextAlign::BOTTOM_CENTER, WIFI_DISCONNECTED);
  }
}

void render_left(esphome::display::Display & it, int left, int right) {
  it.start_clipping(left + DASHBOARD_MARGIN, DASHBOARD_MARGIN, right - DASHBOARD_MARGIN, it.get_height() - DASHBOARD_MARGIN);

  int mid = (left + right) / 2;

  // Current time
  it.strftime(mid, DASHBOARD_SPACING * 2, text56, TextAlign::TOP_CENTER, "%H:%M", id(rtc).now());

  it.end_clipping();
}

std::string format_time(ESPTime time) {
  return time.strftime("%H:%M");
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
    Bounds bounds = summary.render(it, mid, y, TextAlign::TOP_CENTER);

    y = bounds.bottom + DASHBOARD_SPACING;

    Text time(&id(text24), "%s - %s", start.c_str(), end.c_str());
    bounds = time.render(it, mid, y, TextAlign::TOP_CENTER);

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

    std::string start = format_time(next_event.start);
    std::string end = format_time(next_event.end);

    y += DASHBOARD_SPACING;

    Text summary(&id(text24), next_event.summary);
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
  if (!id(rtc).now().is_valid() || !id(espnet).is_connected()) {
    render_disconnected(it);
  } else {
    int split = (it.get_width() * 3) / 8;

    it.line(split, 0, split, it.get_height());

    render_left(it, 0, split);
    render_right(it, split, it.get_width());
  }
}

}
