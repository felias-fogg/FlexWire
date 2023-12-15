/* 
   FlexWire.cpp - A Wire-like library that can act as a dropin replacement for the Wire lib
   
   It is derived from the Arduino Wire library and for this reason is
   published under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This version is basically a merger of the SlowSoftI2CMaster and SlowSoftWire libs. 
*/


#include <FlexWire.h>

FlexWire::FlexWire(uint8_t sda, uint8_t scl, bool internal_pullup):
  _i2cDelay(I2C_DEFAULT_DELAY),
  _pullup(internal_pullup)  {
  setPins(sda, scl);
}

void FlexWire::begin(void) {
  _rxBufferIndex = 0;
  _rxBufferLength = 0;
  _error = 0;
  _transmitting = false;
  _i2cDelay = I2C_DEFAULT_DELAY;
  i2c_init();
}
  
void  FlexWire::setClock(uint32_t Hz) {
#if AVR_OPTIMIZATION
  if (Hz <= 10000UL) _i2cDelay = 90;
  else if (Hz <= 30000UL) _i2cDelay = 30;
  else if (Hz <= 200000UL) _i2cDelay = 5;
  else _i2cDelay = 0;
#endif
}

void FlexWire::setPins(uint8_t sda, uint8_t scl)
{
  _sda = sda;
  _scl = scl;
#if AVR_OPTIMIZATION
  uint8_t port;

  port = digitalPinToPort(_sda);
  _sdaBitMask  = digitalPinToBitMask(_sda);
  _sdaPortReg  = portOutputRegister(port);
  _sdaDirReg   = portModeRegister(port);
  _sdaPinReg   = portInputRegister(port);      // PinReg is the input register, not the Arduino pin.

  port = digitalPinToPort(_scl);
  _sclBitMask  = digitalPinToBitMask(_scl);
  _sclPortReg  = portOutputRegister(port);
  _sclDirReg   = portModeRegister(port);
  _sclPinReg   = portInputRegister(port);
#endif
}

void  FlexWire::beginTransmission(uint8_t address) {
  if (_transmitting) {
    _error = (i2c_rep_start((address<<1)|I2C_WRITE) ? 0 : 2);
  } else {
    _error = (i2c_start((address<<1)|I2C_WRITE) ? 0 : 2);
  }
  // indicate that we are transmitting
  _transmitting = 1;
}
  
void  FlexWire::beginTransmission(int address) {
  beginTransmission((uint8_t)address);
}

uint8_t  FlexWire::endTransmission(uint8_t sendStop)
{
  uint8_t transError = _error;
  if (sendStop) {
    i2c_stop();
    _transmitting = 0;
  }
  _error = 0;
  return transError;
}

//	This provides backwards compatibility with the original
//	definition, and expected behaviour, of endTransmission
//
uint8_t  FlexWire::endTransmission(void)
{
  return endTransmission(true);
}

size_t  FlexWire::write(uint8_t data) {
  if (i2c_write(data)) {
    return 1;
  } else {
    if (_error == 0) _error = 3;
    return 0;
  }
}

size_t  FlexWire::write(const uint8_t *data, size_t quantity) {
  size_t trans = 0;
  for(size_t i = 0; i < quantity; ++i){
    trans += write(data[i]);
  }
  return trans;
}

uint8_t FlexWire::requestFrom(uint8_t address, uint8_t quantity, bool sendStop) {
  uint8_t localerror = 0;
  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  localerror = !i2c_rep_start((address<<1) | I2C_READ);
  if (_error == 0 && localerror) _error = 2;
  // perform blocking read into buffer
  if (!localerror) {
    for (uint8_t cnt=0; cnt < quantity; cnt++) 
      _rxBuffer[cnt] = i2c_read(cnt == quantity-1);
  } else {
    quantity = 0;
  }
  // set rx buffer iterator vars
  _rxBufferIndex = 0;
  _rxBufferLength = quantity;
  if (sendStop || _error != 0) {
    _transmitting = 0;
    i2c_stop();
  }
  return quantity;
}

uint8_t FlexWire::requestFrom(int address, int quantity, bool sendStop) {
   return requestFrom((uint8_t)address, (uint8_t)quantity, sendStop);
}
   
int FlexWire::available(void) {
  return _rxBufferLength - _rxBufferIndex;
}

int FlexWire::read(void) {
  int value = -1;
  if(_rxBufferIndex < _rxBufferLength){
    value = _rxBuffer[_rxBufferIndex];
    ++_rxBufferIndex;
  }
  return value;
}

int FlexWire::peek(void) {
  int value = -1;
    
  if(_rxBufferIndex < _rxBufferLength){
    value = _rxBuffer[_rxBufferIndex];
  }
  return value;
}

void FlexWire::flush(void) {
}

// Init function. Needs to be called once in the beginning.
// Returns false if SDA or SCL are low, which probably means 
// an I2C bus lockup or that the lines are not pulled up.
bool FlexWire::i2c_init(void) {
  digitalWrite(_sda, LOW);
  digitalWrite(_scl, LOW);
  delayMicroseconds(_i2cDelay/2);
  setSclHigh();
  delayMicroseconds(_i2cDelay/2);
  setSdaHigh();
  delayMicroseconds(_i2cDelay/2);
  if (getSda() == 0 || getScl() == 0) return false;
  return true;
}

