# FlexWire

[![License: LGPL v2.1](https://img.shields.io/badge/License-LGPLv2.1-blue.svg)](https://www.gnu.org/licenses/lgpl-2.1)
[![Installation instructions](https://www.ardu-badge.com/badge/FlexWire.svg?)](https://www.ardu-badge.com/FlexWire)
[![Commits since latest](https://img.shields.io/github/commits-since/felias-fogg/FlexWire/latest)](https://github.com/felias-fogg/FlexWire/commits/main)
[![Build Status](https://github.com/felias-fogg/FlexWire/workflows/LibraryBuild/badge.svg)](https://github.com/felias-fogg/FlexWire/actions)
![Hit Counter](https://visitor-badge.laobi.icu/badge?page_id=felias-fogg.FlexWire)

## Another Software I2C Library emulating the Wire Library

Why would you like to use this library? Well, it is a true drop-in replacement for the Wire library (master part). When you include it in your sketch file before any other library, then it will take over the part of the Wire library. In other words, you can simply use any sensor/actuator library that makes use of the Wire library, and it will work ([magically!](https://arduino-craft-corner.de/index.php/2023/11/29/replacing-the-wire-library-sometimes/)). Additionally, since you can dynamically change the pins for SDA and SCL, you can have as many I2C buses as you want. This means you can implement a sort of software I2C multiplexer, communicating with several I2C devices that have the same I2C address ([see my blog post on that](https://arduino-craft-corner.de/index.php/2023/12/14/software-i2c-multiplexer/)). Finally, since it uses only high-level Arduino built-in functions, it should work on all architectures supported by Arduino.

The only disadvantage is that it is somewhat slow. Since V1.1.0, on AVR MCUs bit-banging is implemented using port manipulation, which means that on an AVR MCU running at 16 MHz, you can get up to 140 kHz bus frequency. The default is 90 kHz. For other architectures, I have not measured the speed (yet).

**Note:** When you have compiled your sketch using the standard Wire library it may be necessary to exit and restart the Arduino IDE before compiling using the FlexWire library. Otherwise, the IDE might reuse the precompiled Wire library. 
