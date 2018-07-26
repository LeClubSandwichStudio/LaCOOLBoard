#!/usr/bin/env python3

import SerialFonctions
import SerialTest
import time
import os
import datetime
import time
import RPi.GPIO as GPIO
import Jetpack
import AwsMessage

# --------------------------PRINCIPAL PROGRAMM-------------------------- #

SerialFonctions.initReset()
Jetpack.initJetpack()

resultFileName = ('CoolBoard-Test-' +
                  datetime.datetime.now().strftime("%y-%m-%d-%H-%M") + ".txt")

endOfTest = False
resetLineList = ["INFO:  Builtin actuator configuration loaded"]#,
                 #"INFO:  Wifi connecting...",
                 #"INFO:  MQTT connecting...",
                 #"INFO:  Listening to update messages..."]
resetLineNumber = 0
SerialFonctions.resetBoard()

while not endOfTest:
    # puts in the good order all the test functions
    SerialTest.SPIFFS(SerialFonctions.serialBus, resultFileName, True)
    SerialTest.MAIN_CONFIGURATION(SerialFonctions.serialBus, resultFileName, True)
    SerialTest.MAC_STATUS_TEST(SerialFonctions.serialBus, resultFileName, True)
    SerialTest.FIRMWARE_VERSION_TEST(SerialFonctions.serialBus, resultFileName, True)

    if (not SerialTest.REGISTERED_WIFI_TEST(SerialFonctions.serialBus, resultFileName, False)):
        SerialTest.WIFI_ACCESS_POINT_OPENED(SerialFonctions.serialBus, resultFileName, True)

    elif (SerialTest.WIFI_CONNECTION(SerialFonctions.serialBus, resultFileName, True)):
        if (not SerialTest.MQTT_CONNECTION(SerialFonctions.serialBus, resultFileName, True)):
            SerialTest.SLEEPMODE_ACTIVATE(SerialFonctions.serialBus, resultFileName, True)
            SerialTest.RETRY_MQTT_CONNECTION(SerialFonctions.serialBus, resultFileName, True)
        else:
            SerialTest.SEND_LOG(SerialFonctions.serialBus, resultFileName, True)
            SerialTest.RTC_SYNCHRO(SerialFonctions.serialBus, resultFileName, True)
            while(SerialTest.DATA_SAVED_JSON(SerialFonctions.serialBus, resultFileName, False)):
                SerialTest.DATA_DELETING_JSON(SerialFonctions.serialBus, resultFileName, True)
            if(SerialTest.EXTERNAL_SENSOR_ACTIVATED(SerialFonctions.serialBus, resultFileName, False)):
                SerialTest.EXTERNAL_SENSOR_DATA_SEND(SerialFonctions.serialBus, resultFileName, True)

            if (SerialTest.OTA_RECEIPTED(SerialFonctions.serialBus, resultFileName, False)):
                SerialTest.OTA_PUBLISHED(SerialFonctions.serialBus, resultFileName, True)
                # if(SerialTest.OTA_UPDATE_FIRMWARE(serialBus, resultFileName, True)):
                # SerialTest.FIRMWARE_VERSION_TEST(serialBus, resultFileName, True)
    else:
        # SerialTest.RETRY_WIFI_CONNECTION(serialBus, resultFileName, giveResult=True)
        # passera a la suite que si succes
        SerialTest.SAVE_LOG(SerialFonctions.serialBus, resultFileName, True)
        SerialTest.DATA_SAVING_JSON(SerialFonctions.serialBus, resultFileName, True)

    time.sleep(1)
    SerialFonctions.resetBoard()
    time.sleep(1)
    endOfTest = SerialFonctions.waitLine(resetLineList, resetLineNumber, resultFileName)
    resetLineNumber = resetLineNumber + 1

AwsMessage.sendMessageLogInterval()
# AwsMessage.sendMessageFirmwareVersion()
# SerialFonctions.waitSynchroAct(resultFileName)
AwsMessage.sendMessageAct0()
AwsMessage.sendMessageAct1()
SerialFonctions.waitSynchroAct(resultFileName)
Jetpack.ACTUATOR_1_ACTIVATED(SerialFonctions.serialBus, resultFileName, True)
Jetpack.ACTUATOR_2_ACTIVATED(SerialFonctions.serialBus, resultFileName, True)
Jetpack.ACTUATOR_3_ACTIVATED(SerialFonctions.serialBus, resultFileName, True)
Jetpack.ACTUATOR_4_DISACTIVATED(SerialFonctions.serialBus, resultFileName, True)

print("\n\n--------------------|END OF THE TEST|------------------------")

SerialFonctions.serialBus.close()
GPIO.cleanup()