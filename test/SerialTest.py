#!/usr/bin/env python3
import SerialRunner

# --------------------------TEST FONCTIONS-------------------------- #


def MAC_STATUS_TEST(serialBus, resultFileName, giveResult=True):
    test_name = "MAC TEST"
    begin = ""
    error = ""
    success = "INFO:  MAC address is:"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def FIRMWARE_VERSION_TEST(serialBus, resultFileName, giveResult=True):
    test_name = "FIRMWARE VERSION"
    begin = ""
    error = ""
    success = "INFO:  Firmware version"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def REGISTERED_WIFI_TEST(serialBus, resultFileName, giveResult=False):
    test_name = "WIFI REGISTERED"
    begin = "INFO:  Wifi configuration loaded"
    error = "INFO:    Wifi count  = 0"
    success = "INFO:    Wifi count"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def RETRY_WIFI_CONNECTION(serialBus, resultFileName, giveResult=True):
    test_name = "RETRY WIFI CONNECTION"
    begin = "WARN:  Network unreachable"
    error = "INFO:  Wifi status: idle"
    success = "INFO:  Wifi status: connected"
    readFromBeginning = False
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def RETRY_MQTT_CONNECTION(serialBus, resultFileName, giveResult=True):
    test_name = "RETRY MQTT CONNECTION"
    begin = "INFO:  Wifi connecting..."
    error = "INFO:  Subscribed to MQTT input topic"
    success = "WARN:  MQTT connection failed, retrying"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def WIFI_CONNECTION(serialBus, resultFileName, giveResult=True):
    test_name = "WIFI CONNECTION"
    begin = "INFO:  Connecting..."
    error = "WARN:  Network unreachable"
    success = "INFO:  Wifi status: connected"
    readFromBeginning = False
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def WIFI_ACCESS_POINT_OPENED(serialBus, resultFileName, giveResult=True):
    print('test')
    test_name = "WIFI ACCESS POINT OPENED"
    begin = "INFO:  Wifi configuration loaded"
    error = "DEBUG: Reading configuration file as JSON: /wifiConfig.json"
    success = "INFO:  Starting Wifi access point and configuration portal"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def MQTT_CONNECTION(serialBus, resultFileName, giveResult=True):
    test_name = "MQTT CONNECTION"
    begin = ""
    error = "WARN:  MQTT connection failed"
    success = "INFO:  Subscribed to MQTT input topic"
    readFromBeginning = False
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def RTC_SYNCHRO(serialBus, resultFileName, giveResult=True):
    test_name = "RTC_SYNCHRO"
    begin = "INFO:  Synchronizing RTC..."
    error = "WARN:  NTP failed, falling back to RTC"
    success = "INFO:  RTC ISO8601 timestamp:"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def SEND_LOG(serialBus, resultFileName, giveResult=True):
    test_name = "SEND LOG"
    begin = "INFO:  Collecting board and sensor data..."
    error = "WARN:  Log not sent"
    success = "INFO:  MQTT publish successful"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def SAVE_LOG(serialBus, resultFileName, giveResult=True):
    test_name = "SAVE LOG"
    begin = "INFO:  Sending log over MQTT..."
    error = ""  # a remplir
    success = "WARN:  Log not sent, saved on SPIFFS"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def MAIN_CONFIGURATION(serialBus, resultFileName, giveResult=True):
    test_name = "READ MAIN CONFIGURATION"
    begin = ""
    error = "ERROR:  Failed to parse main configuration"
    success = "INFO:  Main configuration loaded"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def SPIFFS(serialBus, resultFileName, giveResult=True):
    test_name = "READ ALL SPIFFS"
    begin = ""
    error = "ERROR:  Filesystem failed, please contact support!"
    success = "DEBUG: Configuring MQTT"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def SLEEPMODE_ACTIVATE(serialBus, resultFileName, giveResult=True):
    test_name = "SLEEPMODE ACTIVATE"
    begin = "DEBUG: Reading configuration file as JSON: /coolBoardConfig.json"
    error = "INFO:    Sleep active            = 0"
    success = "INFO:    Sleep active            = 1"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def DATA_SAVING_JSON(serialBus, resultFileName, giveResult=True):
    test_name = "SAVING DATA IN JSON"
    begin = "WARN:  Log not sent, saved on SPIFFS"
    error = ""
    success = "DEBUG: Saved file data name:  /log/"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def DATA_DELETING_JSON(serialBus, resultFileName, giveResult=True):
    test_name = "DELETING DATA SAVED IN JSON"
    begin = "DEBUG: Message to log:"
    error = "INFO:  Going to sleep for seconds:"
    success = "INFO:  Deleted log file: /log/"
    readFromBeginning = False
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def DATA_SAVED_JSON(serialBus, resultFileName, giveResult=True):
    test_name = "DATA SAVED IN JSON"
    begin = "DEBUG: Message to log:"
    error = "INFO:  Going to sleep for seconds:"
    success = "DEBUG: Saved file data name:  /log/"
    readFromBeginning = False
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def EXTERNAL_SENSOR_ACTIVATED(serialBus, resultFileName, giveResult=True):
    test_name = "EXTERNAL SENSOR ACTIVATED"
    begin = "DEBUG: Reading configuration file as JSON: /coolBoardConfig.json"
    error = "INFO:    External sensors active = 0"
    success = "INFO:    External sensors active = 1"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def EXTERNAL_SENSOR_DATA_SEND(serialBus, resultFileName, giveResult=True):
    test_name = "EXTERNAL SENSOR DATA SEND"
    begin = "DEBUG: Reading configuration file as JSON: /externalSensorsConfig.json"
    error = ""
    success = "INFO:  Number of external sensors ="
    readFromBeginning = False
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def OTA_RECEIPTED(serialBus, resultFileName, giveResult=True):
    test_name = "MESSAGE RECEIPTED BY OTA"
    begin = "INFO:  Listening to update messages..."
    error = "INFO:  Going to sleep for seconds:"
    success = "INFO:  MQTT publish successful"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def OTA_PUBLISHED(serialBus, resultFileName, giveResult=True):
    test_name = "MESSAGE RECEIPTED BY OTA PUBLISHED"
    begin = "INFO:  Received new MQTT message"
    error = "ERROR:  Failed to update configuration file:"
    success = "DEBUG: Successfully updated configuration file:"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult


def OTA_SYNCHRO(serialBus, resultFileName, giveResult=True):
    test_name = "OTA SYNCHRONISATION"
    begin = "INFO:  Received new MQTT message"
    error = ""
    success = "DEBUG: Preparing answer message:"
    readFromBeginning = True
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult

def OTA_UPDATE_FIRMWARE(serialBus, resultFileName, giveResult=True):
    test_name = "FIRMWARE UPDATED BY OTA"
    begin = ""
    error = "ERROR:  HTTP Update failed, code:"
    success = "HTTP update succeeded!"
    readFromBeginning = False
    maxLinesRead = 500
    testResult = SerialRunner.runner(
        serialBus, resultFileName, maxLinesRead,
        test_name, begin, error,
        success, readFromBeginning, giveResult)
    return testResult