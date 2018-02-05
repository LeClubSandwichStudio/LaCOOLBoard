## What is La COOL Board?

**La COOL Board** is a connected environmental monitoring and control board. It can be used to build custom weather stations, self-watering plants, hydroponic control systems, pH probes, and many other things. It's also extensible and compatible with numerous external sensors and actuators.

### Key features and benefits
* 100% Arduino compatible
* Built on top of Espressif's ESP8266, with onboard WiFi
* 7 onboard sensors: IR and visible light, UV index, atmospheric pressure, humidity, temperature, soil moisture.
* Onboard RTC Clock
* UART over USB with onboard FTDI (female micro-USB port)
* NeoPixel RGB programmable LED
* Plug for a LiPo battery and a solar panel
* Fully makable, hackable and customizable open-hardware

## How to make it?

**Have a look at our [Eagle files](https://github.com/LaCoolCo/LaCOOLBoardHardware)** - but you can also buy it from us! Don't hesitate to go and check [our website](https://github.com/LaCoolCo/LaCOOLBoard)

## What is this repository for?

This is a set of Arduino libraries, wrappers and sketches to simplify the access and usage of all 
the capabilites of La COOL Board.

## How does it operates?

La COOL Board default operating mode is **sleep mode,** which uses the deep-sleep low-energy consumption mode of the ESP8266 (80µA). It is on by default and should be enabled whenever your board:
- runs on battery power (e.g. in a weather station)
- runs on AC power, but logs data at a very slow rate. Time to do you part in saving the planet! 

If you need a higher sample rate, you may want to deactivate it.

In sleep mode, your COOL Board will run the following loop:
1. read sensors values
2. activate actuators
3. log the data (either locally or over the network)
4. check for updates
5. go to sleep for `logInterval` seconds


## How can I contribute?

For minor fixes of code and documentation, please go ahead and submit a pull request.

Larger changes (rewriting parts of existing code from scratch, adding new functions to the core, adding new libraries) 
should generally be discussed by opening an issue first.

Feature branches with lots of small commits (especially titled "oops", "fix typo", "forgot to add file", etc.) should be squashed before opening a pull request. 
At the same time, please refrain from putting multiple unrelated changes into a single pull request.

## How can I reach you guys?

If you encounter a problem, have some genius, crazy idea or just want to have a chat with us, please open an issue, a pull request or send us an email at team@lacool.co - we'd absolutely love to hear from you !

## How do I get set up?

### Configuration
 
 1. Download and Install the [Arduino IDE](https://www.arduino.cc/en/Main/Software)
 2. Download and add the [ESP8266 hardware extension to Arduino](https://github.com/esp8266/Arduino). **If you already have Arduino IDE and the ESP8266 Hardware extension, make sure you upgrade them to the latest version.**
 3. Download and add the COOL Board library set to the Arduino IDE
     1. First Method
       * Open the Arduino IDE
       * Go to the `Sketch` Menu 
       * Choose `Include Library > Manage Libraries.` 
       * Search for `CoolBoard`
       * Install
       * Restart the Arduino IDE
     2. Second Method (if you have a CoolBoard.zip file )
       * Open the Arduino IDE
       * Go to the `Sketch > Include Library > Add .ZIP Library` menu item
       * Search for `CoolBoard.zip`
       * Click Open
       * Restart the Arduino IDE
     3. Third Method (with the library GitHub link)
       * Open Arduino IDE
       * Go to the `File > Preferences` menu item
       * Check "SketchBook Location" path
       * Go to the `Arduino/libraries` folder (create it if necessary)
       * Clone the repository the there (`git clone <github_repo_url`)
       * Restart the Arduino IDE
4. Download [ESP8266 FS Tool](https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#uploading-files-to-file-system)
5. **Optional but Heavily Recommended:** Download the [ESP8266 Exception Decoder](https://github.com/me-no-dev/EspExceptionDecoder/releases/tag/1.0.6) and follow its [installation guide](https://github.com/me-no-dev/EspExceptionDecoder)
   
### Dependencies :

You'll need the following libraries to be able to use the COOL Board API:

* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* [NeoPixelBus](https://github.com/Makuna/NeoPixelBus)
* [TimeLib](https://github.com/PaulStoffregen/Time)
* [DS1337RTC](https://github.com/etrombly/DS1337RTC)
* [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library)
* [OneWire](https://github.com/PaulStoffregen/OneWire)

### Configuration files

The CoolBoard API heavily uses the SPIFFS for storing and retreiving configuration and data files. Here is a description of the configuration files and keys.

#### `coolBoardConfig.json` 

`logInterval`: time interval in seconds between two log events.

`ireneActive`: Set this to `true` if you are using the IRN3000 module

`jetpackActive`: Set this to `true` if you are using the JetPack module

`externalSensorsActive`: Set to `true` if you are using a supported external sensor

`sleepActive`: Set to `true` if you want your COOL Board to enable [sleep mode](#sleep-mode)

`userActive`: Set this flag to `true` if you want your COOL Board to collect `userData` (`userName`, `macAddress`, `timeStamp`)

`manual`: Set this flag to `true` enables remoe-control of onboard actuators, bypassing rule-based configuration. **Be extremely careful with this mode:** when it is active, the COOL Board will not restart automatically to apply any new configuration sent on the `update` MQTT topic. Plus, restarting a COOL Board in manual mode will disable all the actuators. Thus, never forget to reset this to false to go back to normal mode!

`saveAsJSON`: Set this flag to 1 if you want the COOL Board to store untransmitted messages in the SPIFFS and send them as soon as we have a network connection. This is the best solution when your COOL Board Internet connectivity is not . Set it to 0 if you have no internet at all and use the `saveAsCSV` flag instead.

`saveAsCSV`: this flag saves the actual data as `.csv` file. Use it when your COOL Board is running in a fully offline scenario.

#### `coolBoardLedConfig.json`
  
`ledActive`: Put this flag to 1(true) if you want to turn on the onboard LED.

#### `coolBoardSensorsConfig.json`

`temperature`: set this to `true` if you want to sample temperature using the BME280 Sensor

`humidity`: set this to `true` if you want to sample air humidity using the BME280 Sensor

`pressure`: set this to `true` if you want to collect atmospheric pressure using the BME280 Sensor

`visible`: set this to `true` if you want to collect the visible light index using the SI114X Sensor

`ir`: set this to `true` if you want to measure the infrared light using the SI114X Sensor

`uv`: set this to `true` if you want to measure ultraviolet index using the SI114X Sensor

`vbat`: set this to `true` if you want to measure battery voltage 

`soilMoisture`: set this to `true` if you want to activate the soil moisture sensor	
	
#### `externalSensorsConfig.json`

`sensorsNumber`: the number of supported external sensors you connect to the COOL Board

`reference`: the reference of a supported external sensor (e.g. NDIR_I2C, Dallas Temperature...)

`type`: the type of the measurments you are making (e.g. CO2, temperature, voltage...)

`address`: the sensor's address, if it has one (e.g. NDIR_I2C CO2 sensor's address is 77)

`kind0`...`kind4`: names of the sensors sensor connected to ADCs models ADS1015 and ADS1115 (`kind0` is sensor on A0, `kind1` is A1, and so on)

#### `irene3000Config.json`
  
`waterTemp.active`: set to `true` to use the temperature sensor connected to the Irene3000

`phProbe.active`: set to `true` to use the ph sensor connected to the Irene3000

`adc2.active`: set to `true` to use the extra ADC input of the Irene3000

`adc2.gain`: this is the value of the gain applied to the extra ADC input of the Irene3000

`adc2.type`: the type of measurements you are making (e.g. CO2, temperature,voltage...)

#### `jetPackConfig.json` and `coolBoardActorConfig.json`

`Act[i].actif`: set to `true` in order to use the jetpack output #`i` (0 to 7)

`Act[i].inverted`: set `true` if the actor is inverted (e.g. a cooler is turned on when `Temp > TempMax`)

`Act[i].temporal`: set this to `true` if you want the actor to turned on or of based on time of day.

`Act[i].type: ["primaryType", "secondaryType"]`: this array contains the primary type and the secondary type of the actuator

`primaryType` : type of the sensor (e.g. if `Temperature`, actuator is associated to the "Temperature" sensor).

`secondaryType` is only used in temporal mode. It can be: 

* (empty): the actor will be on for `timeHigh` ms and off for `timeLow` ms
* `"hour"`: the actor will be on when current hour is greater than or equal to `hourHigh`. and off when greater than or equal to `hourLow`.
* `"minute"`: the actor will be on when the current minute is greater than or equal to `minuteHigh`, and off when greater than or equal to `minuteLow`
* `"hourMinute"`: behavior of both `"minute"` and `"hour"`

`Act[i].low[rangeLow,timeLow,hourLow,minuteLow]`: this array contains the low values of the activation range

* `rangeLow` is the minimum sensor value at which to turn on (or off in inverted mode) the actor
* `timeLow` is the time spent off in temporal mode
* `hourLow` is the hour to turn off the actor when `secondaryType` is `hour` or `hourMinute`
* `minuteLow` is the minute to turn off the actor when `secondaryType` is `minute` or `hourMinute`
* `Act[i].high:[rangeHigh,timeHigh,hourHigh,minuteHigh]`: this array contains the high values of the activation range

`rangeHigh` is the maximum of the range at which to turn off (or on in inverted mode) the actor

`timeHigh` is the time spent on in temporal mode

`hourHigh` is the hour to turn on the actor when `secondaryType` is `hour` or `hourMinute`

`minuteHigh` is the minute to turn on the actor when `secondaryType` is `minute` or `hourMinute`

Note that `coolBoardActorConfig.json` contains only one actor.

#### `mqttConfig.json`

`mqttServer`: MQTT server IP address or hostname

`user`: user name

`bufferSize`: memory allocated to the MQTT buffer in bytes

`inTopic`: MQTT topic that the COOL Board subscribes to (i.e. receives updates from)

`outTopic`: MQTT topic that the COOL Board will publish to. Autogenerated by default

#### `rtcConfig.json`

`NTP`: set to `true` if you want the RTC to synchronize with a NTP Pool. **Disable this if you don't have an Internet connection!**

`timeServer`: NTP server DNS address. Find more information and the best server for your area [here](http://www.pool.ntp.org)

`localPort`: port used to make the NTP request to update the time

`timeSync`: the last time the board updated the RTC (UNIX Time). By default, La COOL Board tries to update once a week.

#### `wifiConfig.json`
    
`wifiCount`: the number of WiFi networks that can be saved in this configuration file

`timeOut`: access point timeout in seconds

`nomad`: set this to `true` in odrder to activate nomad mode. In nomad mode, La COOL Board won't launch the onboard access point in the event of WiFi connection loss



