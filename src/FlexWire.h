/* Flexible Wire drop-in replacement */

#ifndef FLEXWIRE
#define FLEXWIRE

#include <Arduino.h>
#include <inttypes.h>

#define FLEXWIRE_VERSION 0.0.1
#define I2C_READ 1
#define I2C_WRITE 0
#define DELAY 4 // usec delay
#define BUFFER_LENGTH 32
#define I2C_MAXWAIT 5000

class FlexWire {
protected:
  uint8_t _rxBuffer[BUFFER_LENGTH];
  uint8_t _rxBufferIndex;
  uint8_t _rxBufferLength;
  uint8_t _transmitting;
  uint8_t _error;
  uint8_t _sda;
  uint8_t _scl;
  bool _pullup;

  bool i2c_init(void);
  bool i2c_start(uint8_t addr);
  bool i2c_start_wait(uint8_t addr);
  bool i2c_rep_start(uint8_t addr);
  void i2c_stop(void);
  bool i2c_write(uint8_t value);
  uint8_t i2c_read(bool last);
  void setHigh(uint8_t pin);
  void setLow(uint8_t pin);
  
public:
  FlexWire(uint8_t sda, uint8_t scl);
  FlexWire(uint8_t sda, uint8_t scl, bool internal_pullup);
  void begin(void);
  void end(void);
  void setClock(uint32_t _);
  void beginTransmission(uint8_t address);
  void beginTransmission(int address);
  uint8_t endTransmission(uint8_t sendStop);
  uint8_t endTransmission(void);
  size_t write(uint8_t data);
  size_t write(const uint8_t *data, size_t quantity);
  uint8_t requestFrom(uint8_t address, uint8_t quantity,
		      uint32_t iaddress, uint8_t isize, uint8_t sendStop);
  uint8_t requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop);
  uint8_t requestFrom(int address, int quantity, int sendStop);
  uint8_t requestFrom(uint8_t address, uint8_t quantity);
  uint8_t requestFrom(int address, int quantity);
  int available(void);
  int read(void);
  int peek(void);
  void flush(void);
  inline size_t write(unsigned long n) { return write((uint8_t)n); }
  inline size_t write(long n) { return write((uint8_t)n); }
  inline size_t write(unsigned int n) { return write((uint8_t)n); }
  inline size_t write(int n) { return write((uint8_t)n); }
};

#endif
