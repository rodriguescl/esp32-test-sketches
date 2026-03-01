#include <TFT_eSPI.h>

#ifndef __SLIDE_METER__
#define __SLIDE_METER__

class SlideMeter {
  TFT_eSPI& tft;
  const char * label;
  uint16_t start_x;
  uint16_t start_y;
  uint16_t total_width;
  uint16_t total_height;
  uint16_t top_rect_height;
  uint16_t bottom_rect_height;
  uint16_t middle_rect_height;
  float min_reading;
  float max_reading;
  uint16_t min_reading_y;
  uint16_t max_reading_y;
  uint16_t last_reading_y;
	TFT_eSprite triangle_sprite;
	TFT_eSprite reading_sprite;
  uint8_t ticksFont = 1;
  uint8_t headerFont = 2;
  uint8_t footerFont = 2;
  uint16_t headerTextColor = TFT_WHITE;
  uint16_t headerBackgroundColor = TFT_BLACK;
  uint16_t footerTextColor = TFT_WHITE;
  uint16_t footerBackgroundColor = TFT_BLACK;

  void drawHeader();
  void drawBody();
  void drawFooter();
  void drawOutline();

  public:
  SlideMeter(TFT_eSPI& _tft, const char * _label, uint16_t _start_x, uint16_t _start_y, uint16_t _total_width, uint16_t _total_height, float _min_reading, float _max_reading);
  // Other constructors and assignment deleted just because I didn't put any thought on ho I wanted them to behave
  // Can be implemented is a use case for copying or moving objects of this class
  SlideMeter(const SlideMeter& other) = delete;
  SlideMeter& operator=(const SlideMeter& other) = delete;
  SlideMeter(const SlideMeter&& other) = delete;
  SlideMeter& operator=(const SlideMeter&& other) = delete;

  void draw();
  void updateReading(float reading);
  void updateReading();
};

#endif