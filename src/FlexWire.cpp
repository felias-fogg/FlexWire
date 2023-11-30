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
  _sda(sda),
  _scl(scl),
  _pullup(internal_pullup)  { }

void FlexWire::begin(void) {
  _rxBufferIndex = 0;
  _rxBufferLength = 0;
  _error = 0;
  _transmitting = false;
  
  i2c_init();
}
  
void  FlexWire::setClock(uint32_t _) {
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

uint8_t FlexWire::requestFrom(uint8_t address, uint8_t quantity,
		    uint32_t iaddress, uint8_t isize, uint8_t sendStop) {
  uint8_t localerror = 0;
  if (isize > 0) {
    // send internal address; this mode allows sending a repeated start to access
    // some devices' internal registers. This function is executed by the hardware
    // TWI module on other processors (for example Due's TWI_IADR and TWI_MMR registers)
    beginTransmission(address);
    // the maximum size of internal address is 3 bytes
    if (isize > 3){
      isize = 3;
    }
    // write internal register address - most significant byte first
    while (isize-- > 0)
      write((uint8_t)(iaddress >> (isize*8)));
    endTransmission(false);
  }
  // clamp to buffer length
  if(quantity > BUFFER_LENGTH){
    quantity = BUFFER_LENGTH;
  }
  localerror = !i2c_rep_start((address<<1) | I2C_READ);
  if (_error == 0 && localerror) _error = 2;
  // perform blocking read into buffer
  for (uint8_t cnt=0; cnt < quantity; cnt++) 
    _rxBuffer[cnt] = i2c_read(cnt == quantity-1);
  // set rx buffer iterator vars
  _rxBufferIndex = 0;
  _rxBufferLength = quantity;
  if (sendStop) {
    _transmitting = 0;
    i2c_stop();
  }
  return quantity;
}

uint8_t FlexWire::requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint32_t)0, (uint8_t)0, (uint8_t)sendStop);
}

uint8_t FlexWire::requestFrom(int address, int quantity, int sendStop) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)sendStop);
}


uint8_t FlexWire::requestFrom(uint8_t address, uint8_t quantity) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
}

uint8_t FlexWire::requestFrom(int address, int quantity) {
  return requestFrom((uint8_t)address, (uint8_t)quantity, (uint8_t)true);
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
// a I2C bus lockup or that the lines are not pulled up.
bool FlexWire::i2c_init(void) {
  digitalWrite(_sda, LOW);
  digitalWrite(_scl, LOW);
  setHigh(_sda);
  setHigh(_scl);
  if (digitalRead(_sda) == LOW || digitalRead(_scl) == LOW) return false;
  return true;
}

// Start transfer function: <addr> is the 8-bit I2C address (including the R/W
// bit). 
// Return: true if the slave replies with an "acknowledge", false otherwise
bool FlexWire::i2c_start(uint8_t addr) {
  setLow(_sda);
  delayMicroseconds(DELAY);
  setLow(_scl);
  return i2c_write(addr);
}

// Repeated start function: After having claimed the bus with a start condition,
// you can address another or the same chip again without an intervening 
// stop condition.
// Return: true if the slave replies with an "acknowledge", false otherwise
bool FlexWire::i2c_rep_start(uint8_t addr) {
  setHigh(_sda);
  setHigh(_scl);
  delayMicroseconds(DELAY);
  return i2c_start(addr);
}

// Issue a stop condition, freeing the bus.
void FlexWire::i2c_stop(void) {
  setLow(_sda);
  delayMicroseconds(DELAY);
  setHigh(_scl);
  delayMicroseconds(DELAY);
  setHigh(_sda);
  delayMicroseconds(DELAY);
}

// Write one byte to the slave chip that had been addressed
// by the previous start call. <value> is the byte to be sent.
// Return: true if the slave replies with an "acknowledge", false otherwise
bool FlexWire::i2c_write(uint8_t value) {
  for (uint8_t curr = 0X80; curr != 0; curr >>= 1) {
    if (curr & value) setHigh(_sda); else  setLow(_sda); 
    setHigh(_scl);
    delayMicroseconds(DELAY);
    setLow(_scl);
  }
  // get Ack or Nak
  setHigh(_sda);
  setHigh(_scl);
  delayMicroseconds(DELAY/2);
  uint8_t ack = digitalRead(_sda);
  setLow(_scl);
  delayMicroseconds(DELAY/2);  
  setLow(_sda);
  return ack == 0;
}

// Read one byte. If <last> is true, we send a NAK after having received 
// the byte in order to terminate the read sequence. 
uint8_t FlexWire::i2c_read(bool last) {
  uint8_t b = 0;
  setHigh(_sda);
  for (uint8_t i = 0; i < 8; i++) {
    b <<= 1;
    delayMicroseconds(DELAY);
    setHigh(_scl);
    if (digitalRead(_sda)) b |= 1;
    setLow(_scl);
  }
  if (last) setHigh(_sda); else setLow(_sda);
  setHigh(_scl);
  delayMicroseconds(DELAY/2);
  setLow(_scl);
  delayMicroseconds(DELAY/2);  
  setLow(_sda);
  return b;
}

void FlexWire::setLow(uint8_t pin) {
    noInterrupts();
    if (_pullup) 
      digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
    interrupts();
}


void FlexWire::setHigh(uint8_t pin) {
    noInterrupts();
    if (_pullup) 
      pinMode(pin, INPUT_PULLUP);
    else
      pinMode(pin, INPUT);
    interrupts();
}

