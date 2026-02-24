#ifndef __LED_MATRIX_PROCESSOR__H__
#define __LED_MATRIX_PROCESSOR__H__

#include <SPI.h> 
#include <SPICommand.h>
#include <Matrixinfo.h>
#include <Arduino.h>

class LedMatrixProcessor {
  enum class OpCode : uint8_t {
    OP_DIGIT0 = 1,
    OP_INTENSITY = 10,
    OP_DISPLAYTEST = 15,
    OP_DECODEMODE = 9,
    OP_SCANLIMIT = 11,
    OP_SHUTDOWN = 12
  };
  const uint8_t CS_PIN = 21;
  SPIClass& _spi;
  MatrixInfo _dev;
  SPICommand _command;

  void flushBuffer();
  void initializeMatrix();
  void sendCommand(OpCode command, uint8_t parameter);
  void sendCommand(OpCode command, uint8_t offset, uint8_t parameter);

  public:
  LedMatrixProcessor() : _spi(SPI), _dev(MatrixInfo{}), _command(SPICommand(_spi, CS_PIN)) { }
  void clear();
  void fill();
  void on();
  void off();
  bool getLedState(uint8_t row, uint8_t col);
  void setLedState(uint8_t row, uint8_t col, bool state);
  bool isLedMatrixFull();
  bool isLedMatrixClear();
  void begin();
};

#endif // __LED_MATRIX_PROCESSOR__H__