#include <SPICommand.h>
#include <Arduino.h>

void SPICommand::send(uint8_t command, uint8_t parameter) {
  _spi.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs_pin, LOW);
  _spi.transfer(command);
  _spi.transfer(parameter);
  digitalWrite(_cs_pin, HIGH);
  _spi.endTransaction();
}
