#include <LedMatrixProcessor.h>

void LedMatrixProcessor::flushBuffer() {
  for (uint8_t i = 0; i < _dev.getRows(); i++)
  {
    if (_dev.isChanged(i)) {
      sendCommand(OpCode::OP_DIGIT0, i, _dev[i]);
    }
  }
  _dev.clearChangedState();
}

void LedMatrixProcessor::initializeMatrix() {
    sendCommand(OpCode::OP_DISPLAYTEST, 0);
    sendCommand(OpCode::OP_INTENSITY, 1);
    sendCommand(OpCode::OP_SCANLIMIT, _dev.getRows() - 1);
    sendCommand(OpCode::OP_DECODEMODE, 0);
    clear();
    sendCommand(OpCode::OP_SHUTDOWN, 1);
}  

void LedMatrixProcessor::clear() {
  _dev.clear();
  flushBuffer();
}

void LedMatrixProcessor::fill() {
  _dev.fill();
  flushBuffer();
}

void LedMatrixProcessor::on() {
  sendCommand(OpCode::OP_SHUTDOWN, 1);
}

void LedMatrixProcessor::off() {
  sendCommand(OpCode::OP_SHUTDOWN, 0);
}

bool LedMatrixProcessor::getLedState(uint8_t row, uint8_t col) {
  return _dev.getState(row, col);
}

void LedMatrixProcessor::setLedState(uint8_t row, uint8_t col, bool state) {
    if(_dev.getState(row, col) != state) {
      _dev.setState(row, col, state);
      flushBuffer();
    }
}

bool LedMatrixProcessor::isLedMatrixFull() {
  return _dev.isMatrixFull();
}

bool LedMatrixProcessor::isLedMatrixClear() {
  return _dev.isMatrixClear();
}

void LedMatrixProcessor::begin() {
  pinMode(CS_PIN, OUTPUT);
  SPI.begin();
  digitalWrite(CS_PIN, HIGH);
  initializeMatrix();
}

void LedMatrixProcessor::sendCommand(OpCode command, uint8_t parameter) {
  if(command == OpCode::OP_INTENSITY) {
    parameter = std::min(parameter, _dev.getMaxIntensity());
  }
  _command.send(static_cast<uint8_t>(command), parameter);
}

void LedMatrixProcessor::sendCommand(OpCode command, uint8_t offset, uint8_t parameter) {
  if(command == OpCode::OP_DIGIT0) {
    _command.send(static_cast<uint8_t>(command)+offset, parameter);      
  }
}

