#pragma once

#include "esphome/components/display/display.h"
#include "esphome/components/font/font.h"

namespace shared {
namespace display {

enum class Align {
  Start,
  Center,
  End,
  Baseline
};

enum class Anchor {
  Top_Left,
  Top_Center,
  Top_Right,
  Center_Left,
  Center,
  Center_Right,
  Bottom_Left,
  Bottom_Center,
  Bottom_Right
};

struct Bounds {
  int left;
  int top;
  int right;
  int bottom;

  int width() const { return right - left; }
  int height() const { return bottom - top; }
};

int font_space(esphome::font::Font * font);

class Renderable {
  public:
    int width;
    int height;
    int baseline;

    virtual Bounds render(esphome::display::Display & it, int x, int y, Anchor align) const = 0;
};

class Icon : public Renderable {
  public:
    Icon(esphome::font::Font * font, const std::string & icon);

    Bounds render(esphome::display::Display & it, int x, int y, Anchor align) const;

  private:
    std::string icon;
    esphome::font::Font * font;
    int xoffset;
};

class Text : public Renderable {
  public:
    Text(esphome::font::Font * font, const std::string & text);
    Text(esphome::font::Font * font, const char* fmt, ...);

    Bounds render(esphome::display::Display & it, int x, int y, Anchor align) const;

  private:
    std::string text;
    esphome::font::Font * font;
    int xoffset;
};

class Row : public Renderable {
  public:
    Row(Align align, esphome::font::Font * font);

    void add(Renderable * item);

    Bounds render(esphome::display::Display & it, int x, int y, Anchor align) const;

  protected:
    Align align;
    int spacing;
    int above_baseline = 0;
    int below_baseline = 0;
    std::vector<Renderable*> items;

    int item_offset(const Renderable* item) const;
};

class CroppedRow : public Row {
  public:
    CroppedRow(Align align, esphome::font::Font * font, int max_width, int cropped_margin = 5);

    Bounds render(esphome::display::Display & it, int x, int y, Anchor align) const;

  protected:
    int max_width;
    int cropped_margin;
    esphome::font::Font * font;
};

class Column : public Renderable {
  public:
    Column(Align align, int spacing);

    void add(Renderable * item);

    Bounds render(esphome::display::Display & it, int x, int y, Anchor align) const;

  private:
    Align align;
    int spacing;
    std::vector<Renderable*> items;
};

}
}
