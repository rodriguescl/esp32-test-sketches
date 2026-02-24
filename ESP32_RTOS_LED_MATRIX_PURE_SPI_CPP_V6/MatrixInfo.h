#ifndef __MATRIX__INFO__H__
#define __MATRIX__INFO__H__

#include <bitset>
#include <array>

class MatrixInfo {
  static const uint8_t ROW_SIZE = 8;
  static const uint8_t COL_SIZE = 8;
  static const uint8_t MAX_INTENSITY = 0xf;
  std::array<std::bitset<COL_SIZE>, ROW_SIZE> digits;
  std::bitset<8> changed;
  
  public:
  void clear();
  void fill();
  uint8_t operator[](uint8_t index);
  void setState(uint8_t r, uint8_t c, bool state);
  bool getState(uint8_t r, uint8_t c);
  bool isChanged(uint8_t row);
  void clearChangedState();
  bool isMatrixFull();
  bool isMatrixClear();
  uint8_t getRows() { return ROW_SIZE; }
  uint8_t getCols() { return COL_SIZE; }
  uint8_t getMaxIntensity() { return MAX_INTENSITY; }
};

#endif // __MATRIX__INFO__H__