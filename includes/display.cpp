#include "display.h"

namespace shared {
namespace display {

using namespace esphome::display;

int font_space(esphome::font::Font * font) {
  int width, xoffset, baseline, height;
  font->measure(" ", &width, &xoffset, &baseline, &height);
  return width;
}

static void translate(int &x, int &y, const Renderable* renderable, Anchor& anchor) {
  switch (anchor) {
    case Anchor::Top_Right:
    case Anchor::Center_Right:
    case Anchor::Bottom_Right:
      x -= renderable->width;
      break;
    case Anchor::Top_Center:
    case Anchor::Center:
    case Anchor::Bottom_Center:
      x -= renderable->width / 2;
      break;
  }

  switch (anchor) {
    case Anchor::Bottom_Left:
    case Anchor::Bottom_Center:
    case Anchor::Bottom_Right:
      y -= renderable->height;
      break;
    case Anchor::Center_Left:
    case Anchor::Center:
    case Anchor::Center_Right:
      y -= renderable->height / 2;
      break;
  }
}

Icon::Icon(esphome::font::Font * font, const std::string & icon)
    : icon(icon), font(font) {
  font->measure(icon.c_str(), &width, &xoffset, &baseline, &height);
  height = baseline;
}

Bounds Icon::render(esphome::display::Display & it, int x, int y, Anchor align) const {
  translate(x, y, this, align);
  it.printf(x, y, font, TextAlign::TOP_LEFT, icon.c_str());
  return Bounds { x, y, x + width, y + height };
}

Text::Text(esphome::font::Font * font, const std::string & text)
    : text(text), font(font) {
  font->measure(text.c_str(), &width, &xoffset, &baseline, &height);
}

Text::Text(esphome::font::Font * font, const char* fmt, ...)
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

Bounds Text::render(esphome::display::Display & it, int x, int y, Anchor align) const {
  translate(x, y, this, align);
  it.printf(x, y, font, TextAlign::TOP_LEFT, text.c_str());
  return Bounds { x, y, x + width, y + height };
}

Row::Row(Align align, int spacing)
    : align(align), spacing(spacing) {
  width = 0;
  height = 0;
}

Row::Row(Align align, esphome::font::Font * font)
    : align(align), spacing(font_space(font)) {
  width = 0;
  height = 0;
}

void Row::add(Renderable * item) {
  items.push_back(item);
  width += item->width;

  above_baseline = std::max(above_baseline, item->baseline);
  below_baseline = std::max(below_baseline, item->height - item->baseline);

  if (align == Align::Baseline) {
    height = above_baseline + below_baseline;
  } else {
    height = std::max(height, item->height);
  }

  if (items.size() > 1) {
    width += spacing;
  }
}

Bounds Row::render(esphome::display::Display & it, int x, int y, Anchor align) const {
  translate(x, y, this, align);

  for (auto item : items) {
    int item_y;

    switch (this->align) {
      case Align::Start:
        item_y = y;
        break;
      case Align::Center:
        item_y = y + (height - item->height) / 2;
        break;
      case Align::End:
        item_y = y + (height - item->height);
        break;
      case Align::Baseline:
        item_y = y + (above_baseline - item->baseline) / 2;
        break;
    }

    item->render(it, x, item_y, Anchor::Top_Left);
    x += item->width + spacing;
  }

  return Bounds { x, y, x + width, y + height };
}

Column::Column(Align align, int spacing)
    : align(align), spacing(spacing) {
  width = 0;
  height = 0;

  if (align == Align::Baseline) {
    ESP_LOGE("display", "Column does not support Baseline alignment");
    this->align = Align::End;
  }
}

void Column::add(Renderable * item) {
  items.push_back(item);
  width = std::max(width, item->width);
  height += item->height;

  if (items.size() > 1) {
    height += spacing;
  }
}

Bounds Column::render(esphome::display::Display & it, int x, int y, Anchor align) const {
  translate(x, y, this, align);

  for (auto item : items) {
    int item_x;

    switch (this->align) {
      case Align::Start:
        item_x = x;
        break;
      case Align::Center:
        item_x = x + (width - item->width) / 2;
        break;
      case Align::Baseline:
      case Align::End:
        item_x = x + (width - item->width);
        break;
    }

    item->render(it, item_x, y, Anchor::Top_Left);
    y += item->height + spacing;
  }

  return Bounds { x, y, x + width, y + height };
}

}
}
