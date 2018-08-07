#!/usr/bin/env python3
import serial
import time
#import AwsMessage
serialBus = serial.Serial('/dev/cu.usbserial-DN02PRAQ', 115200, timeout=10)


def wifiRegistered():
    if serialBus.isOpen():
        while (True):
            #time.sleep(1)
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            #print(lineReturned)
            if (lineReturned[34:57] == "INFO:    Wifi count  = "):#5! si chiffre mettre ou =2 =3 ?
                print(lineReturned[34:])
                return False
            else:
                print(lineReturned)


def updateOTA():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:64] == "INFO:  MQTT publish successful"):
                print(lineReturned[34:])
                return False
            else:
                print(lineReturned)


def updateJsonFile():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:62] == "DEBUG: Saved JSON config to"):
                print(lineReturned[34:])
                return False
            else:
                print(lineReturned)


def firmwareUpdate():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            #if (lineReturned[34:69] == "INFO:  New firmware update received")
            if (lineReturned[34:61] == "INFO:  Firmware version is:"):
                print(lineReturned[34:])
                return False
            else:
                print(lineReturned)


def wifiConnexion():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            print(lineReturned)
            if (lineReturned[34:63] == "INFO:  Wifi status: connected"):
                print(lineReturned[34:])
                return False
            else:
                print(lineReturned)


def endLoop():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            print(lineReturned)
            if (lineReturned[34:68] == "INFO:  Going to sleep for seconds:"):
                print(lineReturned[34:])
                return False
            else:
                print(lineReturned)


def accessPoint():
    if serialBus.isOpen():
        lineNumber = 0
        lineBuffer = []
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[1:29] == "WM: Configuring access point"):
                print(lineReturned[1:])
                return False
            else:
                print(lineReturned)
    else:
        return False