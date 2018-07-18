#!/usr/bin/env python3

import Config
import Runner
import Test
# import serial
import time
import os
import datetime
import time
import RPi.GPIO as GPIO

# --------------------------PRINCIPAL PROGRAMM-------------------------- #

# serialBus = serial.Serial('/dev/cu.usbserial-DN02PRAQ', 115200, timeout=10)
# serialBus = serial.Serial('/COM7', 115200, timeout=10)
# serialBus = serial.Serial('/dev/ttyAMA0', 115200, timeout=10)

resultFileName = ('CoolBoard-Test-' +
                  datetime.datetime.now().strftime("%y-%m-%d-%H-%M") + ".txt")

GPIO.setmode(GPIO.BCM)
GPIO.setup(4, GPIO.OUT, initial=GPIO.LOW)

endOfTest = False
resetLineList = ["INFO:  Builtin actuator configuration loaded",
                 "INFO:  Wifi connecting...",
                 "INFO:  MQTT connecting...",
                 "INFO:  Listening to update messages..."]
resetLineNumber = 0
Config.resetBoard()

while not endOfTest:
    # puts in the good order all the test functions
    Test.SPIFFS(Config.serialBus, resultFileName, True)
    Test.MAIN_CONFIGURATION(Config.serialBus, resultFileName, True)
    Test.MAC_STATUS_TEST(Config.serialBus, resultFileName, True)
    Test.FIRMWARE_VERSION_TEST(Config.serialBus, resultFileName, True)

    if (not Test.REGISTERED_WIFI_TEST(Config.serialBus, resultFileName, False)):
        Test.WIFI_ACCESS_POINT_OPENED(Config.serialBus, resultFileName, True)

    elif (Test.WIFI_CONNECTION(Config.serialBus, resultFileName, True)):
        if (not Test.MQTT_CONNECTION(Config.serialBus, resultFileName, True)):
            Test.SLEEPMODE_ACTIVATE(Config.serialBus, resultFileName, True)
            Test.RETRY_MQTT_CONNECTION(Config.serialBus, resultFileName, True)
        else:
            Test.SEND_LOG(Config.serialBus, resultFileName, True)
            Test.RTC_SYNCHRO(Config.serialBus, resultFileName, True)
            while(Test.DATA_SAVED_JSON(Config.serialBus, resultFileName, False)):
                Test.DATA_DELETING_JSON(Config.serialBus, resultFileName, True)
            if(Test.EXTERNAL_SENSOR_ACTIVATED(Config.serialBus, resultFileName, False)):
                Test.EXTERNAL_SENSOR_DATA_SEND(Config.serialBus, resultFileName, True)

            if (Test.OTA_RECEIPTED(Config.serialBus, resultFileName, False)):
                Test.OTA_PUBLISHED(Config.serialBus, resultFileName, True)
                # if(Test.OTA_UPDATE_FIRMWARE(serialBus, resultFileName, True)):
                # Test.FIRMWARE_VERSION_TEST(serialBus, resultFileName, True)
    else:
        # Test.RETRY_WIFI_CONNECTION(serialBus, resultFileName, giveResult=True)
        # passera a la suite que si succes
        Test.SAVE_LOG(Config.serialBus, resultFileName, True)
        Test.DATA_SAVING_JSON(Config.serialBus, resultFileName, True)

    time.sleep(1)
    Config.resetBoard()
    time.sleep(1)
    endOfTest = Config.waitLine(resetLineList, resetLineNumber, resultFileName)
    resetLineNumber = resetLineNumber + 1
print("\n\n--------------------|END OF THE TEST|------------------------")

Config.serialBus.close()
GPIO.cleanup()