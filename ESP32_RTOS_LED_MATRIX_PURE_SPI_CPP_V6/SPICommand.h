#ifndef __SPI__COMMAND__H__
#define __SPI__COMMAND__H__

#include <SPI.h>
#include <utility>

class SPICommand {
  SPIClass& _spi;
  uint8_t _cs_pin;

  public:
  SPICommand(SPIClass& spi, uint8_t cs_pin) : _spi(spi), _cs_pin(cs_pin) { }
  void send(uint8_t command, uint8_t parameter);  
};

#endif // __SPI__COMMAND__H__