THIS_DIR := $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
SKETCH := ${THIS_DIR}/examples/WeatherStation/WeatherStation.ino
LIBS = ${ESP_LIBS}/SPI \
	${ESP_LIBS}/ESP8266WiFi/ \
	${ESP_LIBS}/ESP8266WebServer/ \
	${ESP_LIBS}/Wire/ \
	$(THIS_DIR)/lib/ArduinoJson/src/ \
	$(THIS_DIR)/lib/NeoPixelBus/src/ \
	$(THIS_DIR)/lib/SparkFun_BME280_Arduino_Library/src/ \
	$(THIS_DIR)/lib/Time/ \
	$(THIS_DIR)/lib/DS1337RTC/ \
	$(THIS_DIR)/lib/Arduino-Temperature-Control-Library/ \
	$(THIS_DIR)/lib/OneWire/ \
  $(THIS_DIR)/src/

UPLOAD_SPEED = 115200
FLASH_DEF = 4M3M
F_CPU = 80000000L
FLASH_MODE = dio
FLASH_SPEED = 40
UPLOAD_RESET = ck
UPLOAD_SPEED = 115200

