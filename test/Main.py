#!/usr/bin/env python3

import SerialParameters
import Runner
import Test
import time
import os
import datetime
import time
import RPi.GPIO as GPIO
import Jetpack


# --------------------------PRINCIPAL PROGRAMM-------------------------- #

SerialParameters.initReset()
Jetpack.initJetpack()

resultFileName = ('CoolBoard-Test-' +
                  datetime.datetime.now().strftime("%y-%m-%d-%H-%M") + ".txt")

endOfTest = False
resetLineList = ["INFO:  Builtin actuator configuration loaded",
                 "INFO:  Wifi connecting...",
                 "INFO:  MQTT connecting...",
                 "INFO:  Listening to update messages..."]
resetLineNumber = 0
SerialParameters.resetBoard()

while not endOfTest:
    # puts in the good order all the test functions
    Test.SPIFFS(SerialParameters.serialBus, resultFileName, True)
    Test.MAIN_CONFIGURATION(SerialParameters.serialBus, resultFileName, True)
    Test.MAC_STATUS_TEST(SerialParameters.serialBus, resultFileName, True)
    Test.FIRMWARE_VERSION_TEST(SerialParameters.serialBus, resultFileName, True)

    if (not Test.REGISTERED_WIFI_TEST(SerialParameters.serialBus, resultFileName, False)):
        Test.WIFI_ACCESS_POINT_OPENED(SerialParameters.serialBus, resultFileName, True)

    elif (Test.WIFI_CONNECTION(SerialParameters.serialBus, resultFileName, True)):
        if (not Test.MQTT_CONNECTION(SerialParameters.serialBus, resultFileName, True)):
            Test.SLEEPMODE_ACTIVATE(SerialParameters.serialBus, resultFileName, True)
            Test.RETRY_MQTT_CONNECTION(SerialParameters.serialBus, resultFileName, True)
        else:
            Test.SEND_LOG(SerialParameters.serialBus, resultFileName, True)
            Test.RTC_SYNCHRO(SerialParameters.serialBus, resultFileName, True)
            while(Test.DATA_SAVED_JSON(SerialParameters.serialBus, resultFileName, False)):
                Test.DATA_DELETING_JSON(SerialParameters.serialBus, resultFileName, True)
            if(Test.EXTERNAL_SENSOR_ACTIVATED(SerialParameters.serialBus, resultFileName, False)):
                Test.EXTERNAL_SENSOR_DATA_SEND(SerialParameters.serialBus, resultFileName, True)

            if (Test.OTA_RECEIPTED(SerialParameters.serialBus, resultFileName, False)):
                Test.OTA_PUBLISHED(SerialParameters.serialBus, resultFileName, True)
                # if(Test.OTA_UPDATE_FIRMWARE(serialBus, resultFileName, True)):
                # Test.FIRMWARE_VERSION_TEST(serialBus, resultFileName, True)
    else:
        # Test.RETRY_WIFI_CONNECTION(serialBus, resultFileName, giveResult=True)
        # passera a la suite que si succes
        Test.SAVE_LOG(SerialParameters.serialBus, resultFileName, True)
        Test.DATA_SAVING_JSON(SerialParameters.serialBus, resultFileName, True)

    time.sleep(1)
    SerialParameters.resetBoard()
    time.sleep(1)
    endOfTest = SerialParameters.waitLine(resetLineList, resetLineNumber, resultFileName)
    resetLineNumber = resetLineNumber + 1
print("\n\n--------------------|END OF THE TEST|------------------------")

SerialParameters.serialBus.close()
GPIO.cleanup()