#!/usr/bin/env python3

import SerialFunctions
import SerialTest
import time
import os
import datetime
import time
import RPi.GPIO as GPIO
import Jetpack
import AwsMessage

# --------------------------PRINCIPAL PROGRAMM-------------------------- #

SerialFunctions.initReset()
Jetpack.initJetpack()

SerialFunctions.resultFileName

endOfTest = False
resetLineList = ["INFO:  Builtin actuator configuration loaded"]#,
                 #"INFO:  Wifi connecting...",
                 #"INFO:  MQTT connecting...",
                 #"INFO:  Listening to update messages..."]
resetLineNumber = 0
SerialFunctions.resetBoard()

while not endOfTest:
    # puts in the good order all the test functions
    SerialTest.SPIFFS(SerialFunctions.serialBus, resultFileName, True)
    SerialTest.MAIN_CONFIGURATION(SerialFunctions.serialBus, resultFileName, True)
    SerialTest.MAC_STATUS_TEST(SerialFunctions.serialBus, resultFileName, True)
    SerialTest.FIRMWARE_VERSION_TEST(SerialFunctions.serialBus, resultFileName, True)

    if (not SerialTest.REGISTERED_WIFI_TEST(SerialFunctions.serialBus, resultFileName, False)):
        SerialTest.WIFI_ACCESS_POINT_OPENED(SerialFunctions.serialBus, resultFileName, True)

    elif (SerialTest.WIFI_CONNECTION(SerialFunctions.serialBus, resultFileName, True)):
        if (not SerialTest.MQTT_CONNECTION(SerialFunctions.serialBus, resultFileName, True)):
            SerialTest.SLEEPMODE_ACTIVATE(SerialFunctions.serialBus, resultFileName, True)
            SerialTest.RETRY_MQTT_CONNECTION(SerialFunctions.serialBus, resultFileName, True)
        else:
            SerialTest.SEND_LOG(SerialFunctions.serialBus, resultFileName, True)
            SerialTest.RTC_SYNCHRO(SerialFunctions.serialBus, resultFileName, True)
            while(SerialTest.DATA_SAVED_JSON(SerialFunctions.serialBus, resultFileName, False)):
                SerialTest.DATA_DELETING_JSON(SerialFunctions.serialBus, resultFileName, True)
            if(SerialTest.EXTERNAL_SENSOR_ACTIVATED(SerialFunctions.serialBus, resultFileName, False)):
                SerialTest.EXTERNAL_SENSOR_DATA_SEND(SerialFunctions.serialBus, resultFileName, True)

            if (SerialTest.OTA_RECEIPTED(SerialFunctions.serialBus, resultFileName, False)):
                SerialTest.OTA_PUBLISHED(SerialFunctions.serialBus, resultFileName, True)
                if(SerialTest.OTA_UPDATE_FIRMWARE(SerialFunctions.serialBus, resultFileName, True)):
                    SerialTest.FIRMWARE_VERSION_TEST(SerialFunctions.serialBus, resultFileName, True)
    else:
        #SerialTest.RETRY_WIFI_CONNECTION(serialBus, resultFileName, giveResult=True)
        # passera a la suite que si succes
        SerialTest.SAVE_LOG(SerialFunctions.serialBus, resultFileName, True)
        SerialTest.DATA_SAVING_JSON(SerialFunctions.serialBus, resultFileName, True)

    time.sleep(1)
    SerialFunctions.resetBoard()
    time.sleep(1)
    endOfTest = SerialFunctions.waitLine(resetLineList, resetLineNumber, resultFileName)
    resetLineNumber = resetLineNumber + 1

SerialFunctions.resetBoard()
AwsMessage.sendMessageLogInterval()
# AwsMessage.sendMessageFirmwareVersion()
# SerialFunctions.waitSynchroAct(resultFileName)
AwsMessage.sendMessageAct0()
AwsMessage.sendMessageAct1()
#SerialFunctions.resetBoard()
# SerialFunctions.waitSynchroAct(resultFileName)
Jetpack.ACTUATOR_1_ACTIVATED(SerialFunctions.serialBus, resultFileName, True)
Jetpack.ACTUATOR_2_DISACTIVATED(SerialFunctions.serialBus, resultFileName, True)
Jetpack.ACTUATOR_3_ACTIVATED(SerialFunctions.serialBus, resultFileName, True)
Jetpack.ACTUATOR_4_DISACTIVATED(SerialFunctions.serialBus, resultFileName, True)


print("\n\n--------------------|END OF THE TEST|------------------------")

SerialFunctions.serialBus.close()
GPIO.cleanup()