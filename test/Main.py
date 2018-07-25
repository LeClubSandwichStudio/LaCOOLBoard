#!/usr/bin/env python3

import SerialParameters
import SerialTest
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
    SerialTest.SPIFFS(SerialParameters.serialBus, resultFileName, True)
    SerialTest.MAIN_CONFIGURATION(SerialParameters.serialBus, resultFileName, True)
    SerialTest.MAC_STATUS_TEST(SerialParameters.serialBus, resultFileName, True)
    SerialTest.FIRMWARE_VERSION_TEST(SerialParameters.serialBus, resultFileName, True)

    if (not SerialTest.REGISTERED_WIFI_TEST(SerialParameters.serialBus, resultFileName, False)):
        SerialTest.WIFI_ACCESS_POINT_OPENED(SerialParameters.serialBus, resultFileName, True)

    elif (SerialTest.WIFI_CONNECTION(SerialParameters.serialBus, resultFileName, True)):
        if (not SerialTest.MQTT_CONNECTION(SerialParameters.serialBus, resultFileName, True)):
            SerialTest.SLEEPMODE_ACTIVATE(SerialParameters.serialBus, resultFileName, True)
            SerialTest.RETRY_MQTT_CONNECTION(SerialParameters.serialBus, resultFileName, True)
        else:
            SerialTest.SEND_LOG(SerialParameters.serialBus, resultFileName, True)
            SerialTest.RTC_SYNCHRO(SerialParameters.serialBus, resultFileName, True)
            while(SerialTest.DATA_SAVED_JSON(SerialParameters.serialBus, resultFileName, False)):
                SerialTest.DATA_DELETING_JSON(SerialParameters.serialBus, resultFileName, True)
            if(SerialTest.EXTERNAL_SENSOR_ACTIVATED(SerialParameters.serialBus, resultFileName, False)):
                SerialTest.EXTERNAL_SENSOR_DATA_SEND(SerialParameters.serialBus, resultFileName, True)

            if (SerialTest.OTA_RECEIPTED(SerialParameters.serialBus, resultFileName, False)):
                SerialTest.OTA_PUBLISHED(SerialParameters.serialBus, resultFileName, True)
                # if(SerialTest.OTA_UPDATE_FIRMWARE(serialBus, resultFileName, True)):
                # SerialTest.FIRMWARE_VERSION_TEST(serialBus, resultFileName, True)
    else:
        # SerialTest.RETRY_WIFI_CONNECTION(serialBus, resultFileName, giveResult=True)
        # passera a la suite que si succes
        SerialTest.SAVE_LOG(SerialParameters.serialBus, resultFileName, True)
        SerialTest.DATA_SAVING_JSON(SerialParameters.serialBus, resultFileName, True)

    time.sleep(1)
    SerialParameters.resetBoard()
    time.sleep(1)
    endOfTest = SerialParameters.waitLine(resetLineList, resetLineNumber, resultFileName)
    resetLineNumber = resetLineNumber + 1
print("\n\n--------------------|END OF THE TEST|------------------------")

SerialParameters.serialBus.close()
GPIO.cleanup()