#include <Matrixinfo.h>
#include <algorithm>

void MatrixInfo::clear() {
  changed.set();
  std::ranges::for_each(digits, [](auto& bs){ bs.reset(); });
}

void MatrixInfo::fill() {
  changed.set();
  std::ranges::for_each(digits, [](auto& bs){ bs.set(); });
}

uint8_t MatrixInfo::operator[](uint8_t index) { 
  return (uint8_t) digits[index].to_ulong(); 
}

void MatrixInfo::setState(uint8_t r, uint8_t c, bool state) {
  digits[r][c] = state;
  changed[r] = true;
}

bool MatrixInfo::getState(uint8_t r, uint8_t c) {
  return digits[r][c];
}

bool MatrixInfo::isChanged(uint8_t row) {
  return changed[row];
}

void MatrixInfo::clearChangedState() {
  changed.reset();
}

bool MatrixInfo::isMatrixFull() {
  auto checkBitset = [](std::bitset<COL_SIZE>& bs) -> bool { return bs.all(); };
  return std::all_of(digits.begin(), digits.end(), checkBitset);
}

bool MatrixInfo::isMatrixClear() {
  auto checkBitset = [](std::bitset<COL_SIZE>& bs) -> bool { return bs.none(); };
  return std::all_of(digits.begin(), digits.end(), checkBitset);
}

