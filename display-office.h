namespace dashboard {

void render_dashboard(esphome::display::Display & it) {
  it.line(300, 0, 300, 480);

  it.start_clipping(10, 10, 290, 470);

  // Current time
  it.strftime(150, 20, &id(font56), TextAlign::TOP_CENTER, "%H:%M", id(rtc).now());

  it.end_clipping();

  it.start_clipping(310, 10, 790, 470);

  int y = 0;
  time_t ts;

  // Current event
  if (!id(current_event_summary).has_state()) {
    ESP_LOGD("dashboard", "Next event: '%s' %s - %s", id(current_event_summary).state.c_str(), id(current_event_start).state.c_str(), id(current_event_end).state.c_str());
    it.printf(550, y + 80, &id(font48), TextAlign::BOTTOM_CENTER, "%S", id(current_event_summary).state.c_str());

    ts = strtol(id(current_event_start).state.c_str(), nullptr, 10);
    std::string start_time = ESPTime::from_epoch_local(ts).strftime("%H:%M");
    ts = strtol(id(current_event_end).state.c_str(), nullptr, 10);
    std::string end_time = ESPTime::from_epoch_local(ts).strftime("%H:%M");
    it.printf(550, y + 90, &id(font24), TextAlign::TOP_CENTER, "%s - %s", start_time.c_str(), end_time.c_str());

    it.line(310, y + 130, 790, y + 130);

    y += 130;
  } else {
    ESP_LOGD("dashboard", "No current event");
  }

  // Next event
  ESP_LOGD("dashboard", "Next event: '%s' %s - %s", id(next_event_summary).state.c_str(), id(next_event_start).state.c_str(), id(next_event_end).state.c_str());
  it.printf(320, y + 40, &id(font24), TextAlign::BOTTOM_LEFT, "Next: %s", id(next_event_summary).state.c_str());

  ts = strtol(id(next_event_start).state.c_str(), nullptr, 10);
  std::string start_time = ESPTime::from_epoch_local(ts).strftime("%H:%M");
  ts = strtol(id(next_event_end).state.c_str(), nullptr, 10);
  std::string end_time = ESPTime::from_epoch_local(ts).strftime("%H:%M");
  it.printf(780, y + 40, &id(font16), TextAlign::TOP_RIGHT, "%s - %s", start_time.c_str(), end_time.c_str());

  it.line(310, y + 70, 790, y + 70);

  it.end_clipping();
}

}
