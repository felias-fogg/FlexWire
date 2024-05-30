/* Flexible Wire drop-in replacement */

#ifndef FLEXWIRE_h
#define FLEXWIRE_h
#define FLEXWIRE_VERSION 1.2.1

// #define AVR_OPTIMIZATION 0 // without optimizations, less code, but much slower (55 kHz)

#include <Arduino.h>
#include <inttypes.h>
#if defined(ARDUINO_ARCH_AVR)
#include <util/atomic.h>
#ifndef AVR_OPTIMIZATION
#define AVR_OPTIMIZATION 1
#endif
#endif

#define I2C_READ 1
#define I2C_WRITE 0
#if AVR_OPTIMIZATION
#define I2C_DEFAULT_DELAY 3 // usec delay
#else
#define I2C_DEFAULT_DELAY 0 // usec delay
#endif

#ifdef ESP32
#define I2C_BUFFER_LENGTH 128
#else
#ifdef ARDUINO_ARCH_SAMD
#define BUFFER_LENGTH 250
#define I2C_BUFFER_LENGTH BUFFER_LENGTH
#else // ordinary AVRs
#define BUFFER_LENGTH 32
#define I2C_BUFFER_LENGTH BUFFER_LENGTH
#endif
#endif

#define I2C_MAXWAIT 5000

class FlexWire {
protected:
  uint8_t _rxBuffer[I2C_BUFFER_LENGTH];
  uint8_t _rxBufferIndex;
  uint8_t _rxBufferLength;
  uint8_t _transmitting;
  uint8_t _error;
  uint8_t _sda;
  uint8_t _scl;
  uint16_t _i2cDelay;
  bool _pullup;
  int8_t _sdastate;
  int8_t _sclstate;
#if AVR_OPTIMIZATION
  uint8_t _sdaBitMask;
  uint8_t _sclBitMask;

  volatile uint8_t *_sdaPortReg;
  volatile uint8_t *_sclPortReg;
  volatile uint8_t *_sdaDirReg;
  volatile uint8_t *_sclDirReg;
  volatile uint8_t *_sdaPinReg;
  volatile uint8_t *_sclPinReg;
#endif

  bool i2c_init(void);
  bool i2c_start(uint8_t addr);
  bool i2c_rep_start(uint8_t addr);
  void i2c_stop(void);
  bool i2c_write(uint8_t value);
  uint8_t i2c_read(bool last);
#if AVR_OPTIMIZATION
  inline void setSdaHigh(void); 
  inline void setSdaLow(void); 
  inline void setSclHigh(void);
  inline void setSclLow(void); 
#else
  void setSdaHigh(void);
  void setSdaLow(void);
  void setSclHigh(void);
  void setSclLow(void);
#endif
  inline uint8_t getSda(void) __attribute__((always_inline));
  inline uint8_t getScl(void) __attribute__((always_inline));
  
public:
  FlexWire(uint8_t sda = 0, uint8_t scl = 0, bool internal_pullup = false);
  void begin(void);
  void end(void);
  void setClock(uint32_t _);
  void setPins(uint8_t sda, uint8_t scl);
  void beginTransmission(uint8_t address);
  void beginTransmission(int address);
  uint8_t endTransmission(uint8_t sendStop);
  uint8_t endTransmission(void);
  size_t write(uint8_t data);
  size_t write(const uint8_t *data, size_t quantity);
  uint8_t requestFrom(uint8_t address, uint8_t quantity, bool sendStop = true);
  uint8_t requestFrom(int address, int quantity, bool sendStop = true);
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
