#ifndef COOLPINS_H
#define COOLPINS_H

#ifdef ARDUINO_ARCH_ESP32
// No I²C enable pin
#define ENABLE_I2C_PIN 5
// TODO
#define SDA_I2C_PIN 2
#define SCL_I2C_PIN 14
#define BOOTSTRAP_PIN 37
// No onboard actuator here
#define ONBOARD_ACTUATOR_PIN -1
// No moisture sensor here
#define MOISTURE_SENSOR_PIN -1
// ??
#define ANALOG_MULTIPLEXER_PIN -1
// No Jetpack... Then what ?
#define JETPACK_CLOCK_PIN 4
#define JETPACK_DATA_PIN 15
#define JETPACK_I2C_ENABLE_PIN 5
// No SHT1X
#define SHT1X_DATA_PIN -1
#define SHT1X_CLOCK_PIN -1
#else
#define ENABLE_I2C_PIN 5
#define SDA_I2C_PIN 2
#define SCL_I2C_PIN 14
#define BOOTSTRAP_PIN 37
#define ONBOARD_ACTUATOR_PIN 15
#define MOISTURE_SENSOR_PIN 13
#define ANALOG_MULTIPLEXER_PIN 12
#define JETPACK_CLOCK_PIN 4
#define JETPACK_DATA_PIN 15
#define JETPACK_I2C_ENABLE_PIN 5
#define SHT1X_DATA_PIN 0
#define SHT1X_CLOCK_PIN 12
#endif

#endif