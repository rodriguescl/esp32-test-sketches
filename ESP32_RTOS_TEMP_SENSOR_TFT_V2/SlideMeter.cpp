#include <SlideMeter.h>
#include <TFT_eSPI.h>
#include <algorithm>

void SlideMeter::drawHeader() {
	tft.fillRect(start_x, start_y, total_width, top_rect_height, headerBackgroundColor);
	tft.setTextColor(headerTextColor, headerBackgroundColor);
	tft.setTextDatum(MC_DATUM);
	tft.drawString(label, start_x + total_width / 2, start_y + top_rect_height / 2, headerFont);
}

void SlideMeter::drawBody() {
	tft.fillRect(start_x, start_y + top_rect_height, total_width, middle_rect_height, TFT_WHITE);
	uint16_t tick_gap = middle_rect_height / 12;
	float step_size = (max_reading - min_reading) / 10;
	uint16_t ticks_count = 0;
	tft.setTextColor(TFT_BLACK, TFT_WHITE);
	tft.setTextDatum(MR_DATUM);
	float current_reading = max_reading;
	bool show_Float = true;

	uint16_t temp_min_reading_y = min_reading_y;
	uint16_t temp_max_reading_y = max_reading_y;
	for(uint16_t y = (start_y + top_rect_height + tick_gap); y < (start_y + top_rect_height + middle_rect_height) && ticks_count < 11; y += tick_gap, ticks_count++) {
		temp_min_reading_y = std::min(temp_min_reading_y, y);
		temp_max_reading_y = std::max(temp_max_reading_y, y);
	}
	uint16_t delta_top = temp_min_reading_y - (start_y + top_rect_height);
	uint16_t delta_bottom = (start_y + top_rect_height + middle_rect_height) - temp_max_reading_y;
	uint16_t offset = 0;
	if(delta_bottom > delta_top) {
		offset = (delta_bottom - delta_top) / 2;
	}

	ticks_count = 0;
	for(uint16_t y = (start_y + top_rect_height + tick_gap + offset); y < (start_y + top_rect_height + middle_rect_height) && ticks_count < 11; y += tick_gap, ticks_count++) {
		min_reading_y = std::min(min_reading_y, y);
		max_reading_y = std::max(max_reading_y, y);
		tft.drawLine(start_x + total_width - 18, y, start_x + total_width - 5, y, TFT_BLACK);
		if(show_Float) {
			tft.drawFloat(current_reading, 0, start_x + total_width - 20, y, ticksFont);
		}
		show_Float = !show_Float;
		current_reading -= step_size;
	}
}

void SlideMeter::drawFooter() {
	tft.fillRect(start_x, start_y + top_rect_height + middle_rect_height, total_width, bottom_rect_height, footerBackgroundColor);
}

void SlideMeter::drawOutline() {
	tft.drawRect(start_x, start_y, total_width, top_rect_height + middle_rect_height + bottom_rect_height, TFT_BLACK);
}

SlideMeter::SlideMeter(TFT_eSPI& _tft, const char * _label, uint16_t _start_x, uint16_t _start_y, uint16_t _total_width, uint16_t _total_height, float _min_reading, float _max_reading) : 
tft(_tft), triangle_sprite(TFT_eSprite(&_tft)), reading_sprite(TFT_eSprite(&_tft)), label(_label), start_x(_start_x), start_y(_start_y), total_width(_total_width), total_height(_total_height),	min_reading(_min_reading), max_reading(_max_reading) {
	top_rect_height = total_height / 8;
	bottom_rect_height = top_rect_height;
	middle_rect_height = 6 * top_rect_height;
	min_reading_y = start_y + top_rect_height + middle_rect_height + bottom_rect_height;
	max_reading_y = 0;
	last_reading_y = 1000;
  triangle_sprite.createSprite(30, 24);
  triangle_sprite.fillScreen(TFT_WHITE);
  triangle_sprite.fillTriangle(0, 2, 0, 21, 29, 12, TFT_RED);
	reading_sprite.createSprite(total_width - 4, bottom_rect_height - 4);
	reading_sprite.fillScreen(footerBackgroundColor);
}

void SlideMeter::draw() {
	drawHeader();
	drawBody();
	drawFooter();
	drawOutline();
}

void SlideMeter::updateReading(float reading) {
	uint16_t reading_x = start_x + 2;
	reading_sprite.setTextColor(footerTextColor, footerBackgroundColor);
	reading_sprite.setTextDatum(MC_DATUM);
	reading_sprite.fillScreen(footerBackgroundColor);
	reading_sprite.drawFloat(reading, 2, (total_width - 4) / 2, (bottom_rect_height - 4) / 2, footerFont);
	reading_sprite.pushSprite(start_x + 2, start_y + top_rect_height + middle_rect_height + 2);
	uint16_t reading_y = map(reading, min_reading, max_reading, max_reading_y, min_reading_y);
	if(last_reading_y == 1000) {
		triangle_sprite.pushSprite(reading_x, reading_y - 12);
		last_reading_y = reading_y;
	} else {
		if (last_reading_y != reading_y) {
			uint16_t current_y = last_reading_y;
			uint16_t delta = last_reading_y > reading_y ? -1 : 1;
			while(reading_y != current_y) {
				triangle_sprite.pushSprite(reading_x, current_y - 12);
				vTaskDelay(pdMS_TO_TICKS(10));
				current_y += delta;
			}
			last_reading_y = current_y;
		}
	}    
}

void SlideMeter::updateReading() {
	updateReading(min_reading);
}
