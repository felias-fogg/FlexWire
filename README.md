# FlexWire

[![License: LGPL v2.1](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/lgpl-2.1)
[![Installation instructions](https://www.ardu-badge.com/badge/FlexWire.svg?)](https://www.ardu-badge.com/FlexWire)
[![Commits since latest](https://img.shields.io/github/commits-since/felias-fogg/FlexMaster/latest)](https://github.com/felias-fogg/FlexWire/commits/main)
[![Build Status](https://github.com/felias-fogg/FlexWire/workflows/LibraryBuild/badge.svg)](https://github.com/felias-fogg/FlexWire/actions)
![Hit Counter](https://visitor-badge.laobi.icu/badge?page_id=felias-fogg_FlexWire)

## Another Software I2C Library emulating the Wire Library

Why would you like to use this library? Well, it is a true drop-in replacement for the Wire library (master part). When you include it in your sketch file before any other library, then it will take over the part of the Wire library. In other words, you can simply use any sensor/actuator library that makes use of the Wire library, and it will work (magically!). Further, since you can dynamically change the pins for SDA and SCL, you can have as many I2C buses as you want. This means you can implement a sort of software I2C multiplexer, communicating with I2C devices that have the same I2C address.

Furthermore, since it uses only high-level Arduino built-in functions, it should work on all architectures supported by Arduino.

The only (?) disadvantage is that it is somewhat slow. However, it is planned to speed up things for the AVR MCUs. For ARM and ESP chips, speed should not be the problem.
