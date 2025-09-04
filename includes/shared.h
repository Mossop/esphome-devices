namespace shared {

ESPTime from_timestamp(const std::string & timestamp) {
  time_t ts = strtol(timestamp.c_str(), nullptr, 10);
  return ESPTime::from_epoch_local(ts);
}

struct Bounds {
  int left;
  int top;
  int right;
  int bottom;

  int width() const { return right - left; }
  int height() const { return bottom - top; }
};

class Text {
  public:
    Text(esphome::font::Font * font, const std::string & text)
        : text(text), font(font) {
      font->measure(text.c_str(), &width, &xoffset, &baseline, &height);
    }

    Text(esphome::font::Font * font, const char* fmt, ...)
        : font(font) {
      va_list args;
      va_start(args, fmt);
      size_t size = vsnprintf(nullptr, 0, fmt, args) + 1;
      va_end(args);

      std::vector<char> buf(256);
      va_start(args, fmt);
      vsnprintf(buf.data(), buf.size(), fmt, args);
      va_end(args);

      text = buf.data();

      font->measure(text.c_str(), &width, &xoffset, &baseline, &height);
    }

    Bounds measure(esphome::display::Display & it, int x, int y, TextAlign align) const {
      Bounds bounds;

      int width;
      int height;
      it.get_text_bounds(x, y, text.c_str(), font, align, &bounds.left, &bounds.top, &width, &height);
      bounds.right = bounds.left + width;
      bounds.bottom = bounds.top + height;

      return bounds;
    }

    Bounds render(esphome::display::Display & it, int x, int y, TextAlign align) const {
      it.printf(x, y, font, align, text.c_str());
      return measure(it, x, y, align);
    }

    int width;
    int height;
    int xoffset;
    int baseline;

  private:
    std::string text;
    esphome::font::Font * font;
};

}
