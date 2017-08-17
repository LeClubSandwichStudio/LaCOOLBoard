# README #

This README would normally document whatever steps are necessary to get your application up and running.

### What is this repository for? ###

Quick summary : 

CoolBoard API is a set of libraries and wrappers to simplify the access and usage of all 
the capabilites of the CoolBoard.

Version : 1.0

### How do I get set up? ###

Configuration :
 
 1/Download and Install the Arduino IDE (https://www.arduino.cc/en/Main/Software )
 
 2/Download and Add the ESP8266 Hardware extension to Arduino (https://github.com/esp8266/Arduino )
 
 3/Download and Add the CoolBoard Library set to the Arduino IDE :
 
   a)First Method :
   -Open the Arduino IDE
   -Go to the "Sketch" Menu 
   -Include Library > Manage Libraries. 
   -Search for CoolBoard
   -Install 
   
   b)Second Method (if you have a CoolBoard.zip file )
   -Open the Arduino IDE
   -Go to the "Sketch" Menu 
   -Include Library > "Add .ZIP Library". 
   -Search for CoolBoard.zip 
   -Click Open
   
   c)Third Method (if you have the bitbucket/github link)
   -Open Arduino IDE > File > Preferences
   -Check the "SketchBook Location" path
   -Go to the Arduino/libraries folder (if it doesn't exist, create one )
   -Clone the repo there ( git clone "bitbucket/github link" )
   
Dependencies :

You need the following libraries to be able to use the CoolBoard API:

 -ArduinoJson(https://github.com/bblanchon/ArduinoJson)

-NeoPixelBus(https://github.com/Makuna/NeoPixelBus)

-TimeLib(https://github.com/PaulStoffregen/Time)

-DS1337RTC(https://github.com/etrombly/DS1337RTC)

-DallasTemperature(https://github.com/milesburton/Arduino-Temperature-Control-Library)



* Database configuration
* How to run tests
* Deployment instructions

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### Who do I talk to? ###

* Repo owner or admin
* Other community or team contact