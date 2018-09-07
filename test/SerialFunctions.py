#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time
import serial
import SerialTest
import datetime

GPIO.setmode(GPIO.BCM)
GPIO.setup(4, GPIO.OUT, initial=GPIO.LOW)

resultFileName = ('CoolBoard-Test-' +
                  datetime.datetime.now().strftime("%y-%m-%d-%H-%M") + ".txt")

#serialBus = serial.Serial('/dev/cu.usbserial-DN02PRAQ', 115200, timeout=10) # MAC
serialBus = serial.Serial('/dev/ttyUSB0', 115200, timeout=10) # Raspberry 3 B+

lineNumber = 0
lineBuffer = []


def absolutFlushBuffer():
    global lineBuffer
    global lineNumber
    lineBuffer = []
    lineNumber = 0
    serialBus.flushInput()
    return


def flushBuffer():
    global lineBuffer
    global lineNumber
    lineBuffer = lineBuffer[lineNumber:]
    lineNumber = 0
    return


def resetBuffer():
    global lineNumber
    lineNumber = 0
    return


def readLine(serialBus):
    global lineBuffer
    global lineNumber
    if (len(lineBuffer) == lineNumber):
        lineReturned = str(serialBus.readline())
        if lineReturned[2] == "[":
            lineNumber += 1
            if lineReturned[35] == "]":
                lineBuffer.append(lineReturned[36:-5])
                return lineReturned[36:-5]
            else:
                lineBuffer.append(lineReturned[35:-5])
                return lineReturned[35:-5]
        elif (lineReturned[2] == "I" or lineReturned[2] == "W" or
                lineReturned[2] == "E" or lineReturned[2] == "D"):
            lineNumber += 1
            lineBuffer.append(lineReturned[2:-5])
            return lineReturned[2:-5]
        elif lineReturned[2] == "*":
            lineNumber += 1
            lineBuffer.append(lineReturned[6:])
            return lineReturned[6:]
        else:
            return readLine(serialBus)
    else:
        lineReturned = lineBuffer[lineNumber]
        lineNumber += 1
        return lineReturned


def resetBoard():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(4, GPIO.OUT, initial=GPIO.LOW)
    GPIO.output(4, True)
    time.sleep(2)
    GPIO.output(4, False)
    absolutFlushBuffer()


def writeResetLine(Line, resultFileName):
    resultFile = open(resultFileName, 'a')
    print("\n Reset at line : ", Line)
    resultFile.write("\nRESET at line:" + " " * 30 + Line + "\n")
    resultFile.close()


def waitLine(resetLineList, lineNumber, resultFileName):
    if lineNumber >= len(resetLineList):
        return True
    else:
        writeResetLine(resetLineList[lineNumber], resultFileName)
        resetLineLen = len(resetLineList[lineNumber])
        while (resetLineList[lineNumber] !=
                readLine(serialBus)[: resetLineLen]):
            continue
        resetBoard()
        return False


def waitSynchroAct(resultFileName):
    if(not SerialTest.OTA_SYNCHRO(serialBus, resultFileName, False)):
        resultFile = open(resultFileName, 'a')
        resultFile.write("WARNING reached limit time for OTA to be published \n")
        print("-------------------: WARNING reached limit time for OTA to be published\n")
        resultFile.close()