// Start transfer function: <addr> is the 8-bit I2C address (including the R/W
// bit). 
// Return: true if the slave replies with an "acknowledge", false otherwise
// return also false when one of the lines is initially low (which might be a shortcut)
bool FlexWire::i2c_start(uint8_t addr) {
  if (getSda() == 0 || getScl() == 0) return false;
  setSdaLow();
  delayMicroseconds(_i2cDelay);
  setSclLow();
  delayMicroseconds(_i2cDelay/2);
  return i2c_write(addr);
}

// Repeated start function: After having claimed the bus with a start condition,
// you can address another or the same chip again without an intervening 
// stop condition.
// Return: true if the slave replies with an "acknowledge", false otherwise
bool FlexWire::i2c_rep_start(uint8_t addr) {
  setSdaHigh();
  delayMicroseconds(_i2cDelay/2);
  setSclHigh();
  delayMicroseconds(_i2cDelay);
  return i2c_start(addr);
}

// Issue a stop condition, freeing the bus.
void FlexWire::i2c_stop(void) {
  setSdaLow();
  delayMicroseconds(_i2cDelay);
  setSclHigh();
  delayMicroseconds(_i2cDelay);
  setSdaHigh();
  delayMicroseconds(_i2cDelay);
}

// Write one byte to the slave chip that had been addressed
// by the previous start call. <value> is the byte to be sent.
// Return: true if the slave replies with an "acknowledge", false otherwise
bool FlexWire::i2c_write(uint8_t value) {
  for (uint8_t curr = 0x80; curr != 0; curr >>= 1) {
    if (curr & value) setSdaHigh(); else  setSdaLow(); 
    setSclHigh();
    delayMicroseconds(_i2cDelay);
    setSclLow();
  }
  // get Ack or Nak
  setSdaHigh();
  setSclHigh();
  delayMicroseconds(_i2cDelay/2);
  uint8_t ack = getSda();
  setSclLow();
  delayMicroseconds(_i2cDelay/2);  
  setSdaLow();
  return ack == 0;
}

// Read one byte. If <last> is true, we send a NAK after having received 
// the byte in order to terminate the read sequence. 
uint8_t FlexWire::i2c_read(bool last) {
  uint8_t b = 0;
  setSdaHigh();
  for (uint8_t i = 0; i < 8; i++) {
    b <<= 1;
    delayMicroseconds(_i2cDelay);
    setSclHigh();
    if (getSda()) b |= 1;
    setSclLow();
  }
  if (last) setSdaHigh(); else setSdaLow();
  setSclHigh();
  delayMicroseconds(_i2cDelay/2);
  setSclLow();
  delayMicroseconds(_i2cDelay/2);  
  setSdaLow();
  return b;
}

#if AVR_OPTIMIZATION
inline void FlexWire::setSdaLow(void) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    *_sdaPortReg &= ~_sdaBitMask;
    *_sdaDirReg  |=  _sdaBitMask;

  }
}

void FlexWire::setSdaHigh(void) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    *_sdaDirReg &= ~_sdaBitMask;
    if(_pullup)  *_sdaPortReg |= _sdaBitMask; 

  }
}

void FlexWire::setSclLow(void) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    *_sclPortReg &= ~_sclBitMask;
    *_sclDirReg  |=  _sclBitMask;
  }
}

void FlexWire::setSclHigh(void) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    *_sclDirReg &= ~_sclBitMask;
    if(_pullup) { *_sclPortReg |= _sclBitMask; }
  }
}

uint8_t FlexWire::getSda(void) {
  return  ((uint8_t) (*_sdaPinReg & _sdaBitMask) ? 1 : 0);
}

uint8_t FlexWire::getScl(void) {
  return  ((uint8_t) (*_sclPinReg & _sclBitMask) ? 1 : 0);
}


#else

void FlexWire::setSdaLow(void) {
#ifdef ATOMIC_BLOCK
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
#endif
  {
    if (_pullup) 
      digitalWrite(_sda, LOW);
    pinMode(_sda, OUTPUT);
  }
}

void FlexWire::setSdaHigh(void) {
    if (_pullup) 
      pinMode(_sda, INPUT_PULLUP);
    else
      pinMode(_sda, INPUT);
}

void FlexWire::setSclLow(void) {
#ifdef ATOMIC_BLOCK
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
#endif
  {
    if (_pullup) 
      digitalWrite(_scl, LOW);
    pinMode(_scl, OUTPUT);
  }
}

void FlexWire::setSclHigh(void) {
    if (_pullup) 
      pinMode(_scl, INPUT_PULLUP);
    else
      pinMode(_scl, INPUT);
}

uint8_t FlexWire::getSda(void) {
  return  (digitalRead(_sda));
}

uint8_t FlexWire::getScl(void) {
  return  (digitalRead(_scl));
}

#endif
