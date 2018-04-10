#!/bin/bash

libdir=$(dirname $0)/../lib/
rm -rf ${libdir}
mkdir ${libdir}
pushd ${libdir}
git clone https://github.com/bblanchon/ArduinoJson
git clone https://github.com/Makuna/NeoPixelBus
git clone https://github.com/PaulStoffregen/Time
git clone https://github.com/etrombly/DS1337RTC
git clone https://github.com/milesburton/Arduino-Temperature-Control-Library
git clone https://github.com/PaulStoffregen/OneWire
git clone https://github.com/sparkfun/SparkFun_BME280_Arduino_Library
popd